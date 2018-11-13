/**
 ******************************************************************************
 * @file    motor.c
 * @author  Lucien Senaneuch
 * @version V1.0
 * @date    16-mai-2016
 * @brief   Gestion des moteurs du robots
 *			Ce fichier contient les fonctions lié à la gestions du moteur :
 *			- L'initialisation des PIN utilisé par les moteurs et encodeurs
 * 			- L'initialisation du Timer2 servant de bases de temps aux PWM pour
 * 			  moteur1 et moteur2
 *			- L'initialisation du Timer3 servant à échantilloner les capteurs magnétique de position
 *			  des 2 moteurs.
 * 			- La commandes des moteurs dans leurs 3 modes de fonctionnement (AVANT, ARRIERE, FREIN)
 * 			  et de leurs vitesse (valeur de PWM entre 0 et 255).
 *
 * @note			TABLE DE VERITEE DU DRIVER MOTEUR
 *
 *	ENABLE 			|			INPUTS			| 		 Moteurs
 *					|							|
 *		0			|				X			| Roue Libre
 *--------------------------------------------------------------------
 *					|  CMDA & !CMDB 			| Avant
 *		1			|  !CMDA & CMDB				| Arrière
 *					|  CMDA = CMDB				| Frein
 ******************************************************************************
 ******************************************************************************
 */

#include <stm32f10x.h>

#include "system_dumby.h"
#include "motor.h"

/**
 * @brief 	Variables globales nombre d'incrémentation de capteur de position de la roue droite.
 * @note 	tourPositionG correspond exactement à la même valeur.
 */
uint16_t tourD = 0;

/**
 * @brief 	Variables globales nombre d'incrémentation de capteur de position de la roue gauche.
 * @note 	tourPositionG correspond exactement à la même valeur.
 */
uint16_t tourG = 0;
uint16_t tourPositionD;
uint16_t tourPositionG;

/**
 * @brief 	Variables globales des consignes de vitesses du moteur droit.
 * @note 	Variables utilisées dans le programme principal main.c
 */
uint16_t G_speedRight=20;

/**
 * @brief 	Variables globales des consignes de vitesses du moteur gauche.
 * @note 	Variables utilisées dans le programme principal main.c
 */
uint16_t G_speedLeft=20;

/**
 * @brief 	Variables globales des consignes de position du moteur gauche.
 * @note 	Variables utilisées dans le programme principal main.c
 */
uint16_t G_lapsLeft;

/**
 * @brief 	Variables globales des consignes de position du moteur droit.
 * @note 	Variables utilisées dans le programme principal main.c
 */
uint16_t G_lapsRight;

float integration1 = 0;
float integration2 = 0;
/**
 * @brief  Coefficient PI
 */
const float kp = 15;
/**
 * @brief  Coefficient PI
 */
const float ki = 1.5;
/**
 * @brief  Valeurs de vitesse
 */
float motD = 0, motG = 0;
/**
 * @brief  erreurs entre vitesse réelle et moteur (droite)
 */
int erreurD;

/**
 * @brief  erreurs entre vitesse réelle et moteur (gauche)
 */
int erreurG;

uint16_t asservissement;
uint16_t regulation_vitesseD, regulation_vitesseG;
/** @addtogroup Projects
 * @{
 */

/** @addtogroup Robot Motor
 * @{
 */

/** @addtogroup Init Init GPIO - IT - Timers
 * @{
 */

/**
 * @brief 		Assigne et définis le GPIO nécessaire pour le moteur.
 *
 * 				La fonction MAP_MotorPin va venir configurer le E/S du GPIO pour correspondre avec
 * 				le schéma électrique en ressource.
 * @note			2 pwm en alternate fonction : PA1,PA2. 4 entr�e timer.
 *				PA8,PA9,PA10,PA11. 4 sortie ppull PB12,PB13,PB14,PB15
 *
 * @param  		None
 * @retval 		None
 *
 */
