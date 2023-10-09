/*
 * moteurs.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "moteurs.h"
#include "timers.h"

#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_tim.h"

#include <limits.h>

/*
 * Global informations
 * Main clock: 6 Mhz
 * TIM2 PWM Input (CH1): Encodeur droit PHB : 0 -> 65535
 * TIM21 PWM Input (CH1): Encodeur Gauche PHA: 0 -> 65535
 * TIM3: PWM Output moteur (0->200) (~30 Khz)
 */

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim21;
extern TIM_HandleTypeDef htim3;

#define MOTEURS_MAX_COMMANDE	200
#define MOTEURS_MAX_ENCODEUR	USHRT_MAX

typedef struct {
	int16_t commande;
	int16_t consigne;
	uint16_t encodeur;
	uint16_t encodeurFront;
	uint8_t moteurLent;
} MOTEURS_EtatMoteur;

typedef struct {
	uint8_t type;
	int16_t commande;
	int16_t consigne;
	uint32_t distance;
	uint32_t tours;
} MOTEURS_EtatDiff;

MOTEURS_EtatMoteur MOTEURS_EtatMoteurGauche, MOTEURS_EtatMoteurDroit = {0};
MOTEURS_EtatDiff MOTEURS_EtatDifferentiel = {0};

#define MOTEUR_Kp 		300

/***** Tasks part *****/

/* Tache moteurs (gestion des messages) */
StaticTask_t xTaskMoteurs;
StackType_t xStackMoteurs[ STACK_SIZE ];
TaskHandle_t xHandleMoteurs = NULL;
void MOTEURS_TachePrincipale(void* params);

/* Tache moteurs périodique (asservissement) */
StaticTask_t xTaskMoteursAsservissement;
StackType_t xStackMoteursAsservissement[ STACK_SIZE ];
TaskHandle_t xHandleMoteursAsservissement = NULL;
void MOTEURS_TacheAsservissement( void* params ) ;

/* Fonctions diverses */
void MOTEURS_Set(int16_t cmdGauche, int16_t cmdDroit);
void MOTEURS_DesactiveAlim(void);
void MOTEURS_ActiveAlim(void);
int16_t MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteur etat);

/**
 * @brief Fonction d'initialisation des moteurs
 *
 */
void MOTEURS_Init(void) {
	/* Désactive les alimentations des moteurs */
	MOTEURS_DesactiveAlim();

	/* Create the task without using any dynamic memory allocation. */
	xHandleMoteurs = xTaskCreateStatic(
			MOTEURS_TachePrincipale,       /* Function that implements the task. */
			"MOTEURS Principale",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityMoteursHandler,/* Priority at which the task is created. */
			xStackMoteurs,          /* Array to use as the task's stack. */
			&xTaskMoteurs);  /* Variable to hold the task's data structure. */
	vTaskResume(xHandleMoteurs);

	/* Create the task without using any dynamic memory allocation. */
	xHandleMoteursAsservissement = xTaskCreateStatic(
			MOTEURS_TacheAsservissement,       /* Function that implements the task. */
			"MOTEURS Asservissement",          /* Text name for the task. */
			STACK_SIZE,      /* Number of indexes in the xStack array. */
			NULL,    /* Parameter passed into the task. */
			PriorityMoteursAsservissement,/* Priority at which the task is created. */
			xStackMoteursAsservissement,          /* Array to use as the task's stack. */
			&xTaskMoteursAsservissement);  /* Variable to hold the task's data structure. */
	vTaskSuspend(xHandleMoteursAsservissement); // On ne lance la tache d'asservissement que lorsque'une commande moteur arrive

	MOTEURS_DesactiveAlim();
}

void MOTEURS_Avance(uint32_t distance) {
	static uint32_t dist;

	if (distance) {
		dist = distance;
		MOTEURS_ActiveAlim();
		MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_MOVE, APPLICATION_Mailbox, (void*)&dist);
	} else
		MOTEURS_Stop();
}

