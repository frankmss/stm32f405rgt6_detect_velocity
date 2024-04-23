#include "main.h"
 
#define IDLE   0
#define DONE   1
#define F_CLK  72000000UL
 
volatile uint8_t gu8_State = IDLE;
volatile uint8_t gu8_MSG[35] = {'\0'};
volatile uint32_t gu32_T1 = 0;
volatile uint32_t gu32_T2 = 0;
volatile uint32_t gu32_Ticks = 0;
volatile uint16_t gu16_TIM2_OVC = 0;
volatile uint32_t gu32_Freq = 0;
 
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
 
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
 
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART1_UART_Init();
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
 
    while (1)
    {
 
    }
}
 
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
    if(gu8_State == IDLE)
    {
        gu32_T1 = TIM2->CCR1;
        gu16_TIM2_OVC = 0;
        gu8_State = DONE;
    }
    else if(gu8_State == DONE)
    {
        gu32_T2 = TIM2->CCR1;
        gu32_Ticks = (gu32_T2 + (gu16_TIM2_OVC * 65536)) - gu32_T1;
        gu32_Freq = (uint32_t)(F_CLK/gu32_Ticks);
        if(gu32_Freq != 0)
        {
          sprintf(gu8_MSG, "Frequency = %lu Hz\n\r", gu32_Freq);
          HAL_UART_Transmit(&huart1, gu8_MSG, sizeof(gu8_MSG), 100);
        }
        gu8_State = IDLE;
    }
}
 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    gu16_TIM2_OVC++;
}