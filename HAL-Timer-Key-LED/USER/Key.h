#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

#include "stm32f103xb.h"
// #include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <sys/_intsup.h>

void Key_Init(void);
uint8_t Key_GetNum(void);
void Key_Tick(void);

#endif
