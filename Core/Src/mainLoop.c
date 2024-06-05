#include "mainLoop.h"

#include <stdio.h>
//#include "arm_math.h"
#include "ad9520_function.h"
#include "main.h"
#include "math.h"
#include "max5307.h"
extern UART_HandleTypeDef huart2;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim5;  // 用来做1s的定时器
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
// floating point support in printf isn't enabled by default. To enable, add -u
// _printf_float to your LDFLAGS.
// for ubuntu arm-none-eabi-gcc compile
int _write(int fd, char *ptr, int len) {
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

// for windows keil compile
//  int fputc(int ch, FILE *f)
//  {
//    /* Your implementation of fputc(). */
//  	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//    return ch;
//  }

#define CAPNUM 100
uint32_t chanl1_capNum[CAPNUM];
uint32_t chanl2_capNum[CAPNUM];
int32_t chanl1_xNum[CAPNUM];
int32_t chanl2_xNum[CAPNUM];
float chanl1_capNum_f[CAPNUM];
float chanl2_capNum_f[CAPNUM];
int32_t chanl1_completed = 0;
int32_t chanl2_completed = 0;
float average_xNum1 = 0, average_xNum2 = 0;
int32_t chanl1_midx = 0, chanl2_midx = 0;  // 保存循环DMA队列的中间变量
// tim4 dma
// 方式，获得数据分为两部分，分为前后两个halfword，每个halfword都代表一个上升沿的时间
//  这段代码是用来处理这两个halfword的
// 输入参数 canNum为两个halfword的组合数，有上个上升沿时间,
// 由于第一个数通常是错误的，所以从第二个数开始
void transform_capNum_to_xNum(uint32_t *capNum, int32_t *xNum,
                              int32_t capNum_len) {
  int32_t i;
  for (i = 0; i < capNum_len / 2; i++) {
    xNum[i * 2] = capNum[i] & 0xffff;
    xNum[i * 2 + 1] = capNum[i] >> 16;
  }
}

// 计算平均xNum，
// cal_average_xNum(chanl1_xNum, CAPNUM);
int32_t cal_average_xNum(int32_t *xNum, int32_t capNum_len) {
  int32_t i, sum = 0;
  for (i = 0; i < capNum_len - 1; i++) {
    xNum[i] = xNum[i + 1] - xNum[i];
    sum += xNum[i];
  }
  return sum / (capNum_len - 1);
}

void enable_tim4_capture_Iterrupt(void) {
  // HAL_TIM_Base_Start_IT(&htim4);
  // HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
  // HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
  // HAL_TIM_IC_Start_DMA(&htim4, TIM_CHANNEL_1, (uint32_t*)chanl1_capNum,
  // CAPNUM+4);

  // HAL_TIM_IC_Start_DMA(&htim4, TIM_CHANNEL_2, (uint32_t*)chanl2_capNum,
  // CAPNUM); HAL_TIM_IC_Start_DMA(&htim1, TIM_CHANNEL_4,
  // (uint32_t*)chanl1_capNum, CAPNUM);
  HAL_TIM_Base_Start_IT(&htim5);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim1);
}

uint32_t i = 0;
float f = 3.14;
// int32_t chanl1_capNum = 0;
// int32_t chanl2_capNum = 0;
int32_t fre_dif = 0;
int32_t xx, yy, complete_sign;
int32_t xx_cnt, yy_cnt;
uint8_t buf[4];
float v_dianya;
uint32_t dacV;
double tmp;
#define NS 128
uint32_t Wave_LUT[NS] = {
    2048, 2149, 2250, 2350, 2450, 2549, 2646, 2742, 2837, 2929, 3020, 3108,
    3193, 3275, 3355, 3431, 3504, 3574, 3639, 3701, 3759, 3812, 3861, 3906,
    3946, 3982, 4013, 4039, 4060, 4076, 4087, 4094, 4095, 4091, 4082, 4069,
    4050, 4026, 3998, 3965, 3927, 3884, 3837, 3786, 3730, 3671, 3607, 3539,
    3468, 3394, 3316, 3235, 3151, 3064, 2975, 2883, 2790, 2695, 2598, 2500,
    2400, 2300, 2199, 2098, 1997, 1896, 1795, 1695, 1595, 1497, 1400, 1305,
    1212, 1120, 1031, 944,  860,  779,  701,  627,  556,  488,  424,  365,
    309,  258,  211,  168,  130,  97,   69,   45,   26,   13,   4,    0,
    1,    8,    19,   35,   56,   82,   113,  149,  189,  234,  283,  336,
    394,  456,  521,  591,  664,  740,  820,  902,  987,  1075, 1166, 1258,
    1353, 1449, 1546, 1645, 1745, 1845, 1946, 2047};
#define PI 3.14159265
#define AMPLITUDE 2048.0  // 正弦波幅度
#define FREQUENCY 100.0   // 正弦波频率（Hz）
#define SAMPLE_RATE 4096  // 采样率（每秒采样次数）

