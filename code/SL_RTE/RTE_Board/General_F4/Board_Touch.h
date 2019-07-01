#ifndef __BOARD_TOUCH_H
#define __BOARD_TOUCH_H
#include "stm32f4xx.h"
#include "RTE_Include.h"
#include "common.h"
//////////////////////////////////////////////////////////////////////////////////	 
extern uint8_t  CMD_RDX;
extern uint8_t  CMD_RDY;
//������оƬ��������	   
#define PEN  		PFin(11)  	//T_PEN
#define DOUT 		PBin(2)   	//T_MISO
#define TDIN 		PBout(1)  	//T_MOSI
#define TCLK 		PAout(5)  	//T_SCK
#define TCS  		PBout(0)  	//T_CS 
//����������������
extern	uint8_t touchtype;  //���������� 0Ϊ������  1Ϊ������ �������Ƴ�������
extern	uint16_t Xdown; 		 
extern	uint16_t Ydown;	   //�����������¾ͷ��صĵ�����ֵ
extern  uint16_t Xup;
extern  uint16_t Yup;       //������������֮��̧�𷵻صĵ�����ֵ  
//����������
/*********������SPIͨ�����ȡ������ADֵ*********************/
void TP_Write_Byte(uint8_t num);				  //�����оƬд��һ������
uint16_t TP_Read_AD(uint8_t CMD);							//��ȡADת��ֵ
uint16_t TP_Read_XorY(uint8_t xy);							//���˲��������ȡ(X/Y)
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);				//˫�����ȡ(X+Y)
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);			//���ζ�ȡ��˫���������ȡ

/*********��������ʼ������*********************/
uint8_t Touch_Init(void);						 	//��ʼ��
/*********������ɨ�谴������*********************/
void TP_Scan(uint8_t tp);	        //tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
void TP_Draw_Point(uint16_t x,uint16_t y,uint16_t color);	//�ڰ��´���һ����				


#endif