void MOTEURS_Tourne(uint32_t tours) {
	static uint32_t turns;

	if (tours) {
		turns = tours;
		MOTEURS_ActiveAlim();
		MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_TURN, APPLICATION_Mailbox, (void*)&turns);
	} else
		MOTEURS_Stop();
}

void MOTEURS_Stop(void) {
	MOTEURS_DesactiveAlim();
	MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_STOP, APPLICATION_Mailbox, (void*)NULL);
}

/*
 * @brief Tache de supervision des moteurs
 * 		  Gestion de la boite aux lettres moteurs, et supervision generale
 * @params params non utilisé
 */
void MOTEURS_TachePrincipale(void* params) {
	MESSAGE_Typedef msg;
	uint32_t distance, tours;

	while (1) {
		msg = MESSAGE_ReadMailbox(MOTEURS_Mailbox);

		switch (msg.id) {
		case MSG_ID_MOTEURS_MOVE:
			distance = *((uint32_t*)msg.data);
			MOTEURS_EtatDifferentiel.distance = distance;
			MOTEURS_EtatDifferentiel.tours = 0;

			MOTEURS_EtatMoteurGauche.consigne=50;
			MOTEURS_EtatMoteurDroit.consigne=50;

			vTaskResume(xHandleMoteursAsservissement);
			break;
		case MSG_ID_MOTEURS_TURN:
			tours = *((uint32_t*)msg.data);
			MOTEURS_EtatDifferentiel.distance = 0;
			MOTEURS_EtatDifferentiel.tours = tours;

			MOTEURS_EtatMoteurGauche.consigne=50;
			MOTEURS_EtatMoteurDroit.consigne=50;

			vTaskResume(xHandleMoteursAsservissement);
			break;
		case MSG_ID_MOTEURS_STOP:

			MOTEURS_EtatDifferentiel.distance = 0;
			MOTEURS_EtatDifferentiel.tours = 0;

			MOTEURS_EtatMoteurGauche.consigne=0;
			MOTEURS_EtatMoteurDroit.consigne=0;
			if ((MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurGauche) ==0) &&
					(MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurDroit) ==0))
				// Les moteurs sont déjà arrêtés
				vTaskSuspend(xHandleMoteursAsservissement);
			else
				// Les moteurs tournent encore
				vTaskResume(xHandleMoteursAsservissement);
			break;
		default:
			break;
		}
	}
}

/*
 * @brief Tache d'asservissement, périodique (10ms)
 *
 * @params params non utilisé
 */
