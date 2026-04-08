#include "OLED.h"
#include "OLED_Data.h"
#include "i2c.h"
#include <string.h>
#include <math.h>
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
#include <sys/_intsup.h>

uint8_t OLED_DisplayBuf[8][128];//OLED显存数组

void OLED_WriteCommand(uint8_t Command)//写指令
{
    uint8_t CommandArray[] = {0x00, Command};
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, CommandArray, 2, HAL_MAX_DELAY);
}

void OLED_WriteData(const uint8_t *Data, uint16_t len)//写数据
{
    uint8_t buf[256];
    buf[0] = 0x40;
    // 把要发送的数据拷贝到 buf+1 的位置
    memcpy(buf + 1, Data, len);  
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, len + 1, HAL_MAX_DELAY);
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

void OLED_SetCursor(uint8_t X, uint8_t Page)//设置坐标
{
    /*通过指令设置页地址和列地址*/
    OLED_WriteCommand(0xB0 | Page);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

void OLED_Updata(void)
{
    for (int j=0; j<8; j++) {
        OLED_SetCursor(0, j);
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }

}

void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
        OLED_SetCursor(0, j);
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			// OLED_WriteData(0x00);	//将显存数组数据全部清零
            OLED_DisplayBuf[j][i] = 0x00;
		}
	}
}

void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;
	
	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)				//超出屏幕的内容不显示
			{
				OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	//将显存数组指定数据清零
			}
		}
	}
}

void OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == 6)
    {
        OLED_SetCursor(X, Y);
        OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
        // for (uint8_t i = 0; i < 6; i++) {
            // OLED_WriteData(OLED_F6x8[Char - ' '][i]);
            // OLED_DisplayBuf[Page][X + i] = OLED_F6x8[Char - ' '][i];
        // }
    }
    else if (FontSize == 8) {
        OLED_SetCursor(X, Y);
        OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
        // for (uint8_t i = 0; i < 8; i++) {
            // OLED_WriteData(OLED_F8x16[Char - ' '][i]);
            // OLED_DisplayBuf[Page][X + i] = OLED_F8x16[Char - ' '][i];
        // }
    }
}

void OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t Fomsize)
{
    for (uint8_t i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(X + i * Fomsize, Y, String[i], Fomsize);
    }
}

/*超级主要的函数，显示图像，实现跨页写入*/
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    OLED_ClearArea(X, Y, Width, Height);
    for (uint8_t j = 0; j < (Height - 1) / 8 + 1; j++) {
        for (uint8_t i = 0; i < Width; i++) {
            OLED_DisplayBuf[Y / 8 + j][X + i] |= Image[Width * j + i] << (Y % 8);
            OLED_DisplayBuf[Y / 8 + j + 1][X + i] |= Image[Width * j + i] >> (8 - Y % 8);
        }
    }
}

void OLED_ShowChinese(uint8_t X, uint8_t Y, char *Chinese)   //逻辑为把汉字拆分为一个个独立的汉字。
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

            OLED_ShowImage(X + ((i+1) / 3 - 1) * 16, Y, 16, 16, OLED_CF16x16[pIndex].Data);
            //现在是以实际的像素为单位。所以Height为16
        }
    }
}

void OLED_DrawPoint(uint8_t X, uint8_t Y)
{
    OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
}

uint8_t OLED_GetPoint(uint8_t X, uint8_t Y)
{
    if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
    {
        return  1;
    }
    return 0;
}

/*听完只想给江科大献上我的膝盖-在这里打开OLED教程-2026-04-06*/



