#ifndef __BOARD_i2c_H
#define __BOARD_i2c_H
/* DriverLib Includes */
#include <driverlib.h>
/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <string.h>


//IO��������
#define SDA_IN()  {GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6,GPIO_PIN4);}	//PB4����ģʽ
#define SDA_OUT() {GPIO_setAsOutputPin(GPIO_PORT_P6,GPIO_PIN4);} //PB4���ģʽ
//IO��������	 
#define IIC_SCL(X)    (X==0)?GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN5):GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN5) //MPU6500Ƭѡ�ź�
#define IIC_SDA(X)   	(X==0)?GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN4):GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN4) //MPU6500Ƭѡ�ź�
#define READ_SDA   		GPIO_getInputPinValue(GPIO_PORT_P6,GPIO_PIN4)


//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
int IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data);
uint8_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data);
uint8_t IICwriteBits(uint8_t dev,uint8_t reg,uint8_t bitStart,uint8_t length,uint8_t data);
uint8_t IICwriteBit(uint8_t dev,uint8_t reg,uint8_t bitNum,uint8_t data);
uint8_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data);

int i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
int i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);



#endif