void generateSinWave(int32_t i) {
  double omega = 2.0 * PI * FREQUENCY / SAMPLE_RATE;
  tmp = AMPLITUDE * sin(omega * i) + AMPLITUDE;
}
void mainLoop(void) {
  uint32_t dacN = 0;  //,dacV=0;

  int32_t dacSign = 1;
  init_max5307();
  max5307_w_chanel(max_ch8, 2048, max_outenable);
  //	while(1){
  //	    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  //   //HAL_Delay(3);
  //    HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  //   // HAL_Delay(300);
  //	HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
  //	//HAL_Delay(300);
  //		//void max5307_w_chanel(uint32_t chanel, uint32_t val);
  //
  //		//max5307_w_chanel(0x02,dacN);
  //		generateSinWave(dacN%SAMPLE_RATE);
  //		max5307_w_chanel(max_ch1,round(tmp),max_outenable);
  //		max5307_w_chanel(DACP_1K,round(tmp),max_outenable);
  //		//max5307_loadpin();
  //		//max5307_w_chanel(0x0e,0x2f);
  ////		if(dacN>=0xfff){
  ////			dacSign = -1;
  ////		}else if(dacN==0){
  ////			dacSign = 1;
  ////		}
  //		dacN = dacN + dacSign;

  ////		tmp = (dacN%4096) * 3.14 /4096;
  ////		tmp = sin(tmp);
  ////		dacV = 4096*tmp;
  //	}
  int32_t xx_bak = 0;
  int32_t yy_bak = 0;
  // init_ad9520_spiport();
  complete_sign = 0;
  xx = 0;
  yy = 0;
  fre_dif = 0;
  xx_cnt = 0;
  yy_cnt = 0;
  v_dianya = 1.65;
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  // HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                   (uint32_t)((v_dianya / 3.3) * 4096));
  // HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t
  // Size)
  HAL_UART_Receive_IT(&huart2, (uint8_t *)(buf), 1);
  init_ad9520();
  enable_tim4_capture_Iterrupt();
  while (1) {
    // do{

    // }while(0);

    i++;

    // HAL_DAC_SetValue(DAC_HandleTypeDef* hdac, uint32_t Channel, uint32_t
    // Alignment, uint32_t Data)

    // HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, i++);
    // init_ad9520();
    // set_ad9520_device();
    // HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    HAL_Delay(3);
    HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    HAL_Delay(300);
    check_ad9520_status();
    // printf("hello world %ld:float %f\n",i,f*i);
    fre_dif = chanl1_capNum - chanl2_capNum;

    while (1) {
      // if(chanl1_completed==1 ){
      //   chanl1_completed=0;
      //   transform_capNum_to_xNum((uint32_t *)(&chanl1_capNum[2]),
      //   chanl1_xNum, CAPNUM); average_xNum1 =
      //   (float)cal_average_xNum(chanl1_xNum, CAPNUM);
      //   //HAL_TIM_IC_Start_DMA(&htim4, TIM_CHANNEL_1,
      //   (uint32_t*)chanl1_capNum, CAPNUM);

      //   break;
      // }else if(chanl2_completed==1){
      //   chanl2_completed=0;
      //   //HAL_TIM_IC_Start_DMA(&htim4, TIM_CHANNEL_2,
      //   (uint32_t*)chanl2_capNum, CAPNUM); break;
      // }
      generateSinWave(dacN % SAMPLE_RATE);
      max5307_w_chanel(max_ch1, round(tmp), max_outenable);
      max5307_w_chanel(DACP_1, round(tmp), max_outenable);
      dacN = dacN + dacSign;
      if (v_dianya > 1) {
        v_dianya = 0;
      } else {
        v_dianya = 3.3;
      }
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                       (uint32_t)((v_dianya / 3.3) * 4096));
      HAL_Delay(100);
      HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
      // check_ad9520_status();
    }

    while (complete_sign == 0) {
      HAL_Delay(3);
    };
    complete_sign = 0;
    // fre_dif=chanl1_midx - chanl2_midx;
    // printf("%ld = %ld - %ld \n",fre_dif,xx,yy);
    // printf("cha1=%ld,cha2=%ld\n",xx-xx_bak,yy-yy_bak);
    xx_bak = xx;
    yy_bak = yy;

    HAL_Delay(2);
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
      // chanl1_capNum++;
      // chanl2_capNum = TIM4->CCR1;
      // xx = TIM4->CCR1;
      // for(i=1;i<CAPNUM;i++){
      //   chanl1_capNum[i-1]=chanl1_capNum[i]-chanl1_capNum[i-1];
      // }
      // yy = TIM4->CCR2;
      // chanl1_completed=1;
      chanl1_midx = chanl1_capNum[CAPNUM / 2 - 2] & 0xffff;
      // chanl2_midx = TIM4->CCR2;
      // htim->Channel &= ~HAL_TIM_ACTIVE_CHANNEL_1;
    }
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
      // chanl2_capNum++;
      chanl2_midx = chanl2_capNum[CAPNUM / 2 - 2] & 0xffff;
      // htim->Channel &= ~HAL_TIM_ACTIVE_CHANNEL_2;
    }
  } else if (htim->Instance == TIM1) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
      // chanl2_capNum++;
      chanl1_midx = chanl1_capNum[CAPNUM / 2 - 2] & 0xffff;
      // htim->Channel &= ~HAL_TIM_ACTIVE_CHANNEL_2;
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // htim4.Init.Period = 65535; 1ms 定时
  //  if(htim == &htim4){
  //    HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  //    xx= HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);

  //   yy = TIM4->CCR2;
  //   //TIM4->CCR1=0;
  //   //TIM4->CCR2=0;
  //   fre_dif = xx - yy;
  // }
  if (htim == &htim5) {
    xx = TIM4->CNT;
    // yy = TIM4->CCR2;
    yy = TIM1->CNT;
    fre_dif = xx - yy;
    complete_sign = 1;
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
  // if(htim == &htim1){
  //   xx_cnt++;
  // }
  // if(htim == &htim4){
  //   yy_cnt++;
  // }
}

