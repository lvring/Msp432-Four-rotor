#include "Thread_stabilizer.h"
#include "Thread_System.h"
#include "BSP_Led.h"
#include "BSP_Systick.h"
#include "Board_Usart.h"

#include "Pwm_in.h"
#include "Pwm_out.h"


#include "control.h"
#include "telecontrol.h"
#include "Data_Sensor.h"
#include "DATA_IMU.h"


//姿态线程
uint64_t stabilizer_thread_stk[1024/8] = {0};
const osThreadAttr_t stabilizerTask = {
	.stack_mem = &stabilizer_thread_stk[0],
  .stack_size =  sizeof(stabilizer_thread_stk) , 
	.priority = osPriorityHigh,
};
osThreadId_t ThreadIDStabilizer;


//int i=0,j=0;
__NO_RETURN void ThreadTaskStabilizer(void *argument){
	
	
	PWM_In_Init();       	     //开启输入捕获，定时精度为1us  TIM2遥控器输入捕获 TIM3 
	PWM_Out_Init(4999,0,0,0,0);			     //TIM4 PWM输出初始化.配合电调要求：ARR=5000 36分频 PWM频率400Hz      4000
	attitudeControlInit(); //pid 初始参数
  for (;;) 
	{		
		osThreadFlagsWait(0x00000001ul,osFlagsWaitAny,osWaitForever);
		IMU_getAngle(&IMU_Data);
		WFLY_PWM_Convert();                        //遥控器指令转化为期望角度
		Control(); 
//		i++;
//		RTE_Printf("i %d j %d\r\n",i,j);
//		RTE_Printf("yaw:%d    roll:%d    pitch:%d\r\n",(int)(IMU_Data.Result[0]),(int)(IMU_Data.Result[1]),(int)(IMU_Data.Result[2]));
  }
}


