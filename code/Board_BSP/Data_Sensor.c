/*
//����6500ԭʼ���� Ϊ��̬�������
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




S_INT16_XYZ	MPU6500_Acc_Offset	=	{0,0,0};    //У׼�������ٶ�		
S_INT16_XYZ	MPU6500_Gyro_Offset	=	{0,0,0};    //У׼���ٶ�



S_INT16_XYZ MPU6500_Acc = {0,0,0};            //���ٶ�ԭʼ����
S_INT16_XYZ MPU6500_Gyro = {0,0,0};           //�Ƕ�ԭʼ����



Axis3f	gyroRaw;
Axis3f	accRaw;
Axis3f  magRaw; //�˲���




int16_t  MPU6500_FIFO[6][11];                 //6��10�� ���� ����10������ ���������ƽ��ֵ
int16_t  AK8963_FIFO[3][11];                 //6��10�� ���� ����10������ ���������ƽ��ֵ

/*
 * ��������MPU6500DataInit
 * ����  �����ݳ�ʼ�� ������6500���ݵ��ڴ�
 * ����  ��ax ay az gx gy gz 6500���ԭʼ����
 * ���  ����
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
 * ��������MPU6500_ReadValue
 * ����  ����ȡMPU6500ԭʼ���� �˲���û�н���
 * ����  ����
 * ���  ����
 */ 


static void MPUReadValue(void)
{
	uint8_t i;                                       //��ȡ���ݸ���
	MPU6500_CS(0); 																 	 //ʹ��6500����
	SPI1_Read_Write_Byte(ACCEL_XOUT_H|0x80); 				 //�Ӽ��ٶȼƵļĴ�����ʼ���ж�ȡ�����Ǻͼ��ٶȼƵ�ֵ  ���Ͷ�����+�Ĵ�����
	for(i	=	0;i	<	14;i++)														 //һ����ȡ14�ֽڵ����� �������õ�ʱ12���ֽ�
	{
		MPU6500SensorBuffer[i]	=	SPI1_Read_Write_Byte(0xff);	 
	}	
	MPU6500_Acc.X = Byte16(int16_t, MPU6500SensorBuffer[0],  MPU6500SensorBuffer[1]);
	MPU6500_Acc.Y = Byte16(int16_t, MPU6500SensorBuffer[2],  MPU6500SensorBuffer[3]);
	MPU6500_Acc.Z = Byte16(int16_t, MPU6500SensorBuffer[4],  MPU6500SensorBuffer[5]);
	MPU6500_Gyro.X = Byte16(int16_t, MPU6500SensorBuffer[8],  MPU6500SensorBuffer[9]);
	MPU6500_Gyro.Y = Byte16(int16_t, MPU6500SensorBuffer[10],  MPU6500SensorBuffer[11]);
	MPU6500_Gyro.Z = Byte16(int16_t, MPU6500SensorBuffer[12],  MPU6500SensorBuffer[13]);

	MPU6500_CS(1);  	                              //��ֹ6500����
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
	
	MPU6500_Gyro_Offset.X		= 	MPU6500_Gyro_Offset.X	/	cnt;                     //����������Ʈ ����ʡ��         
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


///*���׵�ͨ�˲�*/
static void applyAxis3fLpf(lpf2pData *data, Axis3f* in)
{
	for (uint8_t i = 0; i < 3; i++) 
	{
		in->axis[i] = lpf2pApply(&data[i], in->axis[i]);
	}
}



