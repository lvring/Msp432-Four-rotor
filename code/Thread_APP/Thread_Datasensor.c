#include "Thread_datasensor.h"
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
#include "board_mpu.h"


//�����߳�
uint64_t DataSensor_thread_stk[1024/8] = {0};
const osThreadAttr_t DataSensorTask = {
	.stack_mem = &DataSensor_thread_stk[0],
  .stack_size = sizeof(DataSensor_thread_stk), 
	.priority = osPriorityNormal,
};
osThreadId_t ThreadIDDataSensor;


__NO_RETURN void ThreadTaskDatasensor(void *argument){


	IMU_init(&IMU_Data);
	MPU6500DataInit();
	while(MPU6500Init()==false)
	{
		RTE_Printf("MPU_Init error\r\n");
		RTE_RoundRobin_DelayMS(1000);     //�ȴ������� �ȴ��������6500����ƽ��
	}		//6500Ӳ����ʼ��
	RTE_RoundRobin_DelayMS(200);
	MPU6500_Date_Offset(100);
	RTE_Printf("Start collecting Data!\r\n");
  for (;;) 
	{		
		Deal_value();			
		osThreadFlagsSet(ThreadIDStabilizer, 0x00000001ul);
		osDelay(1);
			
	}
}

