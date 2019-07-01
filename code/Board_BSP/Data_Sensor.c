/*
//处理6500原始数据 为姿态解算服务
//SJ 2018.1.18
*/ 
#include "Data_Sensor.h"
#include "board_mpu.h"
#include "board_spi.h"

#define SENSOR_MAX_G 8.0f		//constant g		// tobe fixed to 8g. but IMU need to  correct at the same time
#define SENSOR_MAX_W 2000.0f	//deg/s
#define ACC_SCALE  (SENSOR_MAX_G/32768.0f)
#define GYRO_SCALE  (SENSOR_MAX_W/32768.0f)
#define M_PI_F 3.1415926f
#define CONSTANTS_ONE_G					9.80665f		/* m/s^2		*/




S_INT16_XYZ	MPU6500_Acc_Offset	=	{0,0,0};    //校准重力加速度		
S_INT16_XYZ	MPU6500_Gyro_Offset	=	{0,0,0};    //校准角速度



S_INT16_XYZ MPU6500_Acc = {0,0,0};            //加速度原始数据
S_INT16_XYZ MPU6500_Gyro = {0,0,0};           //角度原始数据



Axis3f	gyroRaw;
Axis3f	accRaw;
Axis3f  magRaw; //滤波器




int16_t  MPU6500_FIFO[6][11];                 //6行10列 队列 保存10次数据 最后数据是平均值
int16_t  AK8963_FIFO[3][11];                 //6行10列 队列 保存10次数据 最后数据是平均值

/*
 * 函数名：MPU6500DataInit
 * 描述  ：数据初始化 申请存放6500数据的内存
 * 输入  ：ax ay az gx gy gz 6500输出原始数据
 * 输出  ：无
 */ 
volatile static uint8_t *MPU6500SensorBuffer;
bool MPU6500DataInit(void)
{
	MPU6500SensorBuffer=(uint8_t *)RTE_BGetz(MEM_RTE,14);
	if(MPU6500SensorBuffer!=0)
		return true;
	else 
		return false;
}

 /*
 * 函数名：MPU6500_ReadValue
 * 描述  ：读取MPU6500原始数据 滤波但没有矫正
 * 输入  ：无
 * 输出  ：无
 */ 


static void MPUReadValue(void)
{
	uint8_t i;                                       //读取数据个数
	MPU6500_CS(0); 																 	 //使能6500传输
	SPI1_Read_Write_Byte(ACCEL_XOUT_H|0x80); 				 //从加速度计的寄存器开始进行读取陀螺仪和加速度计的值  发送读命令+寄存器号
	for(i	=	0;i	<	14;i++)														 //一共读取14字节的数据 其中有用的时12个字节
	{
		MPU6500SensorBuffer[i]	=	SPI1_Read_Write_Byte(0xff);	 
	}	
	MPU6500_Acc.X = Byte16(int16_t, MPU6500SensorBuffer[0],  MPU6500SensorBuffer[1]);
	MPU6500_Acc.Y = Byte16(int16_t, MPU6500SensorBuffer[2],  MPU6500SensorBuffer[3]);
	MPU6500_Acc.Z = Byte16(int16_t, MPU6500SensorBuffer[4],  MPU6500SensorBuffer[5]);
	MPU6500_Gyro.X = Byte16(int16_t, MPU6500SensorBuffer[8],  MPU6500SensorBuffer[9]);
	MPU6500_Gyro.Y = Byte16(int16_t, MPU6500SensorBuffer[10],  MPU6500SensorBuffer[11]);
	MPU6500_Gyro.Z = Byte16(int16_t, MPU6500SensorBuffer[12],  MPU6500SensorBuffer[13]);

	MPU6500_CS(1);  	                              //禁止6500传输
}

void MPU6500_Date_Offset(uint16_t cnt)
{
	MPU6500_Acc_Offset.X =	0;                                 							 
	MPU6500_Acc_Offset.Y =	0;
	MPU6500_Acc_Offset.Z =	0;
	
	MPU6500_Gyro_Offset.X = 0;                                							 
	MPU6500_Gyro_Offset.Y = 0;
	MPU6500_Gyro_Offset.Z = 0;
	
	                                          
	for(uint16_t i = cnt; i > 0; i--)                                        
	{
    MPUReadValue();   
		
		MPU6500_Acc_Offset.X	+=	MPU6500_Acc.X;
		MPU6500_Acc_Offset.Y	+=	MPU6500_Acc.Y;		
		MPU6500_Acc_Offset.Z	+=	MPU6500_Acc.Z;
		
		MPU6500_Gyro_Offset.X	+=	MPU6500_Gyro.X;
		MPU6500_Gyro_Offset.Y	+=	MPU6500_Gyro.Y;
		MPU6500_Gyro_Offset.Z	+=	MPU6500_Gyro.Z;
		
	}
	MPU6500_Acc_Offset.X 		=		MPU6500_Acc_Offset.X	/	cnt;                    
	MPU6500_Acc_Offset.Y 		=		MPU6500_Acc_Offset.Y	/	cnt;
	MPU6500_Acc_Offset.Z  	=		MPU6500_Acc_Offset.Z	/	cnt;
	
	MPU6500_Gyro_Offset.X		= 	MPU6500_Gyro_Offset.X	/	cnt;                     //陀螺仪有温飘 不能省略         
	MPU6500_Gyro_Offset.Y		=		MPU6500_Gyro_Offset.Y	/	cnt;
	MPU6500_Gyro_Offset.Z 	=		MPU6500_Gyro_Offset.Z	/	cnt;

}





	
//10.0*4912.0/32760.0; // Proper scale to return milliGauss
void Deal_value(void)
{

	MPUReadValue();
	
	gyroRaw.x = (MPU6500_Gyro.X - MPU6500_Gyro_Offset.X)*GYRO_SCALE ;
	gyroRaw.y = (MPU6500_Gyro.Y - MPU6500_Gyro_Offset.Y)*GYRO_SCALE ;
	gyroRaw.z = (MPU6500_Gyro.Z - MPU6500_Gyro_Offset.Z)*GYRO_SCALE ;
	applyAxis3fLpf(gyroLpf, &gyroRaw);
	
	accRaw.x = (MPU6500_Acc.X - MPU6500_Acc_Offset.X)* ACC_SCALE *CONSTANTS_ONE_G;
	accRaw.y = (MPU6500_Acc.Y - MPU6500_Acc_Offset.Y)* ACC_SCALE *CONSTANTS_ONE_G;
	accRaw.z = (MPU6500_Acc.Z - MPU6500_Acc_Offset.Z)* ACC_SCALE *CONSTANTS_ONE_G;

	applyAxis3fLpf(gyroLpf, &gyroRaw);


}


///*二阶低通滤波*/
static void applyAxis3fLpf(lpf2pData *data, Axis3f* in)
{
	for (uint8_t i = 0; i < 3; i++) 
	{
		in->axis[i] = lpf2pApply(&data[i], in->axis[i]);
	}
}



