/*
//SPI文件 完成配合6500的SPI 通过SPI读取6500数据的服务函数
//SJ 2018.1.18
*/ 
#include "Board_SPI.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
/*
 * 函数名：SPI1_Init
 * 描述  ：SPI1初始化
 * 输入  ：无
 * 输出  ：无
 */ 
 /* Statics */

 //![Simple SPI Config]
/* SPI Master Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,             // SMCLK Clock Source
        12000000,                                   // SMCLK = DCO = 12MHZ
        1000000,                                    // SPICLK = 500khz
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, // HIGH polarity
        EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
};
//![Simple SPI Config]
void SPI1_Init(void)
{
	 GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	
	 GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
										 
									
	 MPU6500_CS(1); 			 																											 
	 									
	
	/* Configuring SPI in 3wire master mode */
  SPI_initMaster(EUSCI_B3_BASE, &spiMasterConfig);			
 /* Enable SPI module */
  SPI_enableModule(EUSCI_B3_BASE);	
	//SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
  //Interrupt_enableInterrupt(INT_EUSCIB0);	

}
/*
 * 函数名：MPU6500_Write_Reg
 * 描述  ：利用SPI写入寄存器 为向6500发送指令封装接口
 * 输入  ：reg:指定的寄存器地址；value：写入的值
 * 输出  ：status：返回状态值
 */ 
void MPU6500_Write_Reg(uint8_t reg,uint8_t value)
{
	MPU6500_CS(0);  										//使能6500片选
	SPI1_Read_Write_Byte(reg);           //发送写命令+寄存器号
	SPI1_Read_Write_Byte(value);				  //写入寄存器值
	MPU6500_CS(1);  										//禁止MPU9500
}
/*
 * 函数名：MPU6500_Read_Reg
 * 描述  ：利用SPI读取寄存器 为6500读取数据封装接口
 * 输入  ：reg:指定的寄存器地址
 * 输出  ：reg_val：reg寄存器地址对应的值
 */ 
uint8_t MPU6500_Read_Reg(uint8_t reg)
{
	uint8_t reg_val=0;
	reg = reg | 0x80;
	MPU6500_CS(0);  										//使能6500片选
	SPI1_Read_Write_Byte(reg); 	      //发送读命令+寄存器号
	reg_val=SPI1_Read_Write_Byte(0xff); //读取寄存器值
	MPU6500_CS(1);  									  //禁止MPU9250
	return(reg_val);
}
/*
 * 函数名：SPI1_Read_Write_Byte
 * 描述  ：SPI基础读写函数 读写一个字节
 * 输入  ：TxData:要写入的字节
 * 输出  ：读取到的字节
 */ 
uint8_t SPI1_Read_Write_Byte(uint8_t TxData)
{				 	
	uint32_t StartTick=RTE_RoundRobin_GetTick();
	while (!(EUSCI_B3->IFG & EUSCI_B_IFG_TXIFG))
	{
		if(RTE_RoundRobin_GetTick()-StartTick>=500)
			return 0;
	}//检查指定的SPI标志位设置与否:发送缓存空标志位
	SPI_transmitData(EUSCI_B3_BASE, TxData); 																//通过外设SPIx发送一个数据
	StartTick=RTE_RoundRobin_GetTick();
	while (!(SPI_getInterruptStatus(EUSCI_B3_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT)))
	{
		if(RTE_RoundRobin_GetTick()-StartTick>=500)
			return 0;
	} //检查指定的SPI标志位设置与否:接受缓存非空标志位		
	return EUSCI_B3->RXBUF;			
//		static volatile uint8_t RXData = 0;	//返回通过SPIx最近接收的数据					    
//   while (!(SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
//		SPI_transmitData(EUSCI_B0_BASE, TxData);
//	 RXData = SPI_receiveData(EUSCI_B0_BASE);
//	 
//	 return RXData;
}