void MOTEURS_TacheAsservissement( void* params ) {
	TickType_t xLastWakeTime;
	int16_t erreurG, erreurD =0;
	int16_t encodeurGauche, encodeurDroit;
	int32_t locCmdG, locCmdD;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		// Wait for the next cycle.
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(MOTEURS_PERIODE_ASSERVISSEMENT));

		encodeurGauche = MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurGauche);
		encodeurDroit = MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurDroit);

		/*
		 * encodeur est entre -32768 et +32767, selon le sens de rotation du moteur
		 * consigne est entre -32768 et + 32767 selon le sens de rotation du moteur
		 * erreur est entre -32768 et 32767 selon la difference à apporter à la commande
		 */

		erreurG = MOTEURS_EtatMoteurGauche.consigne - encodeurGauche;
		erreurD = MOTEURS_EtatMoteurDroit.consigne - encodeurDroit;

		if (((MOTEURS_EtatMoteurDroit.consigne ==0) && (MOTEURS_EtatMoteurGauche.consigne ==0)) &&
				((erreurD==0) && (erreurG==0))) {

			MOTEURS_DesactiveAlim();
			vTaskSuspend(xHandleMoteursAsservissement);
		}

		if (MOTEURS_EtatMoteurGauche.consigne ==0)
			MOTEURS_EtatMoteurGauche.commande =0;
		else {
			if (erreurG !=0) {
				//locCmdG = (int32_t)MOTEURS_EtatMoteurGauche.commande + ((int32_t)MOTEUR_Kp*(int32_t)erreurG)/100;
				locCmdG = ((int32_t)MOTEUR_Kp*(int32_t)erreurG)/100;

				if (MOTEURS_EtatMoteurGauche.consigne>=0) {
					if (locCmdG<0) MOTEURS_EtatMoteurGauche.commande=0;
					else if (locCmdG>SHRT_MAX) MOTEURS_EtatMoteurGauche.commande=SHRT_MAX;
					else MOTEURS_EtatMoteurGauche.commande=(int16_t)locCmdG;
				} else {
					if (locCmdG>0) MOTEURS_EtatMoteurGauche.commande=0;
					else if (locCmdG<SHRT_MIN) MOTEURS_EtatMoteurGauche.commande=SHRT_MIN;
					else MOTEURS_EtatMoteurGauche.commande=(int16_t)locCmdG;
				}
			}
		}

		if (MOTEURS_EtatMoteurDroit.consigne ==0)
			MOTEURS_EtatMoteurDroit.commande =0;
		else {
			if (erreurD !=0) {
				//locCmdD = (int32_t)MOTEURS_EtatMoteurDroit.commande + ((int32_t)MOTEUR_Kp*(int32_t)erreurD)/100;
				locCmdD = ((int32_t)MOTEUR_Kp*(int32_t)erreurD)/100;

				if (MOTEURS_EtatMoteurDroit.consigne>=0) {
					if (locCmdD<0) MOTEURS_EtatMoteurDroit.commande=0;
					else if (locCmdD>SHRT_MAX) MOTEURS_EtatMoteurDroit.commande=SHRT_MAX;
					else MOTEURS_EtatMoteurDroit.commande=(int16_t)locCmdD;
				} else {
					if (locCmdD>0) MOTEURS_EtatMoteurDroit.commande=0;
					else if (locCmdD<SHRT_MIN) MOTEURS_EtatMoteurDroit.commande=SHRT_MIN;
					else MOTEURS_EtatMoteurDroit.commande=(int16_t)locCmdD;
				}
			}
		}

		/* Finalement, on applique les commandes aux moteurs */
		MOTEURS_Set(MOTEURS_EtatMoteurGauche.commande, MOTEURS_EtatMoteurDroit.commande);
	}
}

typedef struct {
	uint16_t encodeur;
	uint16_t correction;
} MOTEURS_CorrectionPoint;

#define MOTEURS_MAX_CORRECTION_POINTS 16

const MOTEURS_CorrectionPoint MOTEURS_CorrectionPoints[MOTEURS_MAX_CORRECTION_POINTS]=
{
		{MOTEURS_MAX_ENCODEUR-1, 1},
		{42000,   100},
		{22000,  2500},
		{18000,  5000},
		{16500,  7500},
		{15500, 10000},
		{14500, 12500},
		{13000, 15000},
		{12500, 17500},
		{12200, 20000},
		{11500, 22500},
		{11100, 25000},
		{11000, 27500},
		{10900, 29000},
		{10850, 30500},
		{10800, SHRT_MAX} // 32767
};

/*
 * @brief Fonction de conversion des valeurs brutes de l'encodeur en valeur linearisées
 *
 * @param encodeur valeur brute de l'encodeur
 * @return valeur linéarisée (entre -32768 et 32767)
 */
