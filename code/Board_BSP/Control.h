#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <driverlib.h>
#include "configParam.h"

#define LAUNCH_THROTTLE	400 			//起飞油门
#define ADD_THROTTLE		1500 			//补充油门



/*角度环积分限幅*/
#define PID_ANGLE_ROLL_INTEGRATION_LIMIT    30.0
#define PID_ANGLE_PITCH_INTEGRATION_LIMIT   30.0
#define PID_ANGLE_YAW_INTEGRATION_LIMIT     180.0

/*角速度环积分限幅*/
#define PID_RATE_ROLL_INTEGRATION_LIMIT		50.0
#define PID_RATE_PITCH_INTEGRATION_LIMIT	50.0
#define PID_RATE_YAW_INTEGRATION_LIMIT		50.0


#define DEFAULT_PID_INTEGRATION_LIMIT 		500.0 //默认pid的积分限幅
#define DEFAULT_PID_OUTPUT_LIMIT      		0.0	  //默认pid输出限幅，0为不限幅

typedef struct PID{float P,Pout,I,Iout,D,Dout,I_Max,Out;}PID;
typedef struct
{
	float X;
	float Y;
	float Z;
}S_FLOAT_XYZ;

typedef struct
{
	float error;        //< error
	float prevError;    //< previous error
	float integ;        //< integral
	float deriv;        //< derivative
	float kp;           //< proportional gain
	float ki;           //< integral gain
	float kd;           //< derivative gain
	float outP;         //< proportional output (debugging)
	float outI;         //< integral output (debugging)
	float outD;         //< derivative output (debugging)
	float iLimit;       //< integral limit
	float outputLimit;  //< total PID output limit, absolute value. '0' means no limit.
} PidObject;

typedef struct  
{
	float roll;
	float pitch;
	float yaw;
} attitude_t;

typedef struct
{
	float roll;
	float pitch;
	float yaw;
} control_t;


void attitudeControlInit(void);

void pidInit(PidObject* pid, const pidInit_t pidParam);
void pidSetIntegralLimit(PidObject* pid, const float limit) ;
float pidUpdate(PidObject* pid, const float error,float dt);


void Control(void);
void PID_Init(void);

extern volatile uint16_t motor[5];
extern volatile S_FLOAT_XYZ Exp_Angle;		

#endif
