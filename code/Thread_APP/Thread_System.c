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
#include "board_spi.h"
#include "board_mpu.h"

static void LEDTimer_Callback(void* UserParameters) {
//	RTE_Printf("m1 %d,m2 %d,m3 %d,m4 %d  x %d  y %d  z %d\r\n",motor[1],motor[2],motor[3],motor[4],
//								(int)Exp_Angle.X,(int)Exp_Angle.Y,(int)Exp_Angle.Z);
//	RTE_Printf("RC1:%d RC2:%d RC3:%d RC4:%d RC5:%d RC6:%d\r\n",Rc_Channel_Val[0],Rc_Channel_Val[1],Rc_Channel_Val[2],
//		Rc_Channel_Val[3],Rc_Channel_Val[4],Rc_Channel_Val[5]);
	RTE_Printf("yaw:%d    roll:%d    pitch:%d\r\n",(int)(IMU_Data.Result[0]),(int)(IMU_Data.Result[1]),(int)(IMU_Data.Result[2]));
	BSP_LED_Toggle(LED_A);
}
uint64_t system_thread_stk[1024/8] = {0};
const osThreadAttr_t SystemThreadControl = {
	.stack_mem = &system_thread_stk[0],
  .stack_size =  sizeof(system_thread_stk) , 
	.priority = osPriorityRealtime,
};
osThreadId_t ThreadIDSYS;


//计算cpu使用
__IO uint32_t  StaticsIdleCnt;        /* 空闲任务计数 */
__IO uint8_t   StaticsCPUUsage;       /* CPU百分比 */
uint32_t       StaticsIdleCntMax;     /* 1秒内最大的空闲计数 */
uint32_t       StaticsIdleCntNow;     /* 1秒内空闲任务当前计数 */
static void SystemStatics_TimerCallback (void *UserParameters) {
	StaticsIdleCntNow = StaticsIdleCnt;    /* 获得100ms内空闲计数 */
	StaticsIdleCnt    = 0uL;          /* 复位空闲计数 */
	StaticsCPUUsage   = (100uL - StaticsIdleCntNow *100 / StaticsIdleCntMax);  /* 计算100ms内的CPU利用率 */
}
__NO_RETURN void ThreadTaskSYS(void *argument){
	osDelay(2);
	StaticsIdleCnt = 0; 
	osDelay(100);
	StaticsIdleCntMax = StaticsIdleCnt; 
	RTE_Init();
	BSP_LED_Init();
	Board_Usart_Init(COM_1);
	SPI1_Init();
	RTE_RoundRobin_CreateTimer(0,"LEDTimer",500,1,1,LEDTimer_Callback,(void *)0);
	//RTE_RoundRobin_CreateTimer(0,"StaticsTimer",100,1,1,SystemStatics_TimerCallback,(void *)0);
	RTE_Printf("SystemCoreClock:%d\r\n",SystemCoreClock);
	RTE_RoundRobin_Demon();
	ThreadIDStabilizer = osThreadNew(ThreadTaskStabilizer, NULL, &stabilizerTask); 
	ThreadIDDataSensor = osThreadNew(ThreadTaskDatasensor, NULL, &DataSensorTask); 
  for (;;) 
	{
		RTE_RoundRobin_TickHandler();
    osDelay(1);
  }
}