int16_t MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteur etat) {
	int16_t correction=0;
	uint8_t index=0;
	uint32_t A,B,C;
	uint16_t encodeur = etat.encodeur;

	if (encodeur ==MOTEURS_MAX_ENCODEUR)
		correction =0;
	else { // recherche par dichotomie de l'intervale
		while (index <MOTEURS_MAX_CORRECTION_POINTS) {
			if ((MOTEURS_CorrectionPoints[index].encodeur>=encodeur) && (MOTEURS_CorrectionPoints[index+1].encodeur<encodeur)) {
				// valeur trouvée, on sort
				break;
			} else
				index++;
		}

		if (index >= MOTEURS_MAX_CORRECTION_POINTS)
			correction = SHRT_MAX;
		else {
			A = encodeur-MOTEURS_CorrectionPoints[index+1].encodeur;
			B = MOTEURS_CorrectionPoints[index+1].correction-MOTEURS_CorrectionPoints[index].correction;
			C = MOTEURS_CorrectionPoints[index].encodeur-MOTEURS_CorrectionPoints[index+1].encodeur;

			correction = (int16_t)(MOTEURS_CorrectionPoints[index+1].correction - (uint16_t)((A*B)/C));
		}
	}

	/*
	 * Selon le sens de rotation du moteur (commande > 0 ou < 0), on corrige le signe du capteur
	 */
	if (etat.consigne<0)
		correction = -correction;

	return correction;
}

/**
 *
 */
void MOTEURS_DesactiveAlim(void) {
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM2);
	LL_TIM_DisableCounter(TIM21);

	LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2|LL_TIM_CHANNEL_CH3|LL_TIM_CHANNEL_CH4);

	LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_DisableChannel(TIM21, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2);

	LL_TIM_DisableIT_CC1(TIM2);
	LL_TIM_DisableIT_CC1(TIM21);
	LL_TIM_DisableIT_UPDATE(TIM2);
	LL_TIM_DisableIT_UPDATE(TIM21);

	LL_GPIO_SetOutputPin(GPIOB, SHUTDOWN_ENCODERS_Pin);
	LL_GPIO_ResetOutputPin(GPIOB, SHUTDOWN_5V_Pin);
}

/**
 *
 */
void MOTEURS_ActiveAlim(void) {
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM2);
	LL_TIM_EnableCounter(TIM21);

	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2|LL_TIM_CHANNEL_CH3|LL_TIM_CHANNEL_CH4);

	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM21, LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH2);

	LL_TIM_EnableIT_CC1(TIM2);
	LL_TIM_EnableIT_CC1(TIM21);
	LL_TIM_EnableIT_UPDATE(TIM2);
	LL_TIM_EnableIT_UPDATE(TIM21);

	LL_GPIO_ResetOutputPin(GPIOB, SHUTDOWN_ENCODERS_Pin);
	LL_GPIO_SetOutputPin(GPIOB, SHUTDOWN_5V_Pin);
}

/**
 * @brief Active les encodeurs et le régulateur des moteur si nécessaire et
 *        règle la commande du moteur (entre -MOTEURS_MAX_COMMANDE et +MOTEURS_MAX_COMMANDE)
 *        On applique une "regle de 3"
 *        pour SHRT_MAX -> MOTEURS_MAX_COMMANDE
 *        pour 0 -> 0
 *        pour une commande C dans l'interval [0 .. 32767], la commande est
 *        	commande = (C * MOTEURS_MAX_COMMANDE)/32767
 */
void MOTEURS_Set(int16_t cmdGauche, int16_t cmdDroit) {
	int32_t locValGauche, locValDroit;

	locValGauche = (int32_t)(((int32_t)cmdGauche * (int32_t)SHRT_MAX)/((int32_t)MOTEURS_MAX_COMMANDE));
	locValDroit = (int32_t)(((int32_t)cmdDroit * (int32_t)SHRT_MAX)/((int32_t)MOTEURS_MAX_COMMANDE));

	if (LL_GPIO_IsOutputPinSet(GPIOB, SHUTDOWN_5V_Pin)==GPIO_PIN_RESET)
		MOTEURS_ActiveAlim();

	// Moteur droit
	if (cmdDroit >=0) {
		LL_TIM_OC_SetCompareCH2(TIM3, (uint32_t)locValDroit);
		LL_TIM_OC_SetCompareCH1(TIM3, (uint32_t)0);
	} else {
		LL_TIM_OC_SetCompareCH2(TIM3, (uint32_t)0);
		LL_TIM_OC_SetCompareCH1(TIM3, (uint32_t)locValDroit);
	}

	// Moteur gauche
	if (cmdGauche >=0) {
		LL_TIM_OC_SetCompareCH4(TIM3, (uint32_t)locValGauche);
		LL_TIM_OC_SetCompareCH3(TIM3, (uint32_t)0);
	} else {
		LL_TIM_OC_SetCompareCH4(TIM3, (uint32_t)0);
		LL_TIM_OC_SetCompareCH3(TIM3, (uint32_t)locValGauche);
	}
}

