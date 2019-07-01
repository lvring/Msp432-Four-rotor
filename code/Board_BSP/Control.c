/****************************
由遥控器给出的期望角度和6500测量的角度 通过关系得出四路pwm控制四个电机
SJ 2018.1.18
							Y(Roll)机头
			  顺时针转 | 逆时针转
			B7一号电机 | 四号电机B6
       -------------------- X(Pitch)         
				逆时针转 | 顺时针转 
		  B8二号电机 | 三号电机B9
****************************/
#include "control.h"
#include "Data_Sensor.h"  
#include "pwm_out.h"       
#include "BSP_Led.h"
#include "telecontrol.h"
#include "DATA_IMU.h"

volatile uint16_t motor[5];																		//定义电机PWM
volatile int16_t throttle = 0;																				//油门值
volatile uint8_t fly_state = 0;																				//定义飞行状态;
volatile PID PID_Roll_Angle,PID_Pitch_Angle,PID_Yaw_Angle;//外环：角度PID环
volatile PID PID_Roll_Rate,PID_Pitch_Rate,PID_Yaw_Rate;		//内环：角速度PID环
volatile PID PID_Height;																	//高度
volatile S_FLOAT_XYZ Exp_Angle;														//期望角度 在control中定义 实际数据来自遥控器
volatile S_FLOAT_XYZ MPU6500_Gyro_Last;										//上一状态记录的陀螺仪数据
volatile uint8_t exp_height_flag = 1;
volatile float exp_height;                                  //期望高度
         
									
void Inner_Loop_Control(void);
void Outter_Loop_Control(void);
void Height_Control(void);
uint16_t limitThrust(float value);


PidObject pidAngleRoll;
PidObject pidAnglePitch;
PidObject pidAngleYaw;
PidObject pidRateRoll;
PidObject pidRatePitch;
PidObject pidRateYaw;


void attitudeControlInit(void)
{
	pidInit(&pidAngleRoll,  configParam.pidAngle.roll);			/*roll  角度PID初始化*/
	pidInit(&pidAnglePitch, configParam.pidAngle.pitch);			/*pitch 角度PID初始化*/
	pidInit(&pidAngleYaw,   configParam.pidAngle.yaw);				/*yaw   角度PID初始化*/
	pidSetIntegralLimit(&pidAngleRoll, PID_ANGLE_ROLL_INTEGRATION_LIMIT);		/*roll  角度积分限幅设置*/
	pidSetIntegralLimit(&pidAnglePitch, PID_ANGLE_PITCH_INTEGRATION_LIMIT);		/*pitch 角度积分限幅设置*/
	pidSetIntegralLimit(&pidAngleYaw, PID_ANGLE_YAW_INTEGRATION_LIMIT);			/*yaw   角度积分限幅设置*/
	
	pidInit(&pidRateRoll,  configParam.pidRate.roll);				/*roll  角速度PID初始化*/
	pidInit(&pidRatePitch, configParam.pidRate.pitch);			/*pitch 角速度PID初始化*/
	pidInit(&pidRateYaw,   configParam.pidRate.yaw);				/*yaw   角速度PID初始化*/
	pidSetIntegralLimit(&pidRateRoll,  PID_RATE_ROLL_INTEGRATION_LIMIT);			/*roll  角速度积分限幅设置*/
	pidSetIntegralLimit(&pidRatePitch, PID_RATE_PITCH_INTEGRATION_LIMIT);		/*pitch 角速度积分限幅设置*/
	pidSetIntegralLimit(&pidRateYaw,   PID_RATE_YAW_INTEGRATION_LIMIT);			/*yaw   角速度积分限幅设置*/
	
}
void attitudeAnglePID(attitude_t *outDesiredRate,float dt)
{
//	Exp_Angle.X=0;
//	Exp_Angle.Y=0;
//	Exp_Angle.Z=0;
	float yawError = Exp_Angle.Z -  IMU_Data.Result[0] ;//Yaw
	float pitchError = Exp_Angle.X + IMU_Data.Result[2] ;//P
	float rollError =  Exp_Angle.Y - IMU_Data.Result[1] ;//Roll
	outDesiredRate->roll  = pidUpdate(&pidAngleRoll,rollError,dt);//roll
	outDesiredRate->pitch = pidUpdate(&pidAnglePitch,pitchError,dt);//pitch

	
	if (yawError > 180.0f) 
		yawError -= 360.0f;
	else if (yawError < -180.0) 
		yawError += 360.0f;
	outDesiredRate->yaw = pidUpdate(&pidAngleYaw, yawError,dt);

}
void attitudeRatePID(Axis3f *actualRate,attitude_t *desiredRate,control_t *output,float dt)	/* 角速度环PID */
{
	output->roll = pidUpdate(&pidRateRoll, desiredRate->roll + actualRate->y,dt);//0.5f缩放因子  
	output->pitch = pidUpdate(&pidRatePitch, desiredRate->pitch + actualRate->x,dt);
	output->yaw = pidUpdate(&pidRateYaw, desiredRate->yaw + actualRate->z,dt);
}