// void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin)
// {
// 	if(GPIO_Pin == capture_sig_it_Pin){
// 	   //This block will be triggered after pin activated.
//      chanl2_capNum++;
// 	}
// }
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart2) {
    // xx++;
    // uint8_t tmp = (uint16_t)(huart->Instance->DR & (uint16_t)0x01FF);
    if (buf[0] == 'u') {
      // HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
      v_dianya += 0.1;
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                       (uint32_t)((v_dianya / 3.3) * 4096));
    } else if (buf[0] == 'd') {
      // HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
      v_dianya -= 0.1;
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                       (uint32_t)((v_dianya / 3.3) * 4096));
    } else if (buf[0] == 'r') {
      v_dianya += 0.01;
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                       (uint32_t)((v_dianya / 3.3) * 4096));
    } else if (buf[0] == 'x') {
      v_dianya -= 0.01;
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                       (uint32_t)((v_dianya / 3.3) * 4096));
    }
    HAL_UART_Receive_IT(&huart2, (uint8_t *)(buf), 1);
  }
}

void set_dac_initVal(uint32_t *val) {
  // set all dac val

  max5307_w_chanel(max_ch1, *(val + 0), max_outdisable);
  max5307_w_chanel(max_ch2, *(val + 1), max_outdisable);
  max5307_w_chanel(max_ch3, *(val + 2), max_outdisable);
  max5307_w_chanel(max_ch4, *(val + 3), max_outdisable);
  max5307_w_chanel(max_ch5, *(val + 4), max_outdisable);
  max5307_w_chanel(max_ch6, *(val + 5), max_outdisable);
  max5307_w_chanel(max_ch7, *(val + 6), max_outdisable);
  max5307_w_chanel(max_ch8, *(val + 7), max_outdisable);

  max5307_loadpin();
}
#define ADC_BUF_SIZE 4000  // 200=1ms
#define ADC_TIMES_N (ADC_BUF_SIZE * 2)
uint32_t value_adc1[ADC_BUF_SIZE], value_adc2[ADC_BUF_SIZE];
#define CP_TIMES_N (ADC_TIMES_N)
int32_t value_cp[CP_TIMES_N];

int32_t cal_k_value_cp(void) {  // cal k
  int32_t kUp = 0, kDown = 0;
  int32_t i = 0;
  for (i = 1; i < CP_TIMES_N; i++) {
    value_cp[i - 1] = value_cp[i] - value_cp[i - 1];
  }
  value_cp[CP_TIMES_N - 1] = 0;

  for (i = 0; i < CP_TIMES_N; i++) {
    if (abs(value_cp[i]) >= 5) {
      if (value_cp[i] > 0) {
        kUp += 1;
      } else {
        kDown += 1;
      }
    }
  }
  return (kDown - kUp);
}

int32_t cal_value_cp(void) {
  int32_t i = 0;
  int32_t tmp1, tmp2, tmp3, tmp4, allSum = 0;
  for (i = 0; i < CP_TIMES_N; i++) {
    tmp1 = (int32_t)(value_adc1[i / 2 + 0] & 0x0fff);
    // tmp2 = (int32_t)((value_adc1[i/2+0] >> 16)&(0x0fff));
    // tmp3 = (int32_t)(value_adc1[i*2+1] & 0xfff);
    // tmp4 = (int32_t)((value_adc1[i*2+1] >> 16)&(0x0fff));
    // value_cp[i] = (tmp1+tmp2)/2 + (tmp3+tmp4)/2;
    value_cp[i] = tmp1;
    allSum += value_cp[i];
  }
  return cal_k_value_cp();
  // return (allSum/CP_TIMES_N)-2048;
}

// 1Hz 10
// adcsumarray_size=2,1,1k收敛很快，
#define ADCSUMARRAY_SIZE (10)
// #define ADCSUMARRAY_SIZE (300)
int32_t adcSumArray[ADCSUMARRAY_SIZE];
int32_t pidP = 0, pidI = 0, pidD = 0;
float_t dletDac = 0;

// 1hz
// #define kP (-0.005)
// #define kI (+0.001)
// #define kD (0.005)
// void cal_cp_output_dac(struct adc_buf_t* adc_buf, uint32_t *dacP, uint32_t
// *dacN){ 	int32_t i=0; 	int32_t N=ADC_BUF_T_SIZE*2; 	int32_t
// allSum=0; 	uint32_t
// tmpUint32=0; 	for(i=0; i<N; i++){ 		allSum += (int32_t)
//((adc_buf->buf[i/2]&0x0fff));
//	}
//	allSum = allSum/N - TARGET_CPV;
//
//	for(i=0; i<ADCSUMARRAY_SIZE-1; i++){
//		adcSumArray[i] = adcSumArray[i+1];
//	}
//	adcSumArray[ADCSUMARRAY_SIZE-1] = allSum;
//
//	//pid
//	pidP = adcSumArray[ADCSUMARRAY_SIZE-1];
//	pidI = 0;
//	for(i=0; i<ADCSUMARRAY_SIZE; i++){
//		pidI += adcSumArray[i];
//	}
//	pidI = pidI/ADCSUMARRAY_SIZE;
//	pidD = adcSumArray[ADCSUMARRAY_SIZE-1] -
// adcSumArray[ADCSUMARRAY_SIZE-2]; 	dletDac = kP*((float_t)pidP) +
// kI*((float_t)pidI) + kD*((float_t)pidD); 	tmpUint32 = abs((int32_t)
// (dletDac));
//	(*dacP) += (int32_t) (dletDac);
//	(*dacN) -= (int32_t) (dletDac);
//	max5307_w_chanel(DACP_1,  (*dacP)>>12,      max_outenable);
//	max5307_w_chanel(DACP_1K, ((*dacP)&0xfff),  max_outenable);
//	max5307_w_chanel(DACN_1,  (*dacN)>>12,      max_outenable);
//	max5307_w_chanel(DACN_1K, ((*dacN)&0xfff),  max_outenable);
//
//	//clear buf
//	for(i=0; i<ADC_BUF_T_SIZE; i++){
//		adc_buf->buf[i]=0;
//	}
// }
// for test before 20231206
// #define kP (-0.0005)
// #define kI (+0.051)
// #define kD ( 0.00005)

