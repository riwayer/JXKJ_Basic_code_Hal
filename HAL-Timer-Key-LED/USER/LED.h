#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <sys/_intsup.h>

void LED_Init(void);
void LED1_SetMode(uint8_t Mode);
void LED2_SetMode(uint8_t Mode);
void LED_Tick(void);

#endif
