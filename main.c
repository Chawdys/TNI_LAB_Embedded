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

#define G_05	                    (uint16_t)784
#define A_05	                    (uint16_t)880
#define Bb_05	                    (uint16_t)932
#define C_06	                    (uint16_t)1046	
#define Db_06	                    (uint16_t)1108
#define D_06	                    (uint16_t)1174
#define Eb_06	                    (uint16_t)1244
#define F_06	                    (uint16_t)1396
#define Gb_06	                    (uint16_t)1480
#define G_06	                    (uint16_t)1568
#define Ab_06	                    (uint16_t)1661
#define A_06	                    (uint16_t)1760
#define Bb_06	                    (uint16_t)1864
#define B_06	                    (uint16_t)1975
#define E_06											(uint16_t)1318
#define MUTE											(uint16_t) 1

/*for 10ms update event*/
#define TIMx_PSC			50

/*Macro function for ARR calculation*/
#define ARR_CALCULATE(N) ((32000000) / ((TIMx_PSC) * (N)))

int sheetNote[] = { ARR_CALCULATE(G_06),MUTE,ARR_CALCULATE(D_06),MUTE,ARR_CALCULATE(G_05),MUTE,ARR_CALCULATE(A_05),MUTE,
										ARR_CALCULATE(G_06),MUTE,ARR_CALCULATE(D_06),MUTE,ARR_CALCULATE(G_05),MUTE,ARR_CALCULATE(A_05),MUTE,
										ARR_CALCULATE(G_06),MUTE,ARR_CALCULATE(D_06),MUTE,ARR_CALCULATE(G_05),MUTE,ARR_CALCULATE(A_05),MUTE,'\0'};

void SystemClock_Config(void);
void TIMBaseMain_Config(void);
void TIM_BASE_Config(uint16_t);
void TIM_OC_GPIO_Config(void);
void TIM_OC_Config(uint16_t);
void TIM_BASE_DurationConfig(void);
void GPIO_Config(void);
void ltc4727_GPIO_Config(void);
uint32_t CheckDigit(uint32_t);
void segment(uint32_t);
uint32_t CharToUint32_t(char number);
void play_music(void);
void LED_TOGGLE(void);

uint32_t seg[3] = {	 LL_GPIO_PIN_10 | LL_GPIO_PIN_11,
										 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14,
										 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 };
uint32_t digit[3] = {LL_GPIO_PIN_0 , LL_GPIO_PIN_1 , LL_GPIO_PIN_2};
uint32_t cnt = 0;
uint8_t chk = 0;		
int i;
uint8_t ROUND=3;
uint32_t count_time = 180;
uint32_t each_round = 60;

int main()
{
	SystemClock_Config();
	TIMBaseMain_Config();
	TIM_OC_Config(ARR_CALCULATE(E_06));
	TIM_BASE_DurationConfig();
	GPIO_Config();
	ltc4727_GPIO_Config();
	segment((uint32_t)count_time);
	count_time+=each_round;
	while(1)
	{
		cnt = LL_TIM_GetCounter(TIM3);
		cnt = (cnt+1)/1000;
		segment((uint32_t)count_time-(each_round-cnt));
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

void TIMBaseMain_Config(void)
{
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_DOWN;
	//timbase_initstructure.Autoreload = 5000-1; //for test 5 second
	timbase_initstructure.Autoreload = 60000-1; //for 1 minute
	timbase_initstructure.Prescaler = 32000-1;
	
	LL_TIM_Init(TIM3, &timbase_initstructure);	
	
	LL_TIM_EnableIT_UPDATE(TIM3);
	
	NVIC_SetPriority(TIM3_IRQn,0);
	NVIC_EnableIRQ(TIM3_IRQn);
	
	//LL_TIM_EnableCounter(TIM3);
}

void TIM3_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM3) == SET)
	{
		LL_TIM_ClearFlag_UPDATE(TIM3);
		if(chk==ROUND){
			segment((uint32_t)0);
			play_music();
			LL_TIM_DisableCounter(TIM3);
		}
		chk += 1;
		count_time -= each_round;
	}
}

