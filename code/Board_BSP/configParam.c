#include "configParam.h"




configParam_t configParam =
{
	.pidAngle=	/*�Ƕ�PID*/
	{	
		.roll=
		{
			.kp=2.5,
			.ki=0,
			.kd=0,
		},
		.pitch=
		{
			.kp=2.5,
			.ki=0,
			.kd=0,
		},
		.yaw=
		{
			.kp=5,
			.ki=0,
			.kd=0,
		},
	},	
	.pidRate=	/*���ٶ�PID*/
	{	
		.roll=
		{
			.kp=1.88,
			.ki=0.4,
			.kd=0.012,
		},
		.pitch=
		{
			.kp=1.88,
			.ki=0.4,
			.kd=0.012,
		}, 
		.yaw=
		{
			.kp=3,
			.ki=0,
			.kd=0.011 ,
		},
	},	
};







