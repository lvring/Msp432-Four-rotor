#ifndef __DATA_MPU6500_H
#define __DATA_MPU6500_H

#include "RTE_Include.h"

#include "Board_MPU9250.h"


#define Byte16(Type, ByteH, ByteL)  ((Type)((((uint16_t)(ByteH))<<8) | ((uint16_t)(ByteL))))
#define Byte16(Type, ByteH, ByteL)  ((Type)((((uint16_t)(ByteH))<<8) | ((uint16_t)(ByteL))))

#if defined(__CC_ARM) 
	#pragma anon_unions
#endif

typedef struct
{
	int16_t X;
	int16_t Y;
	int16_t Z;
}S_INT16_XYZ;

typedef struct
{
	int32_t X;
	int32_t Y;
	int32_t Z;
}S_INT32_XYZ;

typedef union 
{
	struct 
	{
		float x;
		float y;
		float z;
	};
	float axis[3];
} Axis3f;

extern S_INT16_XYZ MPU6500_Acc;
extern S_INT16_XYZ MPU6500_Gyro;
extern S_INT16_XYZ AK8963_Mag;
extern Axis3f	gyroRaw;
extern Axis3f	accRaw;
extern Axis3f magRaw;



bool MPU6500DataInit(void);
void MPU6500_newValues(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz);
void MPU6500_Date_Offset(uint16_t cnt);
void Deal_value(void);



static void applyAxis3fLpf(lpf2pData *data, Axis3f* in);
#endif
