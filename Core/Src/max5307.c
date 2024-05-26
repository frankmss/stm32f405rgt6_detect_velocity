#include "max5307.h"

extern SPI_HandleTypeDef hspi2;

static uint16_t outputList; //recode output reg

void init_max5307(void){
	uint16_t dat;
	HAL_GPIO_WritePin(DAC_LOAD_GPIO_Port, DAC_LOAD_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(2);
	dat = 0x01;
	dat = dat<<12;
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
	dat = 0x0f;
	dat = dat<<12 | 0xfff;
	HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
	outputList=0x0;
}
//all output enable
void max5307_loadpin(void){
	HAL_GPIO_WritePin(DAC_LOAD_GPIO_Port, DAC_LOAD_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DAC_LOAD_GPIO_Port, DAC_LOAD_Pin, GPIO_PIN_SET);
}

void max5307_w_chanel(enum max5306_chanel chanel, uint32_t val, enum max5306_ouput enable ){
	uint16_t dat;

	dat = chanel;
	dat = dat<<12;
	dat = dat | val;
	//HAL_Delay(2);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
    
  HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
  if(enable == max_outenable){
		dat = 0xe000 | 0x01<<(chanel+2) | outputList;
		outputList = outputList | 0x01<<(chanel+2);
		HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	}
  //HAL_Delay(3);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
  //HAL_Delay(2);
}

void max5307_enable_chanel(enum max5306_chanel chanel){
	uint16_t dat;
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
  
	dat = 0xe000 | 0x01<<(chanel+2) | outputList;
	outputList = outputList | 0x01<<(chanel+2);
	HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	
  //HAL_Delay(3);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}

void max5307_enable_ori_chanel(uint16_t oriChanelNo){
	uint16_t dat;
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
  
	dat = 0xe000 | oriChanelNo | outputList;
	outputList = outputList | oriChanelNo;
	HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	
  //HAL_Delay(3);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);	
}

void max5307_disable_chanel(enum max5306_chanel chanel){
	uint16_t dat;
	HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
  dat = 0xe000 & (0xfffffffe<<(chanel+2)) | outputList;
	outputList = outputList & (0xfffffffe<<(chanel+2));
	
	HAL_SPI_Transmit(&hspi2, (uint8_t *)&dat, 1, 10);
	
  //HAL_Delay(3);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}
