/**
 ******************************************************************************
 * @file    debug.c
 * @author  Sébastien DI MERCURIO
 * @version V1.0
 * @date    25-juillet-2017
 * @brief   Envoi de données sur la liaison ITM
 *
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <stm32f10x.h>                  // Device header
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

/** @addtogroup Projects
 * @{
 */

/** @addtogroup Debug
 * @{
 */

/**
 * @brief		Envoi d'une chaine de caractere sur la liaison de debug
 *
 * @param  		data: pointeur sur une chaine
 * @retval 		Aucun
 */
void SendString(char* data)
{
    int i=0;

    while (data[i]!=0)
    {
        ITM_SendChar ((uint32_t)data[i]);
        i++;
    }

    ITM_SendChar ((uint32_t)'\n');
    ITM_SendChar ((uint32_t)'\r');
}

/**
 * @brief		Envoi d'un flottant sur la liaison de debug
 *
 * @param  		data: flottant
 * @retval 		Aucun
 */
void SendFloat(float data)
{
    char str[50];

    sprintf (str, "%f", data);
    SendString(str);
}

/**
 * @brief		Envoi d'un caractere sur la liaison de debug
 *
 * @param  		data: caractere
 * @retval 		Aucun
 */
void SendChar(char data)
{
    ITM_SendChar ((uint32_t)data);
    SendString ("\n\r");
}

/**
 * @brief		Envoi d'un entier non signé 32 bits sur la liaison de debug
 *
 * @param  		data: entier non signé 32 bits
 * @retval 		Aucun
 */
void SendUInt32(uint32_t data)
{ 
    char str[50];

    sprintf (str, "%u", (unsigned int)data);
    SendString(str);
}

/**
 * @brief		Envoi d'un entier non signé 16 bits sur la liaison de debug
 *
 * @param  		data: entier non signé 16 bits
 * @retval 		Aucun
 */
void SendUint16 (uint16_t data)
{
    char str[50];

    sprintf (str, "%u", data);
    SendString(str);
}

/**
 * @brief		Envoi d'un entier non signé 8 bits sur la liaison de debug
 *
 * @param  		data: entier non signé 8 bits
 * @retval 		Aucun
 */
void SendUint8 (uint8_t data)
{
    char str[50];

    sprintf (str, "%u", data);
    SendString(str);
}

/**
 * @}
 */

/**
 * @}
 */
