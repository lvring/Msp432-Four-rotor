#ifndef __DATA_IMU_H
#define __DATA_IMU_H
#include "RTE_Include.h"
#define M_PI  (float)3.1415926535
typedef struct
{
	uint32_t LastTick,NowTick;                 //�������ڼ��� ��λus Ϊ���ּ������
	float exInt, eyInt, ezInt;                 //���ٶȽ������ٶ���������
	float q0, q1, q2, q3;                      //ȫ����Ԫ��
//	float q0_yaw, q1_yaw, q2_yaw, q3_yaw;      //��Ԫ�� Ϊyaw���� ��ֹ����Ư��
	float Result[3];                           //ŷ����
}IMU_Data_t;	


//Mini IMU AHRS �����API
void IMU_init(volatile IMU_Data_t *thisIMUData); //��ʼ��
void IMU_getAngle(volatile IMU_Data_t *thisIMUData) ;//������̬


void AHRSupdate(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MahonyAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT);

void MadgwickAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT);


extern volatile IMU_Data_t IMU_Data;





#endif

//------------------End of File----------------------------