// get jubuzhengdang, jiezhi
// #define kP (-0.0005)
// #define kI (+0.0051)
// #define kD (1.1275)

// for test 20231206 this is almost ok
#define kP (-0.0005)
#define kI (+0.0051)
#define kD (1.1275)

// #define kP (-0.0002)
// #define kI (+0.0031)
// #define kD (0.5275)
// #define TARGET_CPV  (2748)
#define TARGET_CPV (3096)
void cal_cp_output_dac(struct adc_buf_t *adc_buf, uint32_t *dacP,
                       uint32_t *dacN) {
  int32_t i = 0;
  int32_t N = ADC_BUF_T_SIZE * 2;
  int32_t allSum = 0;
  uint32_t tmpUint32 = 0;
  for (i = 0; i < N; i++) {
    allSum += (int32_t)((adc_buf->buf[i / 2] & 0x0fff));
  }
  allSum = allSum / N - TARGET_CPV;

  for (i = 0; i < ADCSUMARRAY_SIZE - 1; i++) {
    adcSumArray[i] = adcSumArray[i + 1];
  }
  adcSumArray[ADCSUMARRAY_SIZE - 1] = allSum;

  // pid
  pidP = adcSumArray[ADCSUMARRAY_SIZE - 1];
  pidI = 0;
  for (i = 0; i < ADCSUMARRAY_SIZE; i++) {
    pidI += adcSumArray[i];
  }
  pidI = pidI / ADCSUMARRAY_SIZE;
  pidD = adcSumArray[ADCSUMARRAY_SIZE - 1] - adcSumArray[ADCSUMARRAY_SIZE - 2];
  dletDac = kP * ((float_t)pidP) + kI * ((float_t)pidI) + kD * ((float_t)pidD);
  tmpUint32 = abs((int32_t)(dletDac));
  (*dacP) += (int32_t)(dletDac);
  (*dacN) -= (int32_t)(dletDac);
  max5307_w_chanel(DACP_1, (*dacP) >> 12, max_outenable);
  max5307_w_chanel(DACP_1K, ((*dacP) & 0xfff), max_outenable);
  max5307_w_chanel(DACN_1, (*dacN) >> 12, max_outenable);
  max5307_w_chanel(DACN_1K, ((*dacN) & 0xfff), max_outenable);

  // clear buf
  for (i = 0; i < ADC_BUF_T_SIZE; i++) {
    adc_buf->buf[i] = 0;
  }
}

#define VALMEANARRAY_N 20
int32_t valMeanArray[VALMEANARRAY_N];
// 计算valMeanArray序列中的均值和差值，这相当与adc—dma采样200次的平均处理
void insertMeanVal(int32_t val, struct adc_buf_t *adc_buf) {
  static int32_t currentP = 0;
  int32_t i = 0, sumval = 0, difval = 0;
  valMeanArray[currentP++] = val;
  currentP = currentP % VALMEANARRAY_N;
  for (i = 0; i < VALMEANARRAY_N - 1; i++) {
    sumval += valMeanArray[i];
    difval += abs(valMeanArray[i] - adc_buf->adcMeanVal);
  }
  sumval = sumval / VALMEANARRAY_N;
  difval = difval / VALMEANARRAY_N;
  adc_buf->adcMeanVal = sumval;
  adc_buf->adcDif2 = difval;
}
struct log_para_t
    logPara;  // 用于向host发送调试过程数据，在cal_cp_output_dac_ext过程中保存
