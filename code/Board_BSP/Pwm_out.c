/*
//��ʱ�����PWM Ϊ������Ʒ���
//SJ 2018.1.18
*/
#include "pwm_out.h"
/*
 * ��������TIM4_Init
 * ����  ����ʱ��4��PWM�����ʼ������ ʵ����·PWM������Ƶ��
 * ����  ��arr���Զ���װ�ؼĴ������ڵ�ֵ��psc��ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ��CCRx����Ӧ��·PWM���ռ�ձ� ����arr��psc��ϵ������PWMҪ������
 * ���  ����
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


void PWM_Out_Init(uint16_t arr,uint16_t	CCR1,uint16_t	CCR2,uint16_t	CCR3,uint16_t	CCR4)//CCRΪ16λ����
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
 * ��������Motor_PWM_Update
 * ����  �����PWM������º���
 * ����  ��motorx_pwm�����PWM����ֵ
 * ���  ����
							Y(Roll)��ͷ
			  ˳ʱ��ת | ��ʱ��ת
			B7һ�ŵ�� | �ĺŵ��B6
       -------------------- X(Pitch)         
				��ʱ��ת | ˳ʱ��ת 
		  B8���ŵ�� | ���ŵ��B9
	*TIM4 PWM�����ʼ��.��ϵ��Ҫ��ARR=5000 36��Ƶ PWMƵ��400Hz  һ��������5000������
 */ 

void Motor_PWM_Update(int16_t motor1_pwm,int16_t motor2_pwm,int16_t motor3_pwm,int16_t motor4_pwm)
{		
	
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,motor1_pwm);
//	Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,motor1_pwm);
	
	
	TA0CCR1 = motor1_pwm;//TIM4->CCR2���pwmռ�ձ�   motor1_pwmһ�ŵ��
	TA0CCR2 = motor2_pwm;
	TA0CCR3 = motor3_pwm;
	TA0CCR4 = motor4_pwm;
}

