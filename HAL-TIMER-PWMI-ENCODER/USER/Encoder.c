#include "Encoder.h"
#include "stm32f1xx_hal_tim.h"
#include "tim.h"

/**
  * 函    数：编码器初始化
  * 参    数：无
  * 返 回 值：无
  */
void Encoder_Init(void)
{
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_2);
}

/**
  * 函    数：获取编码器的增量值
  * 参    数：无
  * 返 回 值：自上此调用此函数后，编码器的增量值
  */
int16_t Encoder_Get(void)
{
	/*使用Temp变量作为中继，目的是返回CNT后将其清零*/
	int16_t Temp;
	Temp = __HAL_TIM_GET_COUNTER(&htim1);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	return Temp;
}
