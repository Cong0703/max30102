#include "blood.h"
uint16_t g_fft_index = 0;  
uint8_t status=0;

struct compx s1[FFT_N+16];           
struct compx s2[FFT_N+16];           	

struct
{
	float 	Hp	;			
	float 	HpO2;			
	
}g_BloodWave;

BloodData g_blooddata = {0};					

#define CORRECTED_VALUE			47   			

/*funcation start ------------------------------------------------------------*/
void blood_data_update(void)
{

	g_fft_index=0;
	while(g_fft_index < FFT_N)
	{
		while(MAX30102_INTPin_Read()==0)
		{
			max30102_read_fifo();  //read from MAX30102 FIFO2
			if(g_fft_index < FFT_N)
			{
				s1[g_fft_index].real = fifo_red;
				s1[g_fft_index].imag= 0;
				s2[g_fft_index].real = fifo_ir;
				s2[g_fft_index].imag= 0;
				g_fft_index++;
			}
		}
	}

}

//ѪҺ��Ϣת��
extern float read_spo2_num;

void blood_data_translate(void)
{	
	float n_denom;
	uint16_t i;

	float dc_red =0; 
	float dc_ir =0;
	float ac_red =0; 
	float ac_ir =0;
	
	for (i=0 ; i<FFT_N ; i++ ) 
	{
		dc_red += s1[i].real ;
		dc_ir +=  s2[i].real ;
	}
		dc_red =dc_red/FFT_N ;
		dc_ir =dc_ir/FFT_N ;
	for (i=0 ; i<FFT_N ; i++ )  
	{
		s1[i].real =  s1[i].real - dc_red ; 
		s2[i].real =  s2[i].real - dc_ir ; 
	}
	

	for(i = 1;i < FFT_N-1;i++) 
	{
			n_denom= ( s1[i-1].real + 2*s1[i].real + s1[i+1].real);
			s1[i].real=  n_denom/4.00; 
			
			n_denom= ( s2[i-1].real + 2*s2[i].real + s2[i+1].real);
			s2[i].real=  n_denom/4.00; 			
	}

	for(i = 0;i < FFT_N-8;i++) 
	{
			n_denom= ( s1[i].real+s1[i+1].real+ s1[i+2].real+ s1[i+3].real+ s1[i+4].real+ s1[i+5].real+ s1[i+6].real+ s1[i+7].real);
			s1[i].real=  n_denom/8.00; 
			
			n_denom= ( s2[i].real+s2[i+1].real+ s2[i+2].real+ s2[i+3].real+ s2[i+4].real+ s2[i+5].real+ s2[i+6].real+ s2[i+7].real);
			s2[i].real=  n_denom/8.00; 
		
	}
//	printf("************8 pt Moving Average ir*************************************************************\r\n");

	g_fft_index = 0;	
	FFT(s1);
	FFT(s2);
	
	for(i = 0;i < FFT_N;i++) 
	{
		s1[i].real=sqrtf(s1[i].real*s1[i].real+s1[i].imag*s1[i].imag);
		s1[i].real=sqrtf(s2[i].real*s2[i].real+s2[i].imag*s2[i].imag);
	}

	for (i=1 ; i<FFT_N ; i++ ) 
	{
		ac_red += s1[i].real ;
		ac_ir +=  s2[i].real ;
	}

	int s1_max_index = find_max_num_index(s1, 30);
	int s2_max_index = find_max_num_index(s2, 30);

		float Heart_Rate = 60.00 * ((100.0 * s1_max_index )/ 512.00);
		
	   	g_blooddata.heart = Heart_Rate;
			
			float R = (ac_ir*dc_red)/(ac_red*dc_ir);
			float sp02_num =-45.060*R*R+ 30.354 *R + 94.845;
			g_blooddata.SpO2 = sp02_num;	
}


void blood_Loop(void)
{
	blood_data_update();
	blood_data_translate();
	OLED_Printf_EN(0,0,"Hr:%3d/min",g_blooddata.heart);
	g_blooddata.SpO2 = (g_blooddata.SpO2 > 99.99) ? 99.99:g_blooddata.SpO2;
	OLED_Printf_EN(2,0,"SpO2:%2.2f%%",g_blooddata.SpO2);
	Delay_ms(10);
}

