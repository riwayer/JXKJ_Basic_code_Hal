#ifndef __SERIAL_BLE_H
#define __SERIAL_BLE_H

#include "stm32f1xx_hal.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint8_t BlueSerial_TxPacket[4];
extern uint8_t BlueSerial_RxPacket[50];
// extern uint8_t RxData;
extern uint8_t BlueSerial_proRxPacket[50];
extern uint8_t BlueSerial_RxFlag;

void BlueSerial_Init(void);
void BlueSerial_SendByte(uint8_t Byte);
void BlueSerial_SendArray(uint8_t *Array, uint16_t Length);
void BlueSerial_SendString(char *String);
void BlueSerial_SendNumber(uint32_t Number, uint8_t Length);
void BlueSerial_Printf(char *format, ...);

void BlueSerial_SendPacket(void);
uint8_t BlueSerial_GetRxFlag(void);

#endif
