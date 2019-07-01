#ifndef __BOARD_DHT11_H
#define __BOARD_DHT11_H
#include "stm32f4xx.h"
#include "RTE_Include.h"
#define DHT11_DataPin  GPIO_Pin_15
#define DHT11_GPIO     GPIOA
#define DHT11_GPIOCLK  RCC_AHB1Periph_GPIOA
typedef struct
{
	uint8_t TemperHigh8;
	uint8_t TemperLow8;
	uint8_t HumdityHigh8;
	uint8_t HumdityLow8;
	uint8_t CheckData;
}DHT11Result_t;
void DHT11_GetData(void);
extern DHT11Result_t DHT11Result;
#endif
