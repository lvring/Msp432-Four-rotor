#ifndef __R_Touch_Adjust_H
#define __R_Touch_Adjust_H	 

#include "common.h" 
 

extern float xfac;		 //������У׼����(����������ҪУ׼) 
extern float yfac;
extern short xoff;
extern short yoff;	 

/*********������У׼�õ��ĺ���*********************/
void TP_Save_Adjdata(void);				//����У׼����
void TP_Adjust(void);							//������У׼
uint8_t TP_Get_Adjdata(void);					//��ȡУ׼����
void TP_Adjdata_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);//��ʾУ׼��Ϣ


#endif
