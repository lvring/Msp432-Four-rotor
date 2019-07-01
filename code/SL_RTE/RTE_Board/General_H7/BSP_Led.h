/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_LED_H
#define __BSP_LED_H
#ifdef __cplusplus  
extern "C" {  
#endif  
	/* Includes ------------------------------------------------------------------*/
	#include "stm32h7xx.h"
	#include "RTE_Include.h"
	/* Exported types ------------------------------------------------------------*/
	//--------------------------------------------------------------
	// Liste aller LEDs
	// (keine Nummer doppelt und von 0 beginnend)
	//--------------------------------------------------------------
	typedef enum 
	{
		LED_B = 0,  			 // LED0
		LED_N,
	}BSP_LED_Name_t;
	//--------------------------------------------------------------
	// Status einer LED
	//--------------------------------------------------------------
	typedef enum {
		LED_OFF = 0,  // LED AUS
		LED_ON        // LED EIN
	}BSP_LED_Status_e;
	//--------------------------------------------------------------
	// Struktur einer LED
	//--------------------------------------------------------------
	typedef struct {
		BSP_LED_Name_t LedName;    // Name
		GPIO_TypeDef* LedPort; // Port
		uint16_t LedPin; // Pin
	#ifdef USE_STDPERIPH_DRIVER
		uint32_t LedClk; // Clock
	#endif
		BSP_LED_Status_e LedInitStatus;  // Init
	}BSP_LED_Handle_t;
	void BSP_LED_Init(void);
	void BSP_LED_Toggle(BSP_LED_Name_t led_name);
	void BSP_LED_On(BSP_LED_Name_t led_name);
	void BSP_LED_Off(BSP_LED_Name_t led_name);
#ifdef __cplusplus  
}  
#endif  
#endif
