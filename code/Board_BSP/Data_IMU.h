#ifndef __DATA_IMU_H
#define __DATA_IMU_H
#include "RTE_Include.h"
#define M_PI  (float)3.1415926535
typedef struct
{
	uint32_t LastTick,NowTick;                 //采样周期计数 单位us 为积分计算服务
	float exInt, eyInt, ezInt;                 //加速度矫正角速度误差积分量
	float q0, q1, q2, q3;                      //全局四元数
//	float q0_yaw, q1_yaw, q2_yaw, q3_yaw;      //四元数 为yaw服务 防止积分漂移
	float Result[3];                           //欧拉角
}IMU_Data_t;	


//Mini IMU AHRS 解算的API
void IMU_init(volatile IMU_Data_t *thisIMUData); //初始化
void IMU_getAngle(volatile IMU_Data_t *thisIMUData) ;//更新姿态


void AHRSupdate(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MahonyAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT);

void MadgwickAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT);


extern volatile IMU_Data_t IMU_Data;





#endif

//------------------End of File----------------------------
