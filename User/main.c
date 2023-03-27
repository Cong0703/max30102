#include "stm32f10x.h"
#include "exti.h"
#include "systick.h"
#include "iic.h"
#include "Font.h"
#include "OLED_IIC.h"
#include "MAX30102.h"
#include "algorithm.h"
#include "blood.h"
#include "hr.h"
float read_spo2_num=0;
uint8_t interrupt;

int main(void)
{	
	int i=0;
	IIC_GPIO_INIT();
  OLED_Init();	
	fill_picture(0x00); // clear screen
	MAX30102_GPIO();
	
	Max30102_reset();
	
	MAX30102_Config();
	
	for(i = 0;i < 128;i++) 
	{
		while(MAX30102_INTPin_Read()==0)
		{
			max30102_read_fifo();
		}
	}
	while(1)
	{	
		blood_Loop();
	}
}
