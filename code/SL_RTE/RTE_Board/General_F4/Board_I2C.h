#ifndef __BOARD_I2C_H
#define __BOARD_I2C_H
#include "RTE_Include.h"
#include "stm32f4xx.h"
#define I2C_SCLPin  GPIO_Pin_8
#define I2C_SDAPin  GPIO_Pin_9
#define I2C_GPIO    GPIOB
#define I2C_GPIOCLK RCC_AHB1Periph_GPIOB
#define SoftI2C_SDARead() GPIO_ReadInputDataBit(I2C_GPIO,I2C_SDAPin)
#define SoftI2C_SDASetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SDAPin)
#define SoftI2C_SDASetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SDAPin)
#define SoftI2C_SCLSetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SCLPin)
#define SoftI2C_SCLSetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SCLPin)
void SoftI2C_Init(void);
void SoftI2C_StartSignal(void);
void SoftI2C_StopSignal(void);
uint8_t SoftI2C_WaitAck(void);
uint8_t SoftI2C_ReadByte(uint8_t ack);
void SoftI2C_SendByte(uint8_t txd);

#endif
