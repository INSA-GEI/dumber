/**
 ******************************************************************************
 * @file    debug.h
 * @author  Sebastien DI MERCURIO
 * @version V1.0
 * @date    25-juillet-2017
 * @brief   Envoi de données sur la liaison ITM
 *
 *
 ******************************************************************************
 ******************************************************************************
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "stm32f10x.h"                  // Device header

void SendUInt32(uint32_t data);
void SendString(char* data);
void SendFloat(float data);
void SendChar(char data);
void SendUint16 (uint16_t data);
void SendUint8 (uint8_t data);

#endif /* __DEBUG_H__ */
