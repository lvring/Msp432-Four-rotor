#ifndef __BSP_ADC_H
#define __BSP_ADC_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "RTE_Include.h"
//--------------------------------------------------------------
// Liste aller ADC-Kanäle
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum {
  ADC_PA0 = 0,  // PA3
	ADC1d_ANZ,
}ADC1d_NAME_t;
//--------------------------------------------------------------
// Adressen der ADCs
// (siehe Seite 54+303 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_BASE_ADR        ((uint32_t)0x40012000)
#define ADC_ADC1_OFFSET     ((uint32_t)0x00000000)
//--------------------------------------------------------------
// Adressen der Register
// (siehe Seite 304+305 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_REG_DR_OFFSET         0x4C
#define ADC1_DR_ADDRESS    (ADC_BASE_ADR | ADC_ADC1_OFFSET | ADC_REG_DR_OFFSET)
//--------------------------------------------------------------
// ADC-Clock
// Max-ADC-Frq = 36MHz
// Grundfrequenz = APB2 (APB2=84MHz)
// Mögliche Vorteiler = 2,4,6,8
//--------------------------------------------------------------
//#define ADC1d_VORTEILER     ADC_Prescaler_Div2 // Frq = 42 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div4   // Frq = 21 MHz
#define ADC1d_VORTEILER     ADC_Prescaler_Div6 // Frq = 14 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div8 // Frq = 10.5 MHz
//--------------------------------------------------------------
// DMA Einstellung
// Moegliche DMAs fuer ADC1 :
//   DMA2_STREAM0_CHANNEL0
//   DMA2_STREAM4_CHANNEL0
//--------------------------------------------------------------
#define ADC1_DMA_STREAM0            0
//#define ADC1_DMA_STREAM4          4
#ifdef ADC1_DMA_STREAM0
 #define ADC1_DMA_STREAM            DMA2_Stream0
 #define ADC1_DMA_CHANNEL           DMA_Channel_0
#elif defined ADC1_DMA_STREAM4
 #define ADC1_DMA_STREAM            DMA2_Stream4
 #define ADC1_DMA_CHANNEL           DMA_Channel_0
#endif
//--------------------------------------------------------------
// Struktur eines ADC Kanals
//--------------------------------------------------------------
typedef struct {
  ADC1d_NAME_t ADC_NAME;  // Name
  GPIO_TypeDef* ADC_PORT; // Port
  const uint16_t ADC_PIN; // Pin
  const uint32_t ADC_CLK; // Clock
  const uint8_t ADC_CH;   // ADC-Kanal
}ADC1d_t;
//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_ADC1_DMA_Init(void);
uint16_t UB_ADC1_DMA_Read(ADC1d_NAME_t adc_name);
#endif
