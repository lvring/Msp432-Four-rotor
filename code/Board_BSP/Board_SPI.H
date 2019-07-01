#ifndef __BOARD_SPI_H
#define __BOARD_SPI_H

#include <driverlib.h>
#include "RTE_Include.h"

#define MPU6500_CS(X)			(X==0)?GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0):GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0) //MPU6500片选信号


extern void SPI1_Init(void);
extern void MPU6500_Write_Reg(uint8_t reg,uint8_t value);
extern uint8_t MPU6500_Read_Reg(uint8_t reg);
extern uint8_t SPI1_Read_Write_Byte(uint8_t TxData);//SPI总线读写一个字节
void SPI1_Write_NByte(uint8_t *pData, uint16_t Size);
#endif
