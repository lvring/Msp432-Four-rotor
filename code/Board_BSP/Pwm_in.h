#ifndef __PWM_IN_H
#define __PWM_IN_H

#include <driverlib.h>
#include "RTE_Include.h"

extern uint16_t Rc_Channel_Val[6];	  //ȫ�ֱ��� �����ң������ֵ [0]������  [1]������  [1]�ң�����  [2]�ң����� ���Զ�ʱ�������ж� ���ն��в������仯
void PWM_In_Init(void);
void TIMA1_PWM_In_Init(void);
void TIMA2_PWM_In_Init(void);

#endif


