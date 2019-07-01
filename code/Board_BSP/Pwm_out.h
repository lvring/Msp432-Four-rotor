#ifndef __PWM_OUT_H
#define __PWM_OUT_H

#include <driverlib.h>


#define MOTOR_PWM_MAX 3500//3500

void PWM_Out_Init(uint16_t arr,uint16_t	CCR1,uint16_t	CCR2,uint16_t	CCR3,uint16_t	CCR4);
void Motor_PWM_Update(int16_t motor1_pwm,int16_t motor2_pwm,int16_t motor3_pwm,int16_t motor4_pwm);

#endif