int32_t meanVal_view = 0;
int32_t difVal_view = 0;
#define RUN_30S 180000  // about 90S
void cal_cp_output_dac_ext(struct adc_buf_t *adc_buf, uint32_t *dacP,
                           uint32_t *dacN, int32_t runTime500us) {
  int32_t i = 0;
  int32_t N = ADC_BUF_T_SIZE;
  uint32_t allSum = 0;
  uint16_t forSaveAllSum = 0;
  uint32_t tmpUint32 = 0;
  int32_t deltNoisV = 0;
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);
  for (i = 0; i < N; i++) {
    allSum += (uint32_t)((adc_buf->buf[i] & 0x0fff));
    allSum += (uint32_t)(((adc_buf->buf[i] >> 16) & 0x0fff));
  }
  allSum = allSum / (N * 2);
  allSum = (adc_buf->buf[99] & 0x0fff);
  forSaveAllSum = allSum;
  allSum = allSum - TARGET_CPV;
  insertMeanVal(allSum, adc_buf);
  meanVal_view = adc_buf->adcMeanVal;
  difVal_view = adc_buf->adcDif2;

  for (i = 0; i < ADCSUMARRAY_SIZE - 1; i++) {
    adcSumArray[i] = adcSumArray[i + 1];
  }
  adcSumArray[ADCSUMARRAY_SIZE - 1] = allSum;

  // pid
  pidP = adcSumArray[ADCSUMARRAY_SIZE - 1];
  pidI = 0;
  for (i = 0; i < ADCSUMARRAY_SIZE; i++) {
    pidI += adcSumArray[i];
  }
  pidI = pidI / ADCSUMARRAY_SIZE;
  pidD = adcSumArray[ADCSUMARRAY_SIZE - 1] - adcSumArray[ADCSUMARRAY_SIZE - 2];
  dletDac = kP * ((float_t)pidP) + kI * ((float_t)pidI) + kD * ((float_t)pidD);
  tmpUint32 = abs((int32_t)(dletDac));
  // if(runTime500us < RUN_30S) { //nomal operate
  // 	deltNoisV = 0;
  // }else { //add noise
  // 	if((runTime500us&0x01) != 0){ //2ms
  // 		deltNoisV = 3;
  // 		HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
  // 	}else{
  // 		deltNoisV = 0;
  // 		HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
  // 	}
  // }
  deltNoisV = 0;
  (*dacP) += (int32_t)(dletDac);
  (*dacN) -= (int32_t)(dletDac);
  (*dacP) += deltNoisV;
  (*dacN) -= deltNoisV;
  max5307_w_chanel(DACP_1, (*dacP) >> 12, max_outenable);
  max5307_w_chanel(DACP_1K, ((*dacP) & 0xfff), max_outenable);
  max5307_w_chanel(DACN_1, (*dacN) >> 12, max_outenable);
  max5307_w_chanel(DACN_1K, ((*dacN) & 0xfff), max_outenable);

  // clear buf
  for (i = 0; i < ADC_BUF_T_SIZE; i++) {
    adc_buf->buf[i] = 0;
  }
  logPara.mean_adc_val = forSaveAllSum;
  logPara.dac_val_n = (*dacN);
  logPara.dac_val_p = (*dacP);
  // HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
}

uint16_t forSaveAllSum = 0;
// 2024.4.26调整跟踪算法，以前的算法有可能会出现同步现象
#define TARGET_CPV_1 (2048)
void cal_cp_output_dac_ext_1(struct adc_buf_t *adc_buf, uint32_t *dacP,
                             uint32_t *dacN, int32_t runTime500us) {
  int32_t i = 0;
  int32_t N = ADC_BUF_T_SIZE;
  int32_t allSum = 0;

  uint32_t tmpUint32 = 0;
  int32_t deltNoisV = 0;
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);

  for (i = 0; i < N; i++) {
    allSum += (int32_t)((adc_buf->buf[i] & 0x0fff));
    allSum += (int32_t)(((adc_buf->buf[i] >> 16) & 0x0fff));
  }
  allSum = allSum / (N * 2);
  //allSum = adc_buf->buf[0];
  //  allSum = (adc_buf->buf[99]&0x0fff);
  forSaveAllSum = allSum;
  // max5307_w_chanel(DACP_1,  (((adc_buf->buf[0]) >> 0)&0xfff), max_outenable);
  // max5307_w_chanel(DACN_1,  (((adc_buf->buf[0]) >> 0)&0xfff), max_outenable);
  // return;
  allSum = allSum - TARGET_CPV_1;
  forSaveAllSum = abs(allSum);
  // insertMeanVal(allSum,adc_buf);
  // meanVal_view = adc_buf->adcMeanVal;
  // difVal_view  = adc_buf->adcDif2;

  for (i = 0; i < ADCSUMARRAY_SIZE - 1; i++) {
    adcSumArray[i] = adcSumArray[i + 1];
  }
  adcSumArray[ADCSUMARRAY_SIZE - 1] = allSum;

  // pid
  pidP = adcSumArray[ADCSUMARRAY_SIZE - 1];
  pidI = 0;
  for (i = 0; i < ADCSUMARRAY_SIZE; i++) {
    pidI += adcSumArray[i];
  }
  pidI = pidI / ADCSUMARRAY_SIZE;
  pidD = adcSumArray[ADCSUMARRAY_SIZE - 1] - adcSumArray[ADCSUMARRAY_SIZE - 2];
#define kP_1 (  -5.0100000105)
#define kI_1 (  -50.610551)
#define kD_1 ( -500.26)

