#include "Board_I2C.h"
void SoftI2C_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd( I2C_GPIOCLK, ENABLE); 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;          //��ͨ���ģʽ
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;         //�������
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //����
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin | I2C_SCLPin ;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
static void SoftI2C_SDASetOut(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;          //��ͨ���ģʽ
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;         //�������
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //����
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
static void SoftI2C_SDASetIn(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;          //��ͨ���ģʽ
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //����
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
void SoftI2C_StartSignal(void)
{
	SoftI2C_SDASetOut();
	SoftI2C_SDASetHigh();  
	SoftI2C_SCLSetHigh(); 
	RTE_RoundRobin_DelayUS(4);  
	SoftI2C_SDASetLow();        
	RTE_RoundRobin_DelayUS(4);    
	SoftI2C_SCLSetLow();   
}
void SoftI2C_StopSignal(void)
{
	SoftI2C_SDASetOut();
	SoftI2C_SCLSetLow();    
	SoftI2C_SDASetLow(); 	
	RTE_RoundRobin_DelayUS(4);               
	SoftI2C_SCLSetHigh();  
	SoftI2C_SDASetHigh(); 
	RTE_RoundRobin_DelayUS(4); 
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�

uint8_t SoftI2C_WaitAck(void)
{
	SoftI2C_SDASetIn();      //SDA����Ϊ����  
	SoftI2C_SDASetHigh();
	RTE_RoundRobin_DelayUS(1);	   
	SoftI2C_SCLSetHigh();
	RTE_RoundRobin_DelayUS(1);	 
	uint8_t ucErrTime=0;
	while(SoftI2C_SDARead())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			SoftI2C_StopSignal();
			return 1;
		}
	}
	SoftI2C_SCLSetLow();//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
static void SoftI2C_Ack(void)
{
	SoftI2C_SCLSetLow();
	SoftI2C_SDASetOut();
	SoftI2C_SDASetLow();
	RTE_RoundRobin_DelayUS(2);
	SoftI2C_SCLSetHigh();
	RTE_RoundRobin_DelayUS(2);
	SoftI2C_SCLSetLow();
}
//������ACKӦ��		    
static void SoftI2C_NAck(void)
{
	SoftI2C_SCLSetLow();
	SoftI2C_SDASetOut();
	SoftI2C_SDASetHigh();
	RTE_RoundRobin_DelayUS(2);
	SoftI2C_SCLSetHigh();
	RTE_RoundRobin_DelayUS(2);
	SoftI2C_SCLSetLow();
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void SoftI2C_SendByte(uint8_t txd)
{                        
	uint8_t t;   
	SoftI2C_SDASetOut(); 	    
	SoftI2C_SCLSetLow();//����ʱ�ӿ�ʼ���ݴ���
	for(t=0;t<8;t++)
	{              
		if(((txd&0x80)>>7))
			SoftI2C_SDASetHigh();
		else
			SoftI2C_SDASetLow();
		txd<<=1; 	  
		RTE_RoundRobin_DelayUS(2);   //��TEA5767��������ʱ���Ǳ����
		SoftI2C_SCLSetHigh();
		RTE_RoundRobin_DelayUS(2); 
		SoftI2C_SCLSetLow();	
		RTE_RoundRobin_DelayUS(2);
	}	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t SoftI2C_ReadByte(uint8_t ack)
{
	uint8_t i,receive=0;
	SoftI2C_SDASetIn();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
		SoftI2C_SCLSetLow(); 
		RTE_RoundRobin_DelayUS(2);
		SoftI2C_SCLSetHigh();
		receive<<=1;
		if(SoftI2C_SDARead())receive++;   
		RTE_RoundRobin_DelayUS(1); 
	}					 
	if (!ack)
		SoftI2C_NAck();//����nACK
	else
		SoftI2C_Ack(); //����ACK   
	return receive;
}
