#include "OLED.h"
#include "OLED_Data.h"
#include "i2c.h"
#include <string.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
#include <sys/_intsup.h>


void OLED_WriteCommand(uint8_t Command)
{
    uint8_t CommandArray[] = {0x00, Command};
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, CommandArray, 2, HAL_MAX_DELAY);
}

void OLED_WriteData(uint8_t Data)
{
    uint8_t DataArray[] = {0x40, Data}; //0x40,Control Byte
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, DataArray, 2, HAL_MAX_DELAY);
}


void OLED_Init(void)
{
    MX_I2C1_Init();
    //HAL_Delay(100);

    /*写入一系列的命令，对OLED进行初始化配置*/
    OLED_WriteCommand(0xAE);	//设置显示开启/关闭，0xAE关闭，0xAF开启

    OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);	//0x00~0xFF

    OLED_WriteCommand(0xA8);	//设置多路复用率
    OLED_WriteCommand(0x3F);	//0x0E~0x3F
    
    OLED_WriteCommand(0xD3);	//设置显示偏移
    OLED_WriteCommand(0x00);	//0x00~0x7F

    OLED_WriteCommand(0x40);	//设置显示开始行，0x40~0x7F

    OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常，0xA0左右反置

    OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常，0xC0上下反置

    OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);	//设置对比度
    OLED_WriteCommand(0xCF);	//0x00~0xFF

    OLED_WriteCommand(0xD9);	//设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

    OLED_WriteCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

    OLED_WriteCommand(0x8D);	//设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);	//开启显示

    //HAL_Delay(100);
}

void OLED_SetCursor(uint8_t X, uint8_t Page)
{
    /*通过指令设置页地址和列地址*/
    OLED_WriteCommand(0xB0 | Page);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
        OLED_SetCursor(0, j);
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_WriteData(0x00);	//将显存数组数据全部清零
		}
	}
}

void OLED_ShowChar(uint8_t X, uint8_t Page, char Char, uint8_t FontSize)
{
    if (FontSize == 6)
    {
        OLED_SetCursor(X, Page);
        for (uint8_t i = 0; i < 6; i++) {
            OLED_WriteData(OLED_F6x8[Char - ' '][i]);
        }
    }
    else if (FontSize == 8) {
        OLED_SetCursor(X, Page);
        for (uint8_t i = 0; i < 8; i++) {
            OLED_WriteData(OLED_F8x16[Char - ' '][i]);
        }
        OLED_SetCursor(X, Page + 1);
        for (uint8_t i = 0; i < 8; i++){
            OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
        }
    }
}

void OLED_ShowString(uint8_t X, uint8_t Page, char *String, uint8_t Fomsize)
{
    for (uint8_t i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(X + i * Fomsize, Page, String[i], Fomsize);
    }
}

void OLED_ShowImage(uint8_t X, uint8_t Page, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    for (uint8_t j = 0; j < Height; j++) {
        OLED_SetCursor(X, Page + j);
        for (uint8_t i = 0; i < Width; i++)
        {
            OLED_WriteData(Image[Width * j + i]);   //这里图片是用一维数组表示的？
        }
    
    }
}

void OLED_ShowChinese(uint8_t X, uint8_t Page, char *Chinese)   //逻辑为把汉字拆分为一个个独立的汉字。
{                                                           // 之后遍历汉字的字模，一一匹配汉字的索引，最后取出字模的数据，把它显示出来
    char SingleChinese[4] = {0};                            //独立的汉字（UTF8里占3位）+ 一位的结束
    uint8_t pChinese = 0;                                   //指针，取到字符串的第几个汉字
    uint8_t pIndex;
    
    for (uint8_t i = 0; Chinese[i] != '\0'; i++) {
        SingleChinese[pChinese] = Chinese[i];
        pChinese++;

        if (pChinese >= 3)
        {
            pChinese = 0;
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++)
            {
                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChinese) == 0)
                {
                    break;
                }
            }

            OLED_ShowImage(X + ((i+1) / 3 - 1) * 16, Page, 16, 2, OLED_CF16x16[pIndex].Data);
        }
    }


}