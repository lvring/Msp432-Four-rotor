#ifndef __BOARD_USART__
#define __BOARD_USART__
#include <driverlib.h>
#include "RTE_Include.h"
//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
  COM_2 = 1,   // COM2 (TX=PA2, RX=PA3)
  COM_N        // COMNum
}Board_Usart_NAME_e;
typedef enum
{
	OK        = 0x00,
	TIME_OUT,
}Board_Usart_SendStatus_e;
//--------------------------------------------------------------
// 串口数据接收结构体
//--------------------------------------------------------------
typedef struct
{
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t   ComQuene;  //串口数据环形队列
}Board_Usart_Data_t;
//--------------------------------------------------------------
// Struktur einer UARTs
//--------------------------------------------------------------
typedef struct {
  Board_Usart_NAME_e ComName;    // Name
	EUSCI_A_Type * EUSCIx;
	uint32_t EUSCIx_Base;
  uint_fast8_t selectedPort; // Port
  uint16_t ComPintx; // Pin
  uint16_t ComPinrx; // Pin
  uint32_t ClockSource; // Clock
	uint16_t BRDIV;
	uint16_t UCxBRF;
	uint16_t UCxBRS;
	uint16_t WordLength; 
  uint16_t StopBits;
  uint16_t Parity; 
  uint16_t Mode; 
  uint16_t HardwareFlowControl; 
	Board_Usart_Data_t UsartData;
}Board_Usart_Handle_t;
void Board_Usart_Init(Board_Usart_NAME_e usart_name);
void Board_Usart_Puts(Board_Usart_NAME_e usart_name, char* str,uint16_t len);
void Board_Usart_Putc(Board_Usart_NAME_e usart_name, uint8_t Data);
Board_Usart_Data_t *Board_Usart_ReturnQue(Board_Usart_NAME_e usart_name);
#endif
