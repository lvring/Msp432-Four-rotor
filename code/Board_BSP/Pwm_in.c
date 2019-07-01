/*
//定时器输入捕获文件 为得到接收机信号服务
//SJ 2018.1.18
*/
#include "pwm_in.h"

uint8_t Rc_Capture_Sta[7] = {0};//6个通道的捕获状态
uint16_t Rc_Channel_Val[6] = {0};//6个通道的捕获值
long Rc_Channel_tempVal[6] = {0};//6个通道的捕获值


/*
 * 函数名：Pwm_In_Init
 * 描述  ：输入捕获初始化函数 
 * 输入  ：arr：自动重装载寄存器周期的值；psc：时钟频率除数的预分频值 参数设置与接收机匹配 保证能够准确捕获接收机信号
 * 输出  ：无
 */ 
 
void PWM_In_Init(void)
{	 
	//printf("TIMA1_PWM_In_Init TIMA3_PWM_In_Init\r\n");
   TIMA1_PWM_In_Init();//TIM2 四路负责四路电机pwm
	
}


const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,           // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_12,       // SMCLK/12 =1MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_SKIP_CLEAR                   // Skup Clear Counter
};
///* Timer_A Capture Mode Configuration Parameter */
//const Timer_A_CaptureModeConfig captureModeConfig =
//{
//        TIMER_A_CAPTURECOMPARE_REGISTER_1,        // CC Register 2
//        TIMER_A_CAPTUREMODE_RISING_EDGE,          // Rising Edge
//        TIMER_A_CAPTURE_INPUTSELECT_CCIxB,        // CCIxB Input Select
//        TIMER_A_CAPTURE_SYNCHRONOUS,              // Synchronized Capture
//        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,  // Enable interrupt
//        TIMER_A_OUTPUTMODE_OUTBITVALUE            // Output bit value
//};

Timer_A_CaptureModeConfig captureModeConfig;
Timer_A_CaptureModeConfig captureModeConfig_A2;
void TIMA1_PWM_In_Init(void)
{
	
	
	GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P7,GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P7, GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7,
            GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P8,GPIO_PIN0);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P8, GPIO_PIN0 ,GPIO_SECONDARY_MODULE_FUNCTION);
	
	captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;  //P8.0
	captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
	captureModeConfig.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
	captureModeConfig.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
	captureModeConfig.captureInputSelect = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
	captureModeConfig.captureOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
	
	MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
	
	captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;////P7.7
	MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
	
	captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;////P7.6
	MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
	
	captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;////P7.5
	MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
	
	captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;////P7.4
	MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
	
	MAP_Timer_A_configureContinuousMode(TIMER_A1_BASE, &continuousModeConfig);
	
	Interrupt_enableInterrupt(INT_TA1_N);
	Interrupt_enableInterrupt(INT_TA1_0);
	//MAP_Interrupt_setPriority(INT_TA1_N, 0x00);
  MAP_Interrupt_enableMaster();

	MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);
	
	GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P8,GPIO_PIN1);
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P8, GPIO_PIN1 ,GPIO_SECONDARY_MODULE_FUNCTION);
	
	captureModeConfig_A2.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;  //P8.1
 	captureModeConfig_A2.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
	captureModeConfig_A2.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
	captureModeConfig_A2.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
	captureModeConfig_A2.captureInputSelect = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
	captureModeConfig_A2.captureOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
	
	MAP_Timer_A_initCapture(TIMER_A2_BASE, &captureModeConfig_A2);

	MAP_Timer_A_configureContinuousMode(TIMER_A2_BASE, &continuousModeConfig);
	
	MAP_Interrupt_enableInterrupt(INT_TA2_0);
	
  MAP_Interrupt_enableMaster();

	MAP_Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_CONTINUOUS_MODE);
	
	
}

void TA1_0_IRQHandler(void)
{
	long temp;
	//**************************捕获0发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[4])																				//捕获到下降沿 		
			{	  
				temp = TA1CCR0-Rc_Channel_tempVal[4];	
				//printf("偏航0:%ld\r\n",temp);				
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[4]	= temp;
			 	Rc_Capture_Sta[4]	=	0;		//标记捕获到下升沿
				TA1CCTL0 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;////P7.7
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕
			}
			else  																											//捕获到上升沿
			{
				TA1CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[4]	=	1;																		//标记捕获到了上升沿
				TA1CCTL0 &= ~CM_3;
				//TA1R &= 0x00 ;//TIMERA0 clear COUNT 
				Rc_Channel_tempVal[4]	=	TA1CCR0;							//读取捕获值，这里为PWM高电平时间s				
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;////P7.7
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
				MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);	
				
			}	
      MAP_Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
			return ;
		}	
	
}



