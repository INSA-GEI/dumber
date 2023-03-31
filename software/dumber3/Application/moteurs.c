/*
 * moteurs.c
 *
 *  Created on: Sep 12, 2022
 *      Author: dimercur
 */

#include "moteurs.h"
#include "timers.h"

/*
 * Global informations
 * Main clock: 6 Mhz
 * Tim2 PWM Input (CH1): Encodeur droit PHB : 0 -> 65535
 * TIM21 PWM Input (CH1): Encodeur Gauche PHA: 0 -> 65535
 * TIM3: PWM Output moteur (0->200) (~30 Khz)
 */

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim21;
extern TIM_HandleTypeDef htim3;

#define MOTEURS_MAX_COMMANDE	200
#define MOTEURS_MAX_CONSIGNE	100
#define MOTEURS_MAX_ENCODEUR	65535

typedef struct {
	int16_t commande;
	int16_t consigne;
	uint16_t encodeur;
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
MOTEURS_EtatDiff MOTEURS_EtatDifferentiel;

uint16_t MOTEUR_DerniereValEncodeursG;
uint16_t MOTEUR_DerniereValEncodeursD;

#define MOTEUR_GAUCHE	0
#define MOTEUR_DROIT	1

#define MOTEUR_Kp 		15
#define MOTEUR_DELAY	3

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
GPIO_PinState MOTEURS_EtatAlim(void);
uint16_t MOTEURS_CorrectionEncodeur(uint16_t encodeur);

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
			PriorityMoteurs,/* Priority at which the task is created. */
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
}

void MOTEURS_Avance(uint32_t distance) {
	static uint32_t dist;

	dist = distance;
	MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_MOVE, APPLICATION_Mailbox, (void*)dist);
}

void MOTEURS_Tourne(uint32_t tours) {
	static uint32_t turns;

	turns = tours;
	MESSAGE_SendMailbox(MOTEURS_Mailbox, MSG_ID_MOTEURS_TURN, APPLICATION_Mailbox, (void*)turns);
}

void MOTEURS_Stop(void) {
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

			// TODO: trucs a faire ici
			vTaskResume(xHandleMoteursAsservissement);
			break;
		case MSG_ID_MOTEURS_TURN:
			tours = *((uint32_t*)msg.data);

			// TODO: trucs a faire ici
			vTaskResume(xHandleMoteursAsservissement);
			break;
		case MSG_ID_MOTEURS_STOP:
			// TODO: trucs a faire ici
			vTaskSuspend(xHandleMoteursAsservissement);
			break;
		default:
			break;
		}
	}
}

/*
 * @brief Tache d'asservissement, periodique (10ms)
 *
 * @params params non utilisé
 */
void MOTEURS_TacheAsservissement( void* params ) {
	TickType_t xLastWakeTime;
	int16_t deltaG, deltaD =0;

	uint16_t encodeurGauche, encodeurDroit;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		// Wait for the next cycle.
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(MOTEURS_PERIODE_ASSERVISSEMENT));

		encodeurGauche = MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurGauche.encodeur);
		encodeurDroit = MOTEURS_CorrectionEncodeur(MOTEURS_EtatMoteurDroit.encodeur);

		deltaG = MOTEURS_EtatMoteurGauche.consigne - encodeurGauche;
		deltaD = MOTEURS_EtatMoteurDroit.consigne - encodeurDroit;

		if (((MOTEURS_EtatMoteurDroit.consigne ==0) && (MOTEURS_EtatMoteurGauche.consigne ==0)) &&
				((deltaD==0) && (deltaG==0))) MOTEURS_DesactiveAlim();
		else MOTEURS_ActiveAlim();

		if (deltaG !=0) {
			MOTEURS_EtatMoteurGauche.commande = MOTEURS_EtatMoteurGauche.commande + MOTEUR_Kp*deltaG;
			if (MOTEURS_EtatMoteurGauche.consigne>=0) {
				if (MOTEURS_EtatMoteurGauche.commande>255) MOTEURS_EtatMoteurGauche.commande=255;
				if (MOTEURS_EtatMoteurGauche.commande<0) MOTEURS_EtatMoteurGauche.commande=0;
			} else {
				if (MOTEURS_EtatMoteurGauche.commande>0) MOTEURS_EtatMoteurGauche.commande=0;
				if (MOTEURS_EtatMoteurGauche.commande<-255) MOTEURS_EtatMoteurGauche.commande=-255;
			}
		}

		if (deltaD !=0) {
			MOTEURS_EtatMoteurDroit.commande = MOTEURS_EtatMoteurDroit.commande + MOTEUR_Kp*deltaD;
			if (MOTEURS_EtatMoteurDroit.consigne>=0) {
				if (MOTEURS_EtatMoteurDroit.commande>255) MOTEURS_EtatMoteurDroit.commande=255;
				if (MOTEURS_EtatMoteurDroit.commande<0) MOTEURS_EtatMoteurDroit.commande=0;
			} else {
				if (MOTEURS_EtatMoteurDroit.commande>0) MOTEURS_EtatMoteurDroit.commande=0;
				if (MOTEURS_EtatMoteurDroit.commande<-255) MOTEURS_EtatMoteurDroit.commande=-255;
			}
		}

		MOTEURS_Set(MOTEURS_EtatMoteurGauche.commande, MOTEURS_EtatMoteurDroit.commande);
	}
}