void motorConfigure(void)
{
    GPIO_InitTypeDef Init_Structure;
    TIM_TimeBaseInitTypeDef TIM2_TempsPWMsettings;
    TIM_OCInitTypeDef TIM2_Configure;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Configure les PIN A1 et A2 en output / alternate fonction
    Init_Structure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;

    Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &Init_Structure);

    // Configure les PIN B12,B13,B14, et B15 en output ppull.
    Init_Structure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &Init_Structure);

    // Configure les PIN A12 en output ppull - enable encodeurs
    Init_Structure.GPIO_Pin = GPIO_Pin_12;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &Init_Structure);

    // Configure les PIN A8,A9,A10, et A11 en input floating.
    Init_Structure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
    Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &Init_Structure);

    // Configuration du timer 2 (pwm moteurs*
    // On souhaite une résolution du PWM de 256 valeurs MOTOR1 TIM2
    TIM2_TempsPWMsettings.TIM_Period = 255;
    TIM2_TempsPWMsettings.TIM_Prescaler = 0;
    TIM2_TempsPWMsettings.TIM_ClockDivision=0;
    TIM2_TempsPWMsettings.TIM_CounterMode=TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM2_TempsPWMsettings);

    // Configuration du PWM sur le timer 2
    TIM2_Configure.TIM_OCMode=TIM_OCMode_PWM2;
    TIM2_Configure.TIM_OutputState = TIM_OutputState_Enable;
    TIM2_Configure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM2_Configure.TIM_Pulse = 256; // Constante initialisée à 256, pour un rapport cyclique nul
    TIM2_Configure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC3Init(TIM2, &TIM2_Configure);
    TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);

    TIM_OC2Init(TIM2, &TIM2_Configure);
    TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    // Enable Counter
    TIM_Cmd(TIM2, ENABLE);

    TIM_CtrlPWMOutputs(TIM2,ENABLE);

    // Configuration de la capture de l'encodeur 1
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM1, &TIM_ICInitStructure);
    TIM_Cmd(TIM1, ENABLE);
    TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);

    // Configuration de la capture de l'encodeur 1
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;

    TIM_ICInit(TIM1, &TIM_ICInitStructure);
    TIM_Cmd(TIM1, ENABLE);
    TIM_ITConfig(TIM1, TIM_IT_CC3, ENABLE);

    // Enable the TIM1 Capture interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // RAZ des variables
    regulation_vitesseD = 0;
    regulation_vitesseG = 0;
    asservissement =0;
}

/**
 * @}
 */

/** @addtogroup Regulation Reg Speed
 * @{
 */
/**
 * @brief  Commande de plus haut niveau pour contrôler le moteur droit.
 *
 * @param  Mode de fonctionnement du moteur, peut être égal aux constantes BRAKE, FORWARD, REVERSE définis dans moteur.h
 * @param  Consigne de vitesse du moteur, défini par un pwm entre 0 et 255.
 * @retval None
 */					

void motorCmdRight(char mod, uint16_t pwm)
{
    pwm = 256 - pwm;

    switch (mod) {
        case BRAKE:
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            break;
        case FORWARD:
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            break;

        case REVERSE:
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            break;

        default:
            GPIO_ResetBits(GPIOB, 12);
            GPIO_ResetBits(GPIOB, 13);
    }
    TIM_SetCompare3(TIM2, pwm);
}

/**
 * @brief  Commande de plus haut niveau pour contrôler le moteur Gauche.
 *
 * @param  Mode de fonctionnement du moteur, peut être égal aux constantes BRAKE, FORWARD, REVERSE définis dans moteur.h
 * @param  Consigne de vitesse du moteur, défini par un pwm entre 0 et 255.
 * @retval None
 */

void motorCmdLeft(char mod, uint16_t pwm) {
    pwm = 256 - pwm;
    switch (mod) {
        case BRAKE:
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            break;
        case FORWARD:
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            break;

        case REVERSE:
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            break;

        default:
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }
    TIM_SetCompare2(TIM2, pwm);
}

/**
 * @brief  Commande de plus haut niveau pour contrôler la vitesse moteur Gauche.
 *
 * @param  Consigne de vitesse du moteur, défini par un pwm entre 0 et 255.
 * @retval None
 */
void motorSpeedUpdateLeft(uint16_t pwm) {
    pwm = 256 - pwm;
    TIM_SetCompare2(TIM2, pwm);
}

/**
 * @brief  Commande de plus haut niveau pour contrôler la vitesse moteur Droit.
 *
 * @param  Consigne de vitesse du moteur, défini par un pwm entre 0 et 255.
 * @retval None
 */
void motorSpeedUpdateRight(uint16_t pwm) {
    pwm = 256 - pwm;
    TIM_SetCompare3(TIM2, pwm);
}

/**
 * @brief  Régulation des deux moteurs.
 *
 * 		   Modifie le le mode de fonctionnement des roues, puis
 * 		   va mettre à jour les variables globale G_laps* et G_speed*
 * 		   utilisé dans main.c pour l'asservissement.
 *
 * @param  Mode de fonctionnement la roue droite (BRAKE, REVERSE, FORWARD)
 * @param  Mode de fonctionnement la roue droite (BRAKE, REVERSE, FORWARD)
 * @param  Nombre de tour de roue droite à effectuer.
 * @param  Nombre de tour de roue gauche à effectuer.
 * @param  Vitesse de la roue de droite.
 * @param  Vitesse de la roue de gauche.
 *
 * @retval None
 */
