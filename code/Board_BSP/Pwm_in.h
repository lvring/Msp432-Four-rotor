#ifndef __PWM_IN_H
#define __PWM_IN_H

#include <driverlib.h>
#include "RTE_Include.h"

extern uint16_t Rc_Channel_Val[6];	  //全局变量 捕获的遥控器的值 [0]左：左右  [1]左：上下  [1]右：左右  [2]右：上下 来自定时器捕获中断 在终端中产生并变化
void PWM_In_Init(void);
void TIMA1_PWM_In_Init(void);
void TIMA2_PWM_In_Init(void);

#endif


