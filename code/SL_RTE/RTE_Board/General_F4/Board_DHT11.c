#include "Board_DHT11.h"
DHT11Result_t DHT11Result;
static void DHT11_DataPinSetOutput(void)  
{  
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(DHT11_GPIOCLK, ENABLE);     //ʹ��PC�˿�ʱ��  
	GPIO_InitStructure.GPIO_Pin = DHT11_DataPin;                 //PC.0 �˿�����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DHT11_GPIO, &GPIO_InitStructure);  
}  
static void DHT11_DataPinSetInput(void)  
{       
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(DHT11_GPIOCLK, ENABLE);     //ʹ��PC�˿�ʱ��  
	GPIO_InitStructure.GPIO_Pin = DHT11_DataPin;                 //PC.0 �˿�����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DHT11_GPIO, &GPIO_InitStructure);  
} 
static bool DHT11_GetDataBit(void)  
{  
	uint8_t val;  
	val = GPIO_ReadInputDataBit(DHT11_GPIO, DHT11_DataPin);  
	if(val == Bit_RESET){  
		return false;  
	}else{  
		return true;  
	}  
}  
static void DHT11_SetDataBit(bool level)  
{  
	if(level == true){  
		GPIO_SetBits(DHT11_GPIO, DHT11_DataPin);  
	}else{  
		GPIO_ResetBits(DHT11_GPIO, DHT11_DataPin);  
	}  
} 
static uint8_t DHT11_ReadByte(void)  
{       
	uint8_t i;    
	uint8_t data = 0;      						 
	for(i = 0; i < 8; i++)         
	{      
		data <<= 1;     
		RTE_RoundRobin_DelayUS(20);  		
		while((!DHT11_GetDataBit()));  
		RTE_RoundRobin_DelayUS(25);   
		if(DHT11_GetDataBit())    {  
			data |= 0x01;  
			RTE_RoundRobin_DelayUS(30);     
		} else{  
			data %= 0xFE;  
		}   
	 }        
	 return data;  
}  
static uint8_t DHT11_StartSample(void)  
{  
	DHT11_DataPinSetOutput();  
	//��������18ms     
	DHT11_SetDataBit(false);  
	osDelay(18);  
	DHT11_SetDataBit(true);  
	//������������������ ������ʱ20us  
	RTE_RoundRobin_DelayUS(20);  
	//������Ϊ���� �жϴӻ���Ӧ�ź�   
	DHT11_DataPinSetInput();  
	 //�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������        
	if(!DHT11_GetDataBit())         //T !        
	{  
	 //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����       
		 while((!DHT11_GetDataBit()));  
	 //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬  
		 while((DHT11_GetDataBit()));  
		 return 1;  
	}  
	return 0;                 
} 
void DHT11_GetData(void)  
{        
	uint8_t CheckTemp,TemperHigh8Temp,TemperLow8Temp,HumHigh8Temp,HumLow8Temp;
	if(DHT11_StartSample())
	{  
		//���ݽ���״̬           
		HumHigh8Temp = DHT11_ReadByte();  
		HumLow8Temp = DHT11_ReadByte();  
		TemperHigh8Temp = DHT11_ReadByte();  
		TemperLow8Temp = DHT11_ReadByte();  
		CheckTemp = DHT11_ReadByte();  
		/* Data transmission finishes, pull the bus high */  
		DHT11_DataPinSetOutput();     
		DHT11_SetDataBit(true);  
		//����У��   
		DHT11Result.CheckData=(HumHigh8Temp+HumLow8Temp+TemperHigh8Temp+TemperLow8Temp);  
		if(CheckTemp == DHT11Result.CheckData)  
		{  
			DHT11Result.HumdityHigh8 = HumHigh8Temp;  
			DHT11Result.HumdityLow8 = HumLow8Temp;  
			DHT11Result.TemperHigh8 = TemperHigh8Temp;  
			DHT11Result.TemperLow8 = TemperLow8Temp;
		}
	}
}