void TA1_N_IRQHandler(void)
{
	long temp;
   //**************************捕获1发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[0])																				//捕获到下降沿 		
			{	  
				temp = TA1CCR1-Rc_Channel_tempVal[0];	
				//printf("偏航0:%ld\r\n",temp);				
				
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[0]	= temp;
				//printf("偏航0:%d\r\n",Rc_Channel_Val[0]);		//左  左右
			 	Rc_Capture_Sta[0]	=	0;		//标记捕获到下升沿
				TA1CCTL1 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;////P7.7
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
			
			}
			else  																											//捕获到上升沿
			{
				TA1CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[0]	=	1;																		//标记捕获到了上升沿
				TA1CCTL1 &= ~CM_3;
				//TA1R &= 0x00 ;//TIMERA0 clear COUNT 
				Rc_Channel_tempVal[0]	=	TA1CCR1;							//读取捕获值，这里为PWM高电平时间s				
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;////P7.7
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
				MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);	
				
			}	
      MAP_Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
			return ;
		}		

		
   //**************************捕获2发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[1])																				//捕获到下降沿 		
			{	  			
				temp = TA1CCR2-Rc_Channel_tempVal[1];					
				
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[1]	= temp;
			 	Rc_Capture_Sta[1]	=	0;		//标记捕获到下升沿
				TA1CCTL2 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;////P7.6
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
			}
			else  																											//捕获到上升沿
			{
				TA1CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[1]	=	1;																		//标记捕获到了上升沿
				TA1CCTL2 &= ~CM_3;
				Rc_Channel_tempVal[1]	=	TA1CCR2;							//读取捕获值，这里为PWM高电平时间
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;////P7.6
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);
				MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);
			}	
      MAP_Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_2);
		}		
		
		 //**************************捕获3发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[2])																				//捕获到下降沿 		
			{	  			
				temp = TA1CCR3-Rc_Channel_tempVal[2];					
				
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[2]	= temp;
			
			 	Rc_Capture_Sta[2]	=	0;		//标记捕获到下升沿
				TA1CCTL3 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;////P7.5
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
			}
			else  																											//捕获到上升沿
			{
				TA1CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[2]	=	1;																		//标记捕获到了上升沿
				TA1CCTL3 &= ~CM_3;
				Rc_Channel_tempVal[2]	=	TA1CCR3;							//读取捕获值，这里为PWM高电平时间
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;////P7.5
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			
				MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);
			}	
      MAP_Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3);
			
		}	

		 //**************************捕获4发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[3])																				//捕获到下降沿 		
			{	  			
				temp = TA1CCR4-Rc_Channel_tempVal[3];					
				
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[3]	= temp;
				
			 	Rc_Capture_Sta[3]	=	0;		//标记捕获到下升沿
				TA1CCTL4 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;          //P7.4
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			//设置为上升沿捕获
			}
			else  																											//捕获到上升沿
			{
				TA1CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[3]	=	1;																		//标记捕获到了上升沿
				TA1CCTL4 &= ~CM_3;
				Rc_Channel_tempVal[3]	=	TA1CCR4;							//读取捕获值，这里为PWM高电平时间
				captureModeConfig.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;////P7.4
				captureModeConfig.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				MAP_Timer_A_initCapture(TIMER_A1_BASE, &captureModeConfig);			
				MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_CONTINUOUS_MODE);
			}	
      MAP_Timer_A_clearCaptureCompareInterrupt( TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4);
			
		}		
		
}


/*
 * 函数名：TIM2_PWM_In_Init
 * 描述  ：定时器2输入捕获初始化函数
 */ 


void TA2_0_IRQHandler(void)
{
	long temp;

		//**************************捕获5发生捕获事件**************************	
	if (Timer_A_getCaptureCompareInterruptStatus(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0,TIMER_A_CCTLN_CCIFG))
		{	
			if(Rc_Capture_Sta[5])																				//捕获到下降沿 		
			{	  			
				temp = TA2CCR0-Rc_Channel_tempVal[5];					
				if(temp < 0)
				{
					temp = temp + 65536;
				}
				Rc_Channel_Val[5]	= temp;
			 	Rc_Capture_Sta[5]	=	0;		//标记捕获到下升沿
				TA2CCTL0 &= ~CM_3;			//先要设置不捕获，才能设置其他捕获模式
				captureModeConfig_A2.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
				captureModeConfig_A2.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;////P10.4
				Timer_A_initCapture(TIMER_A2_BASE, &captureModeConfig_A2);			//设置为上升沿捕获
			}
			else  																											//捕获到上升沿
			{
				TA2CTL &= ~MC_3;//先要停止计数器才能进行清零操作
				Rc_Capture_Sta[5]	=	1;																		//标记捕获到了上升沿
				TA2CCTL0 &= ~CM_3;
				Rc_Channel_tempVal[5]	=	TA2CCR0;							//读取捕获值，这里为PWM高电平时间
				captureModeConfig_A2.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;////P10.4
				captureModeConfig_A2.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;			//设置为下降沿捕获
				Timer_A_initCapture(TIMER_A2_BASE, &captureModeConfig_A2);			//设置为上升沿捕获
				Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_CONTINUOUS_MODE);
			}	
      Timer_A_clearCaptureCompareInterrupt( TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
		}
		
}