/*
 * @brief Recupere les mesures brutes des encodeurs et les enregistre dans la structure moteur correspondante
 *
 * @param htim pointeur sur la reference du timer qui generé l'interruption
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance==TIM21) { /* moteur gauche */
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if (MOTEURS_EtatMoteurGauche.moteurLent !=0) {
				MOTEURS_EtatMoteurGauche.encodeur = MOTEURS_MAX_ENCODEUR;
				MOTEURS_EtatMoteurGauche.encodeurFront = MOTEURS_MAX_ENCODEUR;
			} else {
				MOTEURS_EtatMoteurGauche.encodeur = (uint16_t)LL_TIM_IC_GetCaptureCH1(TIM21);
				MOTEURS_EtatMoteurGauche.encodeurFront = (uint16_t)LL_TIM_IC_GetCaptureCH2(TIM21);
			}

			if (LL_TIM_IsActiveFlag_UPDATE(TIM21))
				LL_TIM_ClearFlag_UPDATE(TIM21);

			MOTEURS_EtatMoteurGauche.moteurLent = 0;
		}
	} else { /* moteur droit */
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if (MOTEURS_EtatMoteurDroit.moteurLent !=0) {
				MOTEURS_EtatMoteurDroit.encodeur = MOTEURS_MAX_ENCODEUR;
				MOTEURS_EtatMoteurDroit.encodeurFront = MOTEURS_MAX_ENCODEUR;
			} else {
				MOTEURS_EtatMoteurDroit.encodeur = (uint16_t)LL_TIM_IC_GetCaptureCH1(TIM2);
				MOTEURS_EtatMoteurDroit.encodeurFront = (uint16_t)LL_TIM_IC_GetCaptureCH2(TIM2);
			}

			if (LL_TIM_IsActiveFlag_UPDATE(TIM2))
				LL_TIM_ClearFlag_UPDATE(TIM2);

			MOTEURS_EtatMoteurDroit.moteurLent = 0;
		}
	}
}

/*
 * @brief Gestionnaire d'interruption "overflow"
 * 		  Lorsque deux interruptions "overflow" sont arrivées sans que l'interruption capture n'arrive,
 * 		  cela signifie que le moteur est à l'arret.
 * 		  On met la valeur de l'encodeur à MOTEURS_MAX_ENCODEUR
 *
 * @param htim pointeur sur la reference du timer qui generé l'interruption
 */
void MOTEURS_TimerEncodeurUpdate (TIM_HandleTypeDef *htim) {
	if (htim->Instance==TIM21) { /* moteur gauche */
		if ((MOTEURS_EtatMoteurGauche.moteurLent++) >=1) {
			MOTEURS_EtatMoteurGauche.encodeur = MOTEURS_MAX_ENCODEUR;
			MOTEURS_EtatMoteurGauche.moteurLent = 1;
		}
	} else { /* moteur droit */
		if ((MOTEURS_EtatMoteurDroit.moteurLent++) >=1) {
			MOTEURS_EtatMoteurDroit.encodeur = MOTEURS_MAX_ENCODEUR;
			MOTEURS_EtatMoteurDroit.moteurLent = 1;
		}
	}
}
