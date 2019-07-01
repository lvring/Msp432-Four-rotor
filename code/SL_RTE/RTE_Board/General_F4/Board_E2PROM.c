#include "Board_E2PROM.h"
#include "Board_I2C.h"
/*******************************************************************************
*************************����ΪEEPROM24C02��д����******************************
*******************************************************************************/
//��ʼ��24c02��IIC�ӿ�
void AT24C02_Init(void)
{
	SoftI2C_Init();  //IIC��ʼ��
}
/****************************************************************************
* ��    ��: uint8_t AT24C02_ReadByte(uint8_t ReadAddr)
* ��    �ܣ���AT24C02ָ����ַ����һ������
* ��ڲ�����ReadAddr��Ҫ��ȡ�������ڵĵ�ַ
* ���ز�����������8λ����
* ˵    ����  �B
****************************************************************************/
uint8_t AT24C02_ReadByte(uint8_t ReadAddr)
{				  
	uint8_t receivedata=0;		  	    																 
  
	SoftI2C_StartSignal();  
	SoftI2C_SendByte(0XA0);           //����������ַ0XA0
	SoftI2C_WaitAck();
  SoftI2C_SendByte(ReadAddr);       //���͵͵�ַ
	SoftI2C_WaitAck();	    
	SoftI2C_StartSignal();  	 	   
	SoftI2C_SendByte(0XA1);           //�������ģʽ			   
	SoftI2C_WaitAck();	 
  receivedata=SoftI2C_ReadByte(0);		   
  SoftI2C_StopSignal();                    //����һ��ֹͣ����	    
	
	return receivedata;
}
/****************************************************************************
* ��    ��: uint8_t AT24C02_WriteByte(uint8_t WriteAddr,uint8_t WriteData)
* ��    �ܣ���AT24C02ָ����ַд��һ������
* ��ڲ�����WriteAddr��Ҫд���������ڵĵ�ַ
            WriteData: Ҫд�������
* ���ز����� 
* ˵    ����  �B
****************************************************************************/
void AT24C02_WriteByte(uint8_t WriteAddr,uint8_t WriteData)
{				   	  	    																 
  SoftI2C_StartSignal();  
	SoftI2C_SendByte(0XA0);       //����0XA0,д���� 	 
	SoftI2C_WaitAck();	   
  SoftI2C_SendByte(WriteAddr);  //���͵͵�ַ
	SoftI2C_WaitAck(); 	 										  		   
	SoftI2C_SendByte(WriteData);  //�����ֽ�							   
	SoftI2C_WaitAck();  		    	   
  SoftI2C_StopSignal();                    //����һ��ֹͣ���� 
	RTE_RoundRobin_DelayMS(10);	 
}
/****************************************************************************
* ��    ��: uint8_t AT24C02_Test(void)
* ��    �ܣ�����AT24C02�Ƿ�����
* ��ڲ�������
* ���ز���������1:���ʧ��
            ����0:���ɹ� 
* ˵    ����  �B
****************************************************************************/
uint8_t AT24C02_Test(void)
{
	uint8_t Testdata;
	Testdata=AT24C02_ReadByte(255); //����������ԣ�����ֵ�����ٴ�д��	   
	if(Testdata==0XAB)return 0;		   
	else                             
	{
		AT24C02_WriteByte(255,0XAB);
	  Testdata=AT24C02_ReadByte(255);	  
		if(Testdata==0XAB)return 0;
	}
	return 1;											  
}
/****************************************************************************
* ��    ��: uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode)
* ��    �ܣ���λ�����ֽڻ�ת
* ��ڲ�����mode��1:��λ��ת�ֳ��ֽ�   0:�ֽںϲ���һ����λ��
            Byte_num����Ҫת�����ֽ���
            *pBuffer���ֽڽ���������ֽ���������
            Date_4Byte����λ����
* ���ز�����modeΪ0ʱ�����ض�λ��
* ˵    ����Byte_num���Ϊ4���ֽڣ��ú����ں���Ĵ�����У��ʱ��ȡУ��ֵ���õ�
****************************************************************************/
uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode)
{
  uint8_t i; uint32_t middata=0;
	if(mode)    //��λ��ת�ֳ��ֽ�
	 {
	   for(i=0;i<Byte_num;i++)
	     {
	       *pBuffer++ =(Date_4Byte>>(8*i))&0xff;
	     }
			return 0; 
	 } 
	 else       //�ֽںϲ���һ����λ��
	 {
	    Date_4Byte=0;
		  pBuffer+=(Byte_num-1);
		  for(i=0;i<Byte_num;i++)
	      { 		
		      middata<<=8;
		      middata+= *pBuffer--;			   
	      }
			return middata;	
	 }
}
/****************************************************************************
* ��    ��: void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum)
* ��    �ܣ���AT24C02�����ָ����ַ��ʼ����ָ������������
* ��ڲ�����ReadAddr :��ʼ�����ĵ�ַ  0~255
            pBuffer  :���������׵�ַ
            ReadNum:Ҫ�������ݵĸ���
* ���ز�����
* ˵    ����  �B
****************************************************************************/
void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum)
{
	while(ReadNum--)
	{
		*pBuffer++=AT24C02_ReadByte(ReadAddr++);	
	}
} 
/****************************************************************************
* ��    ��: void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum)
* ��    �ܣ���AT24C02�����ָ����ַ��ʼд��ָ������������
* ��ڲ�����WriteAddr :��ʼд��ĵ�ַ  0~255
            pBuffer  :���������׵�ַ
            WriteNum:Ҫд�����ݵĸ���
* ���ز�����
* ˵    ����  �B
****************************************************************************/
void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum)
{
	while(WriteNum--)
	{
		AT24C02_WriteByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}
