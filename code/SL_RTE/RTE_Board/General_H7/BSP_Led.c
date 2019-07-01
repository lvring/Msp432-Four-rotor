#include "BSP_Led.h"
//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
static BSP_LED_Handle_t LedControlArray[LED_N]=
{
  // Name    ,PORT , PIN ,                        Init
	{LED_B ,GPIOC,GPIO_PIN_0 ,LED_OFF},   // PD2,LED0
};

//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void BSP_LED_Off(BSP_LED_Name_t led_name)
{
#ifdef USE_STDPERIPH_DRIVER
	GPIO_SetBits(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
#endif
#ifdef USE_HAL_DRIVER
	HAL_GPIO_WritePin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, GPIO_PIN_RESET);
#endif
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void BSP_LED_On(BSP_LED_Name_t led_name)
{
#ifdef USE_STDPERIPH_DRIVER
	GPIO_ResetBits(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
#endif
#ifdef USE_HAL_DRIVER
	HAL_GPIO_WritePin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, GPIO_PIN_SET);
#endif
} 
//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void BSP_LED_Toggle(BSP_LED_Name_t led_name)
{
  LedControlArray[led_name].LedPort->ODR ^= LedControlArray[led_name].LedPin;
}
void BSP_LED_Init(void)
{
  for(BSP_LED_Name_t led_name=(BSP_LED_Name_t)0;led_name<LED_N;led_name++) 
	{
#ifdef USE_STDPERIPH_DRIVER
	  GPIO_InitTypeDef  GPIO_InitStructure;
		#ifdef STM32F1
    // Clock Enable
    RCC_APB2PeriphClockCmd(LedControlArray[led_name].LedClk, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = LedControlArray[led_name].LedPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LedControlArray[led_name].LedPort, &GPIO_InitStructure);
		#endif
		#ifdef STM32F4
		// Clock Enable
    RCC_AHB1PeriphClockCmd(LedControlArray[led_name].LedClk, ENABLE);

    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = LedControlArray[led_name].LedPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LedControlArray[led_name].LedPort, &GPIO_InitStructure);
		#endif
#endif

#if USE_HAL_DRIVER
		GPIO_InitTypeDef  GPIO_InitStruct;
		__HAL_RCC_GPIOC_CLK_ENABLE();
		GPIO_InitStruct.Pin = LedControlArray[led_name].LedPin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(LedControlArray[led_name].LedPort, &GPIO_InitStruct);
#endif
    // Default Wert einstellen
    if(LedControlArray[led_name].LedInitStatus==LED_OFF) {
      BSP_LED_Off(led_name);
    }
    else {
      BSP_LED_On(led_name);
    }
	}
}


