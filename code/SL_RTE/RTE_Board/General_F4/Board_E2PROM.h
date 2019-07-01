#ifndef __BOARD_E2PROM_H
#define __BOARD_E2PROM_H
#include "RTE_Include.h"
//EEPROM24c02��غ���
uint8_t AT24C02_ReadByte(uint8_t ReadAddr);							     //ָ����ַ��ȡһ���ֽ�
void AT24C02_WriteByte(uint8_t WriteAddr,uint8_t DataToWrite);		//ָ����ַд��һ���ֽ�

uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode);

void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum);	//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum);   	//��ָ����ַ��ʼ����ָ�����ȵ�����

uint8_t AT24C02_Test(void);  //�������
void AT24C02_Init(void); //��ʼ��IIC
#endif