/**
 *
 */
void MOTEURS_DesactiveAlim(void) {
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_ENCODERS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_5V_Pin, GPIO_PIN_RESET);
}

/**
 *
 */
void MOTEURS_ActiveAlim(void) {
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_ENCODERS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, SHUTDOWN_5V_Pin, GPIO_PIN_SET);
}

/**
 *
 */
GPIO_PinState MOTEURS_EtatAlim(void) {
	return HAL_GPIO_ReadPin(GPIOB, SHUTDOWN_5V_Pin);
}

/**
 * @brief Active les encodeurs et le régulateur des moteur si nécessaire et
 *        règle la commande du moteur (entre -MOTEURS_MAX_CONSIGNE et +MOTEURS_MAX_CONSIGNE)
 */
void MOTEURS_Set(int16_t cmdGauche, int16_t cmdDroit) {
	uint8_t locValGauche, locValDroit;

	if (cmdGauche>=0) {
		if (cmdGauche>MOTEURS_MAX_CONSIGNE)
			locValGauche = MOTEURS_MAX_CONSIGNE;
		else
			locValGauche =(uint8_t)cmdGauche;
	} else {
		if (cmdGauche < -MOTEURS_MAX_CONSIGNE)
			locValGauche = MOTEURS_MAX_CONSIGNE;
		else
			locValGauche =(uint8_t)(-cmdGauche);
	}

	if (cmdDroit>=0) {
		if (cmdDroit>MOTEURS_MAX_CONSIGNE)
			locValDroit = MOTEURS_MAX_CONSIGNE;
		else
			locValDroit =(uint8_t)cmdDroit;
	} else {
		if (cmdDroit < -MOTEURS_MAX_CONSIGNE)
			locValDroit = MOTEURS_MAX_CONSIGNE;
		else
			locValDroit =(uint8_t)(-cmdDroit);
	}

	if (MOTEURS_EtatAlim()==GPIO_PIN_RESET)
		MOTEURS_ActiveAlim();

	// Moteur droit
	if (cmdDroit >=0) {
		htim2.Instance->CCR1 = (uint16_t)locValDroit;
		htim2.Instance->CCR2 = 0;
	} else {
		htim2.Instance->CCR2 = (uint16_t)locValDroit;
		htim2.Instance->CCR1 = 0;
	}

	// Moteur gauche
	if (cmdGauche >=0) {
		htim2.Instance->CCR4 = (uint16_t)locValGauche;
		htim2.Instance->CCR3 = 0;
	} else {
		htim2.Instance->CCR3 = (uint16_t)locValGauche;
		htim2.Instance->CCR4 = 0;
	}
}

/*
 * @brief Recupere les mesures brutes des encodeurs et les enregistre dans la structure moteur correspondante
 *
 * @param htim pointeur sur la reference du timer qui generé l'interruption
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance==TIM2) { /* moteur gauche */
		MOTEURS_EtatMoteurGauche.encodeur = (uint16_t)TIM2->CCR1;
		MOTEURS_EtatMoteurGauche.moteurLent = 0;
	} else { /* moteur droit */
		MOTEURS_EtatMoteurDroit.encodeur = (uint16_t)TIM21->CCR1;
		MOTEURS_EtatMoteurDroit.moteurLent = 0;
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
	if (htim->Instance==TIM2) { /* moteur gauche */
		if ((MOTEURS_EtatMoteurGauche.moteurLent++) >=2) {
			MOTEURS_EtatMoteurGauche.encodeur = MOTEURS_MAX_ENCODEUR;
			MOTEURS_EtatMoteurGauche.moteurLent = 0;
		}
	} else { /* moteur droit */
		if ((MOTEURS_EtatMoteurDroit.moteurLent++) >=2) {
			MOTEURS_EtatMoteurDroit.encodeur = MOTEURS_MAX_ENCODEUR;
			MOTEURS_EtatMoteurDroit.moteurLent = 0;
		}
	}

}

typedef struct {
	uint16_t encodeur;
	uint16_t correction;
} MOTEURS_CorrectionPoint;

#define MOTEURS_MAX_CORRECTION_POINTS 8

const MOTEURS_CorrectionPoint MOTEURS_CorrectionPoints[MOTEURS_MAX_CORRECTION_POINTS]=
{
		{MOTEURS_MAX_ENCODEUR-1, 1},
		{4000, 50},
		{1000, 80},
		{500, 200},
		{400, 400},
		{300, 1000},
		{200,3000},
		{0,MOTEURS_MAX_ENCODEUR}
};

/*
 * @brief Fonction de conversion des valeurs brutes de l'encodeur en valeur linearisées
 *
 * @param encodeur valeur brute de l'encodeur
 * @return valeur linearisée
 */
uint16_t MOTEURS_CorrectionEncodeur(uint16_t encodeur) {
	uint16_t correction=0;
	uint8_t index=0;
	int32_t pente, origine;

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

		pente = 1000*(MOTEURS_CorrectionPoints[index].encodeur-MOTEURS_CorrectionPoints[index+1].encodeur)/(MOTEURS_CorrectionPoints[index].correction - MOTEURS_CorrectionPoints[index+1].correction);
		origine = MOTEURS_CorrectionPoints[index].correction-((pente*MOTEURS_CorrectionPoints[index].encodeur)/1000);

		correction = origine - ((pente*encodeur)/1000);
	}

	return correction;
}
