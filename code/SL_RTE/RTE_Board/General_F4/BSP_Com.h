/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_COM_H
#define __BSP_COM_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "RTE_Include.h"
//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM_1 = 0,  // COM1 (TX=PA9, RX=PA10)
  COM_3 = 1,   // COM3 (TX=PC10, RX=PC11)
	COM_6 = 2,  //COM6 ()
	COM_N ,
}BSP_COM_Name_e;
//--------------------------------------------------------------
// �������ݽ��սṹ��
//--------------------------------------------------------------
typedef struct
{
	//�ṹ�����
	uint8_t *puint8_tDatabuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t   ComQuene;  //�������ݻ��ζ���
}BSP_COM_Data_t;
//--------------------------------------------------------------
// Struktur eines UART-Pins
//--------------------------------------------------------------
typedef struct {
  GPIO_TypeDef* PORT;     // Port
  const uint16_t PIN;     // Pin
  const uint32_t CLK;     // Clock
  const uint8_t SOURCE;   // Source
}BSP_COM_PIN_t;
//--------------------------------------------------------------
// Struktur eines UARTs
//--------------------------------------------------------------
typedef struct {
  BSP_COM_Name_e UART_NAME;    // Name
  const uint32_t CLK;       // Clock
  const uint8_t AF;         // AF
  USART_TypeDef* UART;      // UART
  const uint32_t BAUD;      // Baudrate
  const uint8_t INT;        // Interrupt
  BSP_COM_PIN_t TX;            // TX-Pin
  BSP_COM_PIN_t RX;            // RX-Pin
	uint16_t DataBufferLen;
	uint16_t DataQueneLen;
	BSP_COM_Data_t ComBuffer;
}BSP_COM_Handle_t;
//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void BSP_COM_Init(void);
void BSP_COM_SendByte(BSP_COM_Name_e uart, uint16_t wert);
void BSP_COM_SendArray(BSP_COM_Name_e uart, uint8_t *data, uint16_t cnt);
BSP_COM_Data_t* BSP_COM_ReturnQue(BSP_COM_Name_e com_name);
//--------------------------------------------------------------
#endif // __STM32F4_UB_UART_H