void TIM_BASE_DurationConfig(void)
{
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	//Time-base configure
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP;
	timbase_initstructure.Autoreload = 200 - 1;
	timbase_initstructure.Prescaler =  32000 - 1;
	LL_TIM_Init(TIM2, &timbase_initstructure);
	
	//LL_TIM_EnableCounter(TIM2); 
	//LL_TIM_ClearFlag_UPDATE(TIM2); //Force clear update flag
}

void TIM_BASE_Config(uint16_t ARR)
{
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	//Time-base configure
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP;
	timbase_initstructure.Autoreload = ARR - 1;
	timbase_initstructure.Prescaler =  TIMx_PSC- 1;
	LL_TIM_Init(TIM4, &timbase_initstructure);
	
	//LL_TIM_EnableCounter(TIM4); 
}


void TIM_OC_GPIO_Config(void)
{
	LL_GPIO_InitTypeDef gpio_initstructure;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	gpio_initstructure.Mode = LL_GPIO_MODE_ALTERNATE;
	gpio_initstructure.Alternate = LL_GPIO_AF_2;
	gpio_initstructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstructure.Pin = LL_GPIO_PIN_6;
	gpio_initstructure.Pull = LL_GPIO_PULL_NO;
	gpio_initstructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	LL_GPIO_Init(GPIOB, &gpio_initstructure);
}

void TIM_OC_Config(uint16_t note)
{
	LL_TIM_OC_InitTypeDef tim_oc_initstructure;
	
	TIM_OC_GPIO_Config();
	TIM_BASE_Config(note);
	
	tim_oc_initstructure.OCState = LL_TIM_OCSTATE_DISABLE;
	tim_oc_initstructure.OCMode = LL_TIM_OCMODE_PWM1;
	tim_oc_initstructure.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	tim_oc_initstructure.CompareValue = LL_TIM_GetAutoReload(TIM4) / 2;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &tim_oc_initstructure);
	/*Interrupt Configure*/
	NVIC_SetPriority(TIM4_IRQn, 1);
	NVIC_EnableIRQ(TIM4_IRQn);
	//LL_TIM_EnableIT_CC1(TIM4);
	
	/*Start Output Compare in PWM Mode*/
	LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
	//LL_TIM_EnableCounter(TIM4);
}

void GPIO_Config(void){
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	
	LL_GPIO_InitTypeDef gpio;
	gpio.Mode = LL_GPIO_MODE_INPUT;
	gpio.Pin = LL_GPIO_PIN_0;
	gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio.Pull = LL_GPIO_PULL_NO;
	gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	LL_GPIO_Init(GPIOA, &gpio);
	
	gpio.Mode = LL_GPIO_MODE_OUTPUT;
	gpio.Pin = LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOB, &gpio);
	
	gpio.Pin = LL_GPIO_PIN_5;
	LL_GPIO_Init(GPIOB, &gpio);
	
	gpio.Pin = LL_GPIO_PIN_4;
	LL_GPIO_Init(GPIOB, &gpio);
	
	RCC->APB2ENR |= (1<<0);
	SYSCFG->EXTICR[0] &= ~(15<<0);
	EXTI->IMR |= (1<<0);
	EXTI->RTSR |= (1<<0);
	NVIC_EnableIRQ((IRQn_Type) 6);
}

void EXTI0_IRQHandler(void){
	if((EXTI->PR & (1<<0)) == 1){
		EXTI->PR |= (1<<0);
		LL_TIM_EnableCounter(TIM3);
		//chk+=1;
	}
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

void play_music(void){
	LL_TIM_EnableCounter(TIM4); 
	LL_TIM_EnableCounter(TIM2); 
	int i=0;
	while(sheetNote[i] != '\0')
	{
		if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == SET)
		{
			LL_TIM_ClearFlag_UPDATE(TIM2);
			LED_TOGGLE();
			LL_TIM_SetAutoReload(TIM4, sheetNote[i]); //Change ARR of Timer PWM
			i++;
			LL_TIM_SetCounter(TIM2, 0);
		}
	}
}



void LED_TOGGLE(void){
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_7);
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_5);
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_4);
}