// #define kP_1 (  -0.50100000105)
// #define kI_1 (  -5.0610551)
// #define kD_1 ( -50.1026)
  // 1,1k,1M三段可以收敛
  //  #define kP_1 (-1000.05)
  //  #define kI_1 (+1000.51)
  //  #define kD_1 (1000.1275)
  // 1,1k两段可以收敛
  //  #define kP (-0.0005)
  //  #define kI (+0.0051)
  //  #define kD (1.1275)
  dletDac = kP_1 * ((double_t)pidP) + kI_1 * ((double_t)pidI) +
            kD_1 * ((double_t)pidD);
  tmpUint32 = abs((int32_t)(dletDac));

  deltNoisV = 0;

  // (*dacP) += (int32_t)(dletDac);
  // (*dacN) -= (int32_t)(dletDac);
  // (*dacP) += deltNoisV;
  // (*dacN) -= deltNoisV;
  // max5307_w_chanel(DACP_1, ((*dacP)  >> 12)&0xfff, max_outdisable);
  // max5307_w_chanel(DACP_1K, (((*dacP)>>0) & 0xfff), max_outdisable);
  // max5307_w_chanel(DACN_1, ((*dacN)  >> 12)&0xfff, max_outdisable);
  // max5307_w_chanel(DACN_1K, (((*dacN)>>0) & 0xfff), max_outdisable);

  (*dacP) += (int32_t)(dletDac);
  (*dacN) -= (int32_t)(dletDac);
  (*dacP) += deltNoisV;
  (*dacN) -= deltNoisV;
  max5307_w_chanel(DACP_1, (((*dacP) >> 20) & 0xffc), max_outdisable);
  max5307_w_chanel(DACP_1K, (((*dacP) >> 10) & 0xffc), max_outdisable);
  max5307_w_chanel(DACP_1M, ((*dacP) & 0xfff) & 0xfff, max_outdisable);
  max5307_w_chanel(DACN_1, (((*dacN) >> 20) & 0xffc), max_outdisable);
  max5307_w_chanel(DACN_1K, (((*dacN) >> 10) & 0xffc), max_outdisable);
  max5307_w_chanel(DACN_1M, ((*dacN) & 0xfff) & 0xfff, max_outdisable);

  do {
    uint16_t chanelNo = 0;
    chanelNo = 0x01 << (DACP_1 + 2) | 0x01 << (DACP_1K + 2) |
               0x01 << (DACP_1M + 2) | 0x01 << (DACN_1 + 2) |
               0x01 << (DACN_1K + 2) | 0x01 << (DACN_1M + 2);
    max5307_enable_ori_chanel(chanelNo);
  } while (0);

  // clear buf
  for (i = 0; i < ADC_BUF_T_SIZE; i++) {
    adc_buf->buf[i] = 0;
  }
  logPara.mean_adc_val = forSaveAllSum;
  // logPara.mean_adc_val = pidI;
  logPara.dac_val_n = (*dacN);
  logPara.dac_val_p = (*dacP);
  // HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
}

// 2024.4.26调整跟踪算法，以前的算法有可能会出现同步现象
// 函数名void cal_cp_output_dac_ext_2
//(struct adc_buf_t *adc_buf, uint32_t *dacP,
//  uint32_t *dacN, int32_t runTime500us)
// 参数保持不变，只是调整了跟踪算法
// 具体思路，求原始adc_buf的导数，根据斜率调整dac输出
void cal_cp_output_dac_ext_2(struct adc_buf_t *adc_buf, uint32_t *dacP,
                             uint32_t *dacN, int32_t runTime500us) {
  int32_t i = 0;
  int32_t N = ADC_BUF_T_SIZE;

  uint32_t tmpUint32 = 0;
  int32_t deltNoisV = 0;
  int32_t difAdcValBuf[ADC_BUF_T_SIZE];
  int32_t tmpdif1, tmpdif2;
  int32_t allSum = 0;
  float_t allSumF = 0;
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);

  memset(difAdcValBuf, 0, sizeof(difAdcValBuf));
  for (i = 0; i < N; i++) {
    tmpdif1 = (int32_t)((adc_buf->buf[i] & 0x0fff));
    tmpdif2 = (int32_t)(((adc_buf->buf[i] >> 16) & 0x0fff));
    difAdcValBuf[i] = tmpdif1 - tmpdif2;
  }
  // 计算导数的斜率，统计difAdcValBuf中的正负值，并以正负值的个数差值作为斜率
  // 结论这种思路不行
  // allSum = 0;
  // for (i = 0; i < N; i++) {
  //   if (difAdcValBuf[i] > 0) {
  //     allSum += 1;
  //   } else if (difAdcValBuf[i] < 0) {
  //     allSum -= 1;
  //   }
  // }
  // allSumF = ((float_t)allSum) / N;

  //筛选导数值，只保留绝对值小于100的值
  allSum = 0;
  for (i = 0; i < N; i++) {
    if (abs(difAdcValBuf[i]) < 50) {
      allSum += difAdcValBuf[i];
    }
  }
  allSumF = ((float_t)allSum) / N;

  // 根据斜率调整dac输出
#define kP_2 (+5.0)
  dletDac = ((float_t)allSumF) * kP_2;

  tmpUint32 = abs((int32_t)(dletDac));

  deltNoisV = 0;

  (*dacP) += (int32_t)(dletDac);
  (*dacN) -= (int32_t)(dletDac);
  (*dacP) += deltNoisV;
  (*dacN) -= deltNoisV;
  max5307_w_chanel(DACP_1, (((*dacP) >> 20) & 0xffc), max_outdisable);
  max5307_w_chanel(DACP_1K, (((*dacP) >> 10) & 0xffc), max_outdisable);
  max5307_w_chanel(DACP_1M, ((*dacP) & 0xfff) & 0xfff, max_outdisable);
  max5307_w_chanel(DACN_1, (((*dacN) >> 20) & 0xffc), max_outdisable);
  max5307_w_chanel(DACN_1K, (((*dacN) >> 10) & 0xffc), max_outdisable);
  max5307_w_chanel(DACN_1M, ((*dacN) & 0xfff) & 0xfff, max_outdisable);

  do {
    uint16_t chanelNo = 0;
    chanelNo = 0x01 << (DACP_1 + 2) | 0x01 << (DACP_1K + 2) |
               0x01 << (DACP_1M + 2) | 0x01 << (DACN_1 + 2) |
               0x01 << (DACN_1K + 2) | 0x01 << (DACN_1M + 2);
    max5307_enable_ori_chanel(chanelNo);
  } while (0);

  // clear buf
  for (i = 0; i < ADC_BUF_T_SIZE; i++) {
    adc_buf->buf[i] = 0;
  }
  logPara.mean_adc_val = forSaveAllSum;
  // logPara.mean_adc_val = pidI;
  logPara.dac_val_n = (*dacN);
  logPara.dac_val_p = (*dacP);
  // HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
}

