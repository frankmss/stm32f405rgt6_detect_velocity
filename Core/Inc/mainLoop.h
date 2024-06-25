#ifndef __mainLoop_h__
#define __mainLoop_h__


#include "main.h"

//200 mean 1ms
#define ADC_BUF_T_SIZE 100
#define ADCCAPOK (0x00005555)
#define ADCCAPWAIT (0X00003333)
#define CALOK    (0X0000FFFF)
#define UART_TRI_OK ADCCAPOK
#define UART_TRI_WAIT ADCCAPWAIT

struct adc_buf_t{
	int32_t adc_cap_ok;  //1:adc_cap_ok; 0:calculate ok;
	uint32_t buf[ADC_BUF_T_SIZE];
	int32_t adcMeanVal;
	int32_t adcDif2;
};

//用于通过uart2向host发送过程数据
#pragma pack(1)
struct log_para_t{
	char     sbs;
	uint16_t mean_adc_val;
	int32_t dac_val_p;
	int32_t dac_val_n;
	uint32_t checkBits;
};
#pragma pack()

void mainLoop(void);
void mainLoop1(void);
void preMainLoop1(void);
//void MX_USART2_UART_Init(void);
#endif
