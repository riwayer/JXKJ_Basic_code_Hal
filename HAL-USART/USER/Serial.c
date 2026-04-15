#include "Serial.h"
#include "stm32f1xx_hal_uart.h"
#include "usart.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
#include <sys/_intsup.h>
#include <sys/reent.h>
#include <stdarg.h>


uint8_t Serial_TxPacket[4];				//定义发送数据包数组，数据包格式：FF 01 02 03 04 FE
uint8_t Serial_RxPacket[8];				//定义接收数据包数组
uint8_t Serial_RxFlag;					//定义接收数据包标志位
uint8_t RxData;
uint8_t reCompleted = 0;    //判断HAL_UART_Receive_IT是否完成接收

/**
  * 函    数：串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void Serial_Init(void)
{
	HAL_UART_Init(&huart1);						//使能USART1，串口开始运行
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Serial_SendByte(uint8_t Byte)
{
	const uint8_t* pData = &Byte;
	HAL_UART_Transmit(&huart1, pData, 1, 300);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	HAL_UART_Transmit(&huart1, Array, Length, 300);
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  * 建    议：通过char *str = "Hello World";直接定义字符串
  */
void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		// uint8_t pData = String[i];
		// Serial_SendByte(&pData);		//依次调用Serial_SendByte发送每个字节数据
	}
	HAL_UART_Transmit(&huart1, (const uint8_t*)String, i, 300);
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

// /**
//   * 函    数：使用printf需要重定向的底层函数
//   * 参    数：保持原始格式即可，无需变动
//   * 返 回 值：保持原始格式即可，无需变动
//   */
// int fputc(int ch, FILE *f)
// {
// 	Serial_SendByte(ch);			//将printf的底层重定向到自己的发送字节函数
// 	return ch;
// }

// /**
//   * 函    数：自己封装的prinf函数
//   * 参    数：format 格式化字符串
//   * 参    数：... 可变的参数列表
//   * 返 回 值：无
//   */
// void Serial_Printf(char *format, ...)
// {
// 	char String[100];				//定义字符数组
// 	va_list arg;					//定义可变参数列表数据类型的变量arg
// 	va_start(arg, format);			//从format开始，接收参数列表到arg变量
// 	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
// 	va_end(arg);					//结束变量arg
// 	Serial_SendString(String);		//串口发送字符数组（字符串）
// }

/**
  * 函    数：串口发送数据包
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，Serial_TxPacket数组的内容将加上包头（FF）包尾（FE）后，作为数据包发送出去
  */
void Serial_SendPacket(void)
{
	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 4);
	Serial_SendByte(0xFE);
}

/**
  * 函    数：获取串口接收数据包标志位
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  */
uint8_t Serial_GetRxFlag(void)
{
	HAL_UART_Receive_IT(&huart1, &RxData, 1);	//这个函数调用一次后自动关闭中断接受来着，判断是否完成时随便开一下
	if (Serial_RxFlag == 1)			//如果标志位为1
	{
		Serial_RxFlag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

/**
  * 函    数：HAL_UART_Receive_IT的回调函数，为了实现接受不定长数据，这里的处理方法为将读出的数据复制到缓冲区
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1) {
		reCompleted = 1;	//接受完成,
	}
}

//或许可以开启IDLE中断，后面慢慢看？

/**
  * 函    数：处理接受的数据，不定长接受
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  * 注    意：这个还需要在Core->Src->Core/Src/stm32f1xx_it.c的 USART1_IRQHandler函数中调用，
  */
void on_UART_IDLE(UART_HandleTypeDef *huart)
{
	static uint8_t RxState = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRxPacket = 0;	//定义表示当前接收数据位置的静态变量
	if (reCompleted) {
		if (RxState == 0)
		{
			if (RxData == 0xFF)			//如果数据确实是包头
			{
				RxState = 1;			//置下一个状态
				pRxPacket = 0;			//数据包的位置归零
			}
		}
		/*当前状态为1，接收数据包数据*/
		else if (RxState == 1)
		{
			if (RxData == 0xFE) {
				RxState = 0;			//状态归0
				for (uint8_t i = 0; i<(8 - pRxPacket); i++){
					Serial_RxPacket[pRxPacket + i] = 0;//其他清0				
				}
				Serial_RxFlag = 1;		//接收数据包标志位置1，成功接收一个数据包
			}
			else {
				Serial_RxPacket[pRxPacket] = RxData;	//将数据存入数据包数组的指定位置
				pRxPacket ++;				//数据包的位置自增
			}
		}
	}
}

