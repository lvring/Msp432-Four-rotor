/*
//定时器输出PWM 为电机控制服务
//SJ 2018.1.18
*/
#include "pwm_out.h"
/*
 * 函数名：TIM4_Init
 * 描述  ：定时器4的PWM输出初始化函数 实现四路PWM输出控制电机
 * 输入  ：arr：自动重装载寄存器周期的值；psc：时钟频率除数的预分频值；CCRx：对应四路PWM输出占空比 其中arr和psc配合电调接收PWM要求设置
 * 输出  ：无
 */ 
// 
// typedef struct _Timer_A_PWMConfig
//{
//    uint_fast16_t clockSource;
//    uint_fast16_t clockSourceDivider;
//    uint_fast16_t timerPeriod;
//    uint_fast16_t compareRegister;
//    uint_fast16_t compareOutputMode;
//    uint_fast16_t dutyCycle;
//} Timer_A_PWMConfig;

// 
 
//Timer_A_PWMConfig pwmConfig =
//{
//        TIMER_A_CLOCKSOURCE_SMCLK,//12MHz
//        TIMER_A_CLOCKSOURCE_DIVIDER_6,
//        5000,
//        TIMER_A_CAPTURECOMPARE_REGISTER_1,
//        TIMER_A_OUTPUTMODE_RESET_SET,
//        0
//};
//Timer_A_setCompareValue


void PWM_Out_Init(uint16_t arr,uint16_t	CCR1,uint16_t	CCR2,uint16_t	CCR3,uint16_t	CCR4)//CCR为16位的数
{	 
	Timer_A_PWMConfig pwmConfig;
	
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7,
            GPIO_PRIMARY_MODULE_FUNCTION);
	
	pwmConfig.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
	pwmConfig.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_6;
	pwmConfig.timerPeriod = arr;
	pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
	pwmConfig.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
	pwmConfig.dutyCycle = CCR1;
	
	   /* Configuring Timer_A to have a period of approximately 2.5ms and
     * an initial duty cycle of 10% of that (x ticks)  */
	MAP_Timer_A_generatePWM(TIMER_A0_BASE,&pwmConfig);
	
	pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
	pwmConfig.dutyCycle = CCR2;
	MAP_Timer_A_generatePWM(TIMER_A0_BASE,&pwmConfig);
	
	pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
	pwmConfig.dutyCycle = CCR3;
	MAP_Timer_A_generatePWM(TIMER_A0_BASE,&pwmConfig);
	
	pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
	pwmConfig.dutyCycle = CCR4;
	MAP_Timer_A_generatePWM(TIMER_A0_BASE,&pwmConfig);
}
/*
 * 函数名：Motor_PWM_Update
 * 描述  ：电机PWM输出更新函数
 * 输入  ：motorx_pwm：电机PWM输入值
 * 输出  ：无
							Y(Roll)机头
			  顺时针转 | 逆时针转
			B7一号电机 | 四号电机B6
       -------------------- X(Pitch)         
				逆时针转 | 顺时针转 
		  B8二号电机 | 三号电机B9
	*TIM4 PWM输出初始化.配合电调要求：ARR=5000 36分频 PWM频率400Hz  一个周期有5000个脉冲
 */ 

void Motor_PWM_Update(int16_t motor1_pwm,int16_t motor2_pwm,int16_t motor3_pwm,int16_t motor4_pwm)
{		
	
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,motor1_pwm);
	
	
	TA0CCR1 = motor1_pwm;//TIM4->CCR2存放pwm占空比   motor1_pwm一号电机
	TA0CCR2 = motor2_pwm;
	TA0CCR3 = motor3_pwm;
	TA0CCR4 = motor4_pwm;
}