//
void cal_cp_output_dac_ext_3(struct adc_buf_t *adc_buf, uint32_t *dacP,
                             uint32_t *dacN, int32_t runTime500us){

}


uint32_t adc1_ok = 0, adc2_ok = 0;
#define MID_CP_VALUE (2048)
// uint32_t value_dacP = 0x8020200, value_dacN = 0x8020200;
uint32_t value_dacP = 0x80200800, value_dacN = 0x80200800;
int32_t mainCp = 0, mainCpLast = 0;
struct adc_buf_t adc1_t1, adc1_t2;

void fakeDelay(int32_t delay) {
  int32_t i = 0, k = 0;
  int32_t tmp = 0;
  for (k = 0; k < delay; k++) {
    for (i = 0; i < 100; i++) {
      tmp++;
    }
  }
}

extern SPI_HandleTypeDef hspi1;
void preMainLoop1(void) {
  do {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
      Error_Handler();
    }
  } while (0);
  init_ad9520();
}

struct log_para_t logBufDma;
uint16_t ad9520_reg = 0;
uint32_t locked = 0;
uint32_t adcVal, lastAdcVal;
void mainLoop1(void) {
#define CONST_DELTDA (0X000)
  uint32_t dac_default_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int32_t runTimes =
      0;  // log 30s, for example 0.5ms*2000=1s, runtimes=2000*60=1min
  int32_t xyz = 0;
  adc1_t1.adc_cap_ok = CALOK;
  adc1_t2.adc_cap_ok = CALOK;
  /* USER CODE BEGIN 2 */
  // SystemClock_Config();//from main.c
  // MX_USART2_UART_Init();
  init_max5307();
  set_dac_initVal(dac_default_val);
  max5307_w_chanel(max_ch8, 2048, max_outenable);
  do {  // set 1.5v
    value_dacP -= CONST_DELTDA;
    value_dacN += CONST_DELTDA;
    max5307_w_chanel(DACP_1, (value_dacP >> 12), max_outenable);
    max5307_w_chanel(DACP_1K, (value_dacP & 0XFFF), max_outenable);
    max5307_w_chanel(DACN_1, (value_dacN >> 12), max_outenable);
    max5307_w_chanel(DACN_1K, (value_dacN & 0xFFF), max_outenable);
  } while (0);

  init_ad9520();
  // SystemClock_Config();
  // MX_USART2_UART_Init();
  // fprintf(logBufDma,"helloworld");
  //  for(int i=0;i<10;i++){
  //  	logBufDma[i] = 65 + i;
  //  }
  logBufDma.sbs = 35;  //'#'

  HAL_UART_IRQHandler(&huart2);  // must add this, ornot uart2 dma not work
  HAL_UART_Transmit_DMA(&huart2, (uint8_t *)&logBufDma,
                        sizeof(struct log_para_t));

  HAL_Delay(1);
  adc1_ok = 0;
  adc2_ok = 0;
  //在下面的代码中使用while(1)循环测试adc采样，dac输出，这是一段临时测试代码
    //HAL_ADC_Start(&hadc2);
  //   uint32_t adcVal = 0;
  //   int32_t adcCalVal = 0;
  //   int32_t nowVal=0,lastVal=0;
  //   double adcValF = 0;
  //   uint32_t dacP=value_dacP, dacN=value_dacN;
  //   HAL_ADC_Start_DMA(&hadc2, (uint32_t *)(&adcVal), 1);
  //   while(1){
      
  //     if(adc1_t1.adc_cap_ok == ADCCAPOK){
  //       adc1_t1.adc_cap_ok = ADCCAPWAIT;
  //       lastVal = nowVal;
  //       nowVal = (adcVal&0xfff);
  //       adcCalVal = (int32_t)nowVal - (int32_t)lastVal;
  //       HAL_ADC_Start_DMA(&hadc2, (uint32_t *)(&adcVal), 1);
  //     #define kP_3 (-5000.0)
  //     #define DELV (10)
  //       if(adcCalVal < 20){
  //         dletDac = ((float_t)adcCalVal) * kP_2;
  //         if(dletDac>0){
  //           (dacP) += DELV;
  //           (dacN) -= DELV;
  //         }else{
  //           (dacP) -= DELV;
  //           (dacN) += DELV;
  //         }
  //       }
  //       //int32_t tmpUint32 = abs((int32_t)(dletDac));
  //       // (dacP) += (int32_t)(dletDac);
  //       // (dacN) -= (int32_t)(dletDac);
        
  //     // (dacP) += deltNoisV;
  //     // (dacN) -= deltNoisV;
  //       max5307_w_chanel(DACP_1, (((dacP) >> 10) & 0xffc), max_outdisable);
  //       max5307_w_chanel(DACP_1K, (((dacP) >> 10) & 0xffc), max_outdisable);
  //       max5307_w_chanel(DACP_1M, ((dacP) & 0xfff) & 0xfff, max_outdisable);
  //       max5307_w_chanel(DACN_1, (((dacN) >> 10) & 0xffc), max_outdisable);
  //       max5307_w_chanel(DACN_1K, (((dacN) >> 10) & 0xffc), max_outdisable);
  //       max5307_w_chanel(DACN_1M, ((dacN) & 0xfff) & 0xfff, max_outdisable);

  //       do {
  //         uint16_t chanelNo = 0;
  //         chanelNo = 0x01 << (DACP_1 + 2) | 0x01 << (DACP_1K + 2) |
  //                    0x01 << (DACP_1M + 2) | 0x01 << (DACN_1 + 2) |
  //                    0x01 << (DACN_1K + 2) | 0x01 << (DACN_1M + 2);
  //         max5307_enable_ori_chanel(chanelNo);
  //       } while (0);
  //       // max5307_w_chanel(DACP_1,  (((adcCalVal) >> 0)&0xfff), max_outenable);
  //       HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  //     }
  //  }

  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc1_t1.buf, ADC_BUF_T_SIZE * 2);
  int32_t deltaDAC = 0;
  while (0) {
    value_dacP += deltaDAC;
    value_dacN -= deltaDAC;
    deltaDAC += 1;

    // max5307_w_chanel(DACP_1, (value_dacP  >> 12)&0xffc, max_outdisable);
    // max5307_w_chanel(DACP_1K, ((value_dacP>>0) & 0xfff), max_outdisable);
    // max5307_w_chanel(DACN_1, (value_dacN  >> 12)&0xffc, max_outdisable);
    // max5307_w_chanel(DACN_1K, ((value_dacN>>0) & 0xfff), max_outdisable);

    max5307_w_chanel(DACP_1, (((value_dacP) >> 20) & 0xffc), max_outdisable);
    max5307_w_chanel(DACP_1K, (((value_dacP) >> 10) & 0xffc), max_outdisable);
    max5307_w_chanel(DACP_1M, ((value_dacP) & 0xfff) & 0xfff, max_outdisable);
    max5307_w_chanel(DACN_1, (((value_dacN) >> 20) & 0xffc), max_outdisable);
    max5307_w_chanel(DACN_1K, (((value_dacN) >> 10) & 0xffc), max_outdisable);
    max5307_w_chanel(DACN_1M, ((value_dacN) & 0xfff) & 0xfff, max_outdisable);

    do {
      uint16_t chanelNo = 0;
      chanelNo = 0x01 << (DACP_1 + 2) | 0x01 << (DACP_1K + 2) |
                 0x01 << (DACP_1M + 2) | 0x01 << (DACN_1 + 2) |
                 0x01 << (DACN_1K + 2) | 0x01 << (DACN_1M + 2);
      max5307_enable_ori_chanel(chanelNo);
    } while (0);
    // HAL_Delay(1);
  }

  /* USER CODE END 2 */

  while (1) {
    if (adc1_t1.adc_cap_ok == ADCCAPOK) {
      // cal t1
      // HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc1_t2.buf ,ADC_BUF_T_SIZE*2);
      // cal_cp_output_dac(&adc1_t1, &value_dacP, &value_dacN);
      cal_cp_output_dac_ext_1(&adc1_t1, &value_dacP, &value_dacN, runTimes);
      adc1_t1.adc_cap_ok = CALOK;
      runTimes++;  // log run time runTimes+1 = 500us,

    } else if (adc1_t2.adc_cap_ok == ADCCAPOK) {
      // cal t2
      // HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc1_t1.buf ,ADC_BUF_T_SIZE*2);
      // cal_cp_output_dac(&adc1_t2, &value_dacP, &value_dacN);
      cal_cp_output_dac_ext_1(&adc1_t2, &value_dacP, &value_dacN, runTimes);
      adc1_t2.adc_cap_ok = CALOK;
      runTimes++;  // log run time runTimes+1 = 500us,
    }
    // printf("hello stm32 detect vel\n");
    // HAL_Delay(100);
  }
}