void motorRegulation(char modRight, char modLeft, uint16_t lapsRight,
        uint16_t lapsLeft, uint16_t speedRight, uint16_t speedLeft) {
    /*Moteur Droit*/
    switch (modRight) {
        case BRAKE:
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            break;
        case FORWARD:
            GPIO_SetBits(GPIOB, GPIO_Pin_12);
            GPIO_ResetBits(GPIOB, GPIO_Pin_13);
            break;

        case REVERSE:
            GPIO_SetBits(GPIOB, GPIO_Pin_13);
            GPIO_ResetBits(GPIOB, GPIO_Pin_12);
            break;

        default:
            GPIO_ResetBits(GPIOB, 12);
            GPIO_ResetBits(GPIOB, 13);
    }

    /* Moteur Gauche */
    switch (modLeft) {
        case BRAKE:
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            break;
        case FORWARD:
            GPIO_SetBits(GPIOB, GPIO_Pin_15);
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            break;

        case REVERSE:
            GPIO_SetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            break;

        default:
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }

    if ((speedRight == 0 && lapsRight > 0)
            || (speedLeft == 0 && lapsLeft > 0)) {
        while (1);
    }

    G_lapsLeft = lapsLeft;
    G_speedLeft = speedLeft;
    G_lapsRight = lapsRight;
    G_speedRight = speedRight;
    asservissement = 1;
    tourPositionD = 0;
    tourPositionG = 0;
}

/**
 * @brief  Gestion de l'interuption des capteurs de positions.
 *
 *		   Fonction de callback permettant d'incrémenter le nombre
 *		   de tour des deux roues.
 *
 * @param  None
 * @retval None
 */
void TIM1_CC_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_CC1) == SET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
        tourD++;
        tourPositionD++;
    }

    if (TIM_GetITStatus(TIM1, TIM_IT_CC3) == SET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);
        tourG++;
        tourPositionG++;
    }
}

/**
 * @brief  Gestion de l'asservissement des moteurs
 *
 * @param  None
 * @retval None
 */
void motorManagement(void) {
    if (regulation_vitesseD) {
        erreurD = (signed int) G_speedRight - (signed int) tourD;
        motD = kp * erreurD + integration1;
        integration1 += ki * erreurD;

        if (motD > 255) motD = 255;

        if (motD < 0) motD = 0;

        motD = (uint16_t) motD;
        motorSpeedUpdateRight(motD);
        tourD = 0;
        regulation_vitesseD = 0;

        if (G_lapsRight - tourPositionD < 0) {
            motorCmdRight(BRAKE, 255);
        }
    }

    if (regulation_vitesseG) {
        erreurG = (signed int) G_speedLeft - (signed int) tourG;
        motG = kp * erreurG + integration2;

        integration2 += ki * erreurG;

        if (motG > 255) motG = 255;

        if (motG < 0) motG = 0;

        motG = (uint16_t) motG;

        motorSpeedUpdateLeft(motG);
        tourG = 0;
        regulation_vitesseG = 0;

        if (G_lapsLeft - tourPositionG < 0) {
            motorCmdLeft(BRAKE, 255);
        }
    }

    if (G_lapsLeft - tourPositionG < 0 && G_lapsRight - tourPositionD < 0
            && asservissement == 1) {

        motorCmdLeft(BRAKE, 255);
        motorCmdRight(BRAKE, 255);
        asservissement = 0;
        erreurD = 0;
        erreurG = 0;
        integration1 = 0;
        integration2 = 0;
        Dumber.busyState = FALSE;
        Dumber.cpt_inactivity = 0;
    }
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
/*void regulationCBK(void)
{
	if (regulation_vitesseD == 1)
	{
		erreurD = (signed int)G_speedRight - (signed int)tourD;
		motD = kp * erreurD +integration1;

		integration1 += ki * erreurD;	

		if (motD>255)	motD=255;

		if (motD<0)	motD=0;		

		motD=(uint16_t)motD;	
		majVitesseMotorD(motD);	
		tourD = 0;	
		regulation_vitesseD=0;

		if (G_lapsRight-tourPositionD < 0)	cmdRightMotor(BRAKE,0);
	}

	if (regulation_vitesseG == 1)
	{	
		erreurG = (signed int)G_speedLeft - (signed int)tourG;
		motG = kp* erreurG + integration2;

		integration2 += ki * erreurG;

		if(motG>255) motG=255;

		if(motG<0) motG=0;

		motG=(uint16_t)motG;

		majVitesseMotorG(motG);
		tourG = 0;
		regulation_vitesseG=0;

		if (G_lapsLeft-tourPositionG < 0)	cmdLeftMotor(BRAKE,0);
	}

	if (G_lapsLeft-tourPositionG < 0 && G_lapsRight-tourPositionD < 0 && asservissement == 1)
	{
		cmdLeftMotor(BRAKE,0);
		cmdRightMotor(BRAKE,0);

		asservissement = 0;
		erreurD=0;
		erreurG=0;
		integration1=0;
		integration2=0;
		Dumber.busyState=FALSE;
		Dumber.cpt_inactivity = 0;
	}
}*/