/*
 * 函数名：Control
 * 描述  ：根据姿态信息控制四旋翼
 * 输入  ：Now_Angle：当前姿态；Exp_Angle：期望姿态
 * 输出  ：无
 */ 
attitude_t rateDesired;
control_t 	control;	/*四轴控制参数*/
uint32_t LastTick,NowTick;



void Control(void)
{		
	float dt;
	NowTick  = RTE_RoundRobin_GetTick();                              //进入函数读取当前时间
  if(NowTick<LastTick)                                               //定时器溢出
		{ 
		  dt =  ((float)(NowTick + (0xffff- LastTick))); 
		}
  else	                                                           //定时器未溢出
		{
		  dt =  ((float)(NowTick -LastTick));          
		}
	LastTick = NowTick;
		
//	//双环控制
	attitudeAnglePID(&rateDesired,dt/1000);
	attitudeRatePID(&gyroRaw,&rateDesired,&control,dt/1000);
		
//	float  cosTilt = accRaw.z/ 9.80665f;
//	Outter_Loop_Control();//外环
//	Inner_Loop_Control();//内环

	if(TeleControl_Data.rc_channel_5__status&&Is_WFLY_Connected())
	{
		motor[1] = limitThrust(ADD_THROTTLE + throttle*0.7 - control.roll - control.pitch - control.yaw);
//		motor[1] = ADD_THROTTLE + throttle*0.7 - PID_Roll_Rate.Out - PID_Pitch_Rate.Out- PID_Yaw_Rate.Out+ PID_Height.Out  ;//
		motor[2] = limitThrust(ADD_THROTTLE + throttle*0.7 - control.roll + control.pitch + control.yaw);
//	  motor[2] = ADD_THROTTLE + throttle*0.7  - PID_Roll_Rate.Out + PID_Pitch_Rate.Out+ PID_Yaw_Rate.Out+ PID_Height.Out   ;//+ throttle

		motor[3] = limitThrust(ADD_THROTTLE + throttle*0.7 + control.roll + control.pitch - control.yaw);
//		motor[3] = ADD_THROTTLE + throttle*0.7 + PID_Roll_Rate.Out + PID_Pitch_Rate.Out- PID_Yaw_Rate.Out+ PID_Height.Out  ;//
		motor[4] = limitThrust(ADD_THROTTLE + throttle*0.7 + control.roll - control.pitch + control.yaw);
		//motor[4] = ADD_THROTTLE + throttle*0.7 + PID_Roll_Rate.Out - PID_Pitch_Rate.Out+ PID_Yaw_Rate.Out+ PID_Height.Out  ;//

	}
	else
	{
		BSP_LED_Off(LED_B);
		motor[1]  = 0;                  //安全考虑 关闭所有电机
		motor[2]  = 0;
		motor[3]  = 0;
		motor[4]  = 0;			
		Exp_Angle.Z = - IMU_Data.Result[0];   
	}
	Motor_PWM_Update(motor[1] ,motor[2] ,motor[3] ,motor[4]);
	
}



void pidInit(PidObject* pid, const pidInit_t pidParam)
{
	pid->error     = 0;
	pid->prevError = 0;
	pid->integ     = 0;
	pid->deriv     = 0;
	pid->kp = pidParam.kp;
	pid->ki = pidParam.ki;
	pid->kd = pidParam.kd;
	pid->iLimit = DEFAULT_PID_INTEGRATION_LIMIT;
	pid->outputLimit = DEFAULT_PID_OUTPUT_LIMIT;
}

void pidSetIntegralLimit(PidObject* pid, const float limit) 
{
    pid->iLimit = limit;
}


float pidUpdate(PidObject* pid, const float error,float dt)
{
	float output;

	pid->error = error;   

	pid->integ += pid->error * dt;
	
	//积分限幅
	if (pid->integ > pid->iLimit)
	{
		pid->integ = pid->iLimit;
	}
	else if (pid->integ < -pid->iLimit)
	{
		pid->integ = -pid->iLimit;
	}

	pid->deriv = (pid->error - pid->prevError) / dt;

	pid->outP = pid->kp * pid->error;
	pid->outI = pid->ki * pid->integ;
	pid->outD = pid->kd * pid->deriv;

	output = pid->outP + pid->outI + pid->outD;
	
	//输出限幅
	if (pid->outputLimit != 0)
	{
		if (output > pid->outputLimit)
			output = pid->outputLimit;
		else if (output < -pid->outputLimit)
			output = -pid->outputLimit;
	}
	
	pid->prevError = pid->error;

	return output;
}


uint16_t limitThrust(float value)
{
	if(value > 3500)
	{
		value = 3500;
	}
	else if(value < 2000)
	{
		value = 2000;
	}

	return (uint16_t)value;
}