//用于快速adc采样，采用dma方式，每次只采样一个数值，dma中断代码中设置标志，通知主程序采样完成
//主程序接收到标志后，可以读取adc采样值，并开启下一次adc dma采样
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

//   if (hadc == &hadc1) {
//     adc1_ok = 1;
//     HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_t1.buf, ADC_BUF_T_SIZE * 2);
//   }
//   if (hadc == &hadc2) {
//     adc1_t1.adc_cap_ok = ADCCAPOK;
    
//   }
// }


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  // 这里是当ADC转换完成时执行的代码
  // 可以设置标志，通知主程序采样已经完成
  if (hadc == &hadc1) {
    adc2_ok = 1;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)value_adc2, ADC_TIMES_N);
  }
  if (hadc == &hadc2) {
    if (adc1_t1.adc_cap_ok == CALOK) {
      HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc1_t1.buf, ADC_BUF_T_SIZE * 2);
      adc1_t1.adc_cap_ok = ADCCAPOK;
    } else if (adc1_t2.adc_cap_ok == CALOK) {
      HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc1_t2.buf, ADC_BUF_T_SIZE * 2);
      adc1_t2.adc_cap_ok = ADCCAPOK;
    } else {
    }
    HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
  }
}

// void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart){
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart2) {
    // uint16_t *padcv = (uint16_t *)(&logBufDma[1]);
    // uint32_t *pdacp = (uint32_t *)(&logBufDma[3]);
    // uint32_t *pdacn = (uint32_t *)(&logBufDma[7]);
    logBufDma.mean_adc_val = logPara.mean_adc_val;
    logBufDma.dac_val_p = logPara.dac_val_p;
    logBufDma.dac_val_n = logPara.dac_val_n;

    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)&logBufDma,
                          sizeof(struct log_para_t));
  }
}