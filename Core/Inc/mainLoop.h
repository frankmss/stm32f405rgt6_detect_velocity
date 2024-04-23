#ifndef __mainLoop_h__
#define __mainLoop_h__


#include "main.h"

//200 mean 1ms
#define ADC_BUF_T_SIZE 100
#define ADCCAPOK (0x00005555)
#define CALOK    (0X0000FFFF)
struct adc_buf_t{
	int32_t adc_cap_ok;  //1:adc_cap_ok; 0:calculate ok;
	uint32_t buf[ADC_BUF_T_SIZE];
	int32_t adcMeanVal;
	int32_t adcDif2;
};

void mainLoop(void);
void mainLoop1(void);
#endif
