#include "Serial_BLE.h"
#include <stdint.h>


#define RX_CMD_LEN 1

uint8_t BlueSerial_TxPacket[4];				//定义发送数据包数组，数据包格式：FF 01 02 03 04 FE
uint8_t BlueSerial_RxPacket[50];			//定义理后的接收数据包数组
uint8_t BlueSerial_RxFlag;					//定义接收数据包处理完成标志位。
uint8_t BlueSerial_proRxPacket[50];			//定义处接收数据包数组
uint8_t reCompleted = 0;    //判断HAL_UART_Receive_IT是否完成接收

/**
  * 函    数：串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void BlueSerial_Init(void)
{
	HAL_UART_Init(&huart2);						//使能USART1，串口开始运行
	// HAL_UART_Receive_IT(&huart1, BlueSerialRxPacket, RX_CMD_LEN);//开启中断接收
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2,BlueSerial_proRxPacket,sizeof(BlueSerial_proRxPacket));//开启DMA接收，接收不定长数据
	//检测到不再发数据就进入这个函数，sizeof(receiveData)，接收数据的大小，该完成后会进入回调函数。
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void BlueSerial_SendByte(uint8_t Byte)
{
	const uint8_t* pData = &Byte;
	HAL_UART_Transmit(&huart2, pData, 1, 300);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void BlueSerial_SendArray(uint8_t *Array, uint16_t Length)
{
	HAL_UART_Transmit(&huart2, Array, Length, 300);
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  * 建    议：通过char *str = "Hello World";直接定义字符串
  */
void BlueSerial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		// uint8_t pData = String[i];
		// BlueSerialSendByte(&pData);		//依次调用BlueSerialSendByte发送每个字节数据
		BlueSerial_SendByte(String[i]);
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t BlueSerial_Pow(uint32_t X, uint32_t Y)
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
void BlueSerial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		BlueSerial_SendByte(Number / BlueSerial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void BlueSerial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	BlueSerial_SendString(String);	//蓝牙串口发送字符数组（字符串）
}
// /**
//   * 函    数：自己封装的prinf函数
//   * 参    数：format 格式化字符串
//   * 参    数：... 可变的参数列表
//   * 返 回 值：无
//   */
// void BlueSerialPrintf(char *format, ...)
// {
// 	char String[100];				//定义字符数组
// 	va_list arg;					//定义可变参数列表数据类型的变量arg
// 	va_start(arg, format);			//从format开始，接收参数列表到arg变量
// 	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
// 	va_end(arg);					//结束变量arg
// 	BlueSerialSendString(String);		//串口发送字符数组（字符串）
// }

/**
  * 函    数：串口发送数据包
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，BlueSerialTxPacket数组的内容将加上包头（FF）包尾（FE）后，作为数据包发送出去
  */
void BlueSerial_SendPacket(void)
{
	BlueSerial_SendByte(0xFF);
	BlueSerial_SendArray(BlueSerial_proRxPacket, 4);
	BlueSerial_SendByte(0xFE);
}

/**
  * 函    数：获取串口接收数据包标志位
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  */
uint8_t BlueSerial_GetRxFlag(void)
{
	// HAL_UART_Receive_IT(&huart1, &RxData, 1);	//这个函数调用一次后自动关闭中断接受来着，判断是否完成时随便开一下
	if (BlueSerial_RxFlag == 1)			//如果标志位为1
	{
		BlueSerial_RxFlag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

void USART_BLE_Run(void)
{
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;
	if (reCompleted == 1)
	{	
		reCompleted = 0;
		for (uint8_t i = 0; BlueSerial_proRxPacket[i] != '\0'; i++) {
			if (RxState == 0)
			{
				if (BlueSerial_proRxPacket[0] == '[' && BlueSerial_RxFlag == 0)
				{
					RxState = 1;
					pRxPacket = 0;
				}
			}
			else if (RxState == 1)
			{
				if (BlueSerial_proRxPacket[i] == ']')
				{
					RxState = 0;
					BlueSerial_RxPacket[pRxPacket] = '\0';
					BlueSerial_RxFlag = 1;
				}
				else
				{
					BlueSerial_RxPacket[pRxPacket] = BlueSerial_proRxPacket[i];
					pRxPacket ++;
				}
			}
			
		}
	}
}

/**
  * 函    数：HAL_UARTEx_ReceiveToIdle_DMA的回调函数，为了实现接受不定长数据，这里的处理方法为将读出的数据复制到缓冲区
  * 参    数：无
  * 返 回 值：串口接收数据包标志位，范围：0~1，接收到数据包后，标志位置1，读取后标志位自动清零
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == &huart2) {//huart->Instance == USART2
		reCompleted = 1;	//接受完成.	
		// HAL_UART_Transmit_DMA(&huart2,BlueSerialproRxPacket,Size);
		USART_BLE_Run();	//处理接收的数据包
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2,BlueSerial_proRxPacket,sizeof(BlueSerial_proRxPacket));//开启下一次DMA接收	
	}
}


/******************************************************************************/
/*注意事项																	   */
/******************************************************************************/

//printf()函数重定向到串口输出，需要将下面的函数复制到usart.c文件中。
/*

int _write(int fd, char *pBuffer,int size)
{
  //避免串口发送过程中的死循环，加入超时机制
  const uint32_t timeout = 100000; // 超时时间，单位为毫秒
  uint32_t timeout_counter = 0; // 超时计数器

  for (int i = 0; i < size; i++) {
    // 等待直到串口的数据寄存器空
    while ((USART2->SR & 0x40) == 0) {
      timeout_counter++;
      if (timeout_counter >= timeout) {
        // 如果超过超时限制，可以跳出并返回错误，或者做其他处理
        return -1; // 返回错误
      }
    }
    USART2->DR = (uint8_t)pBuffer[i]; // 写入数据寄存器，发送字符
  }
  return size; // 返回成功发送的字符数量
}

*/

/*
printf的内容后面要跟上\r\n,不然串口打印不显示
printf("hello,world");
printf(\r\n);
*/

/*
默认不能输出浮点数，因为编译器选项没有开启，在路径cmake/gcc-arm-none-eabi.cmake里配置
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections -u _printf_float")
也就是结尾加上-u _printf_float就可以了。
第29行
*/

/*
来自博客：https://www.cnblogs.com/dianmao/p/19294449
*/