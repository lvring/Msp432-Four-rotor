#include "board_mpu.h"
#include "BSP_Systick.h"
/*低通滤波参数*/
#define GYRO_LPF_CUTOFF_FREQ  50
#define ACCEL_LPF_CUTOFF_FREQ 30
#define MAG_LPF_CUTOFF_FREQ 30

lpf2pData accLpf[3];
lpf2pData gyroLpf[3];
lpf2pData magLpf[3];

bool MPU6500Init(void)
{
	RTE_Printf("%d\r\n",MPU6500_Read_Reg(WHO_AM_I));
	RTE_RoundRobin_DelayMS(100);
	MPU6500_Write_Reg(PWR_MGMT_1,0X80);   		//电源管理,复位MPU6500
	RTE_RoundRobin_DelayMS(100);
	if(MPU6500_Read_Reg(WHO_AM_I) == 112)	//正确读取到6500的ID
	{		
		RTE_Printf("hello mpu6500\r\n");
		MPU6500_Write_Reg(PWR_MGMT_1,0X80);   		//电源管理,复位MPU6500
		RTE_RoundRobin_DelayMS(100);
		MPU6500_Write_Reg(SIGNAL_PATH_RESET,0X07);//陀螺仪、加速度计、温度计复位
		RTE_RoundRobin_DelayMS(100);
		MPU6500_Write_Reg(PWR_MGMT_1,0X01);   //选择时钟源
		MPU6500_Write_Reg(PWR_MGMT_2,0X00);   //使能加速度计和陀螺仪
		MPU6500_Write_Reg(CONFIG,0X03);				//低通滤波器 0x03 41hz MPU6500_ACCEL_DLPF_BW_41
		MPU6500_Write_Reg(SMPLRT_DIV,0X00);		//采样率1000/(1+0)=1000HZ
		MPU6500_Write_Reg(GYRO_CONFIG,0X18);  //陀螺仪测量范围 0X18 正负2000度 
		MPU6500_Write_Reg(ACCEL_CONFIG,0x10); //加速度计测量范围 0X10 正负8g
		MPU6500_Write_Reg(ACCEL_CONFIG2,0x02);//低通滤波器98hz MPU6500_DLPF_BW_98
		
		for (uint8_t i = 0; i < 3; i++)// 初始化加速计和陀螺二阶低通滤波
		{
			lpf2pInit(&gyroLpf[i], 1000, GYRO_LPF_CUTOFF_FREQ);
			lpf2pInit(&accLpf[i],  1000, ACCEL_LPF_CUTOFF_FREQ);
			lpf2pInit(&magLpf[i],  1000, MAG_LPF_CUTOFF_FREQ);
		}
		return true;
	}
	else 
		return false;
}

