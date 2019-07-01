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
	{LED_A ,GPIO_PORT_P5,GPIO_PIN0 ,LED_OFF},  
	{LED_B ,GPIO_PORT_P5,GPIO_PIN1 ,LED_OFF},   // PD2,LED0
	{LED_C ,GPIO_PORT_P5,GPIO_PIN2 ,LED_OFF},   // PD2,LED0
};


//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void BSP_LED_Off(BSP_LED_Name_t led_name)
{
	GPIO_setOutputLowOnPin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void BSP_LED_On(BSP_LED_Name_t led_name)
{
		GPIO_setOutputHighOnPin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
} 
//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void BSP_LED_Toggle(BSP_LED_Name_t led_name)
{
		GPIO_toggleOutputOnPin(LedControlArray[led_name].LedPort,LedControlArray[led_name].LedPin);
}
void BSP_LED_Init(void)
{
  for(BSP_LED_Name_t led_name=(BSP_LED_Name_t)0;led_name<LED_N;led_name++) 
	{
		GPIO_setAsOutputPin(
        LedControlArray[led_name].LedPort,
        LedControlArray[led_name].LedPin
        );
    // Default Wert einstellen
    if(LedControlArray[led_name].LedInitStatus==LED_OFF) {
      BSP_LED_Off(led_name);
    }
    else {
      BSP_LED_On(led_name);
    }
	}
}


