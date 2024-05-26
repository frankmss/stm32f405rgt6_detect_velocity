#ifndef __MAX5307_FUNCTION_H
#define __MAX5307_FUNCTION_H


#include "main.h"
#include <stdint.h>

enum max5306_chanel{
	max_ch1 = 2,
	max_ch2 ,
	max_ch3 ,
	max_ch4,
	max_ch5,
	max_ch6,
	max_ch7,
	max_ch8
};

enum max5306_ouput{
	max_outenable = 1,
	max_outdisable ,
};

#define DACP_1     max_ch5
#define DACP_1K    max_ch6
#define DACP_1M    max_ch7

#define DACN_1M    	max_ch2
#define DACN_1K    max_ch3
#define DACN_1    max_ch4

void init_max5307(void);
void max5307_w_chanel(enum max5306_chanel chanel, uint32_t val, enum max5306_ouput enable);
void max5307_loadpin(void);
void max5307_enable_chanel(enum max5306_chanel chanel);
void max5307_disable_chanel(enum max5306_chanel chanel);
void max5307_enable_ori_chanel(uint16_t oriChanelNo);
#endif