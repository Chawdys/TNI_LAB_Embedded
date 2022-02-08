#include "stm32l1xx.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_tim.h"
#include "stm32l1xx_ll_lcd.h"
#include "stm32l152_glass_lcd.h"
#include "stdio.h"

void SystemClock_Config(void);
void ltc4727_GPIO_Config(void);
uint32_t CheckDigit(uint32_t);
void segment(uint32_t);
uint32_t CharToUint32_t(char number);

uint32_t seg[3] = {	 LL_GPIO_PIN_10 | LL_GPIO_PIN_11,
										 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14,
										 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 };
uint32_t digit[3] = {LL_GPIO_PIN_0 , LL_GPIO_PIN_1 , LL_GPIO_PIN_2};
int i;
uint32_t count_time = 180;

int main()
{
	SystemClock_Config();
	ltc4727_GPIO_Config();
	segment((uint32_t)count_time);
	while(1)
	{
		for(i=0;i<3;i++){
			LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2);//Write 0 to GPIOC
			LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15);//Reser all segment
			LL_GPIO_SetOutputPin(GPIOC, digit[i]);
			LL_GPIO_SetOutputPin(GPIOB, seg[i]);
		}
	}
}

void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}

void ltc4727_GPIO_Config(void)
{
	LL_GPIO_InitTypeDef ltc4727_init;
	
	//config ltc4727js
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
		
	ltc4727_init.Mode = LL_GPIO_MODE_OUTPUT;
	ltc4727_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	ltc4727_init.Pull = LL_GPIO_PULL_NO;
	ltc4727_init.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	ltc4727_init.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
	LL_GPIO_Init(GPIOB, &ltc4727_init);
		
	ltc4727_init.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
	LL_GPIO_Init(GPIOC, &ltc4727_init);
}

uint32_t CheckDigit(uint32_t number)
{
    int i;
    int digit;
    char seg[4];
    for(i=0;number!=0;i++)
    {
        number = number/10;
    }
    return i;
}

void segment(uint32_t number)
{
    uint32_t digit;
    char numberForShow[3];
    digit = CheckDigit(number);
    switch(digit)
    {
		case 0:
        numberForShow[0] = '0';
        numberForShow[1] = '0';
        numberForShow[2] = '0';
        break;
    case 1:
        numberForShow[0] = '0';
        numberForShow[1] = '0';
        numberForShow[2] = number+'0';
        break;
    case 2:
        numberForShow[0] = '0';
        numberForShow[1] = (number/10)+'0';
        numberForShow[2] = (number%10)+'0';
        break;
    case 3:
        numberForShow[0] = (number/100)+'0';
        numberForShow[1] = (number%100/10)+'0';
        numberForShow[2] = (number%100%10)+'0';
        break;
    }
		
		for(int i=0;i<3;i++)
		{
			seg[i] = CharToUint32_t(numberForShow[i]);
		}
}

uint32_t CharToUint32_t(char number)
{
    switch(number)
    {
        case '0':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14;
        case '1':
            return LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
        case '2':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_15;
        case '3':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_15;
        case '4':
            return LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '5':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '6':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '7':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
        case '8':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '9':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    }
}