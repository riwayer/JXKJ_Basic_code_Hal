#include "LED.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <sys/_intsup.h>

uint8_t LED1_Mode;
uint8_t LED2_Mode;

uint16_t LED1_Count;
uint16_t LED2_Count;



void LED1_SetMode(uint8_t Mode)
{
	if (Mode != LED1_Mode)
	{
		LED1_Mode = Mode;
		LED1_Count = 0;
	}
}

void LED2_SetMode(uint8_t Mode)
{
	if (Mode != LED2_Mode)
	{
		LED2_Mode = Mode;
		LED2_Count = 0;
	}
}

void LED1_ON(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void LED1_OFF(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}



