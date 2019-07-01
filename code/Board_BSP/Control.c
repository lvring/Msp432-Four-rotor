/****************************
��ң���������������ǶȺ�6500�����ĽǶ� ͨ����ϵ�ó���·pwm�����ĸ����
SJ 2018.1.18
							Y(Roll)��ͷ
			  ˳ʱ��ת | ��ʱ��ת
			B7һ�ŵ�� | �ĺŵ��B6
       -------------------- X(Pitch)         
				��ʱ��ת | ˳ʱ��ת 
		  B8���ŵ�� | ���ŵ��B9
****************************/
#include "control.h"
#include "Data_Sensor.h"  
#include "pwm_out.h"       
#include "BSP_Led.h"
#include "telecontrol.h"
#include "DATA_IMU.h"

volatile uint16_t motor[5];																		//������PWM
volatile int16_t throttle = 0;																				//����ֵ
volatile uint8_t fly_state = 0;																				//�������״̬;
volatile PID PID_Roll_Angle,PID_Pitch_Angle,PID_Yaw_Angle;//�⻷���Ƕ�PID��
volatile PID PID_Roll_Rate,PID_Pitch_Rate,PID_Yaw_Rate;		//�ڻ������ٶ�PID��
volatile PID PID_Height;																	//�߶�
volatile S_FLOAT_XYZ Exp_Angle;														//�����Ƕ� ��control�ж��� ʵ����������ң����
volatile S_FLOAT_XYZ MPU6500_Gyro_Last;										//��һ״̬��¼������������
volatile uint8_t exp_height_flag = 1;
volatile float exp_height;                                  //�����߶�
         
									
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
	pidInit(&pidAngleRoll,  configParam.pidAngle.roll);			/*roll  �Ƕ�PID��ʼ��*/
	pidInit(&pidAnglePitch, configParam.pidAngle.pitch);			/*pitch �Ƕ�PID��ʼ��*/
	pidInit(&pidAngleYaw,   configParam.pidAngle.yaw);				/*yaw   �Ƕ�PID��ʼ��*/
	pidSetIntegralLimit(&pidAngleRoll, PID_ANGLE_ROLL_INTEGRATION_LIMIT);		/*roll  �ǶȻ����޷�����*/
	pidSetIntegralLimit(&pidAnglePitch, PID_ANGLE_PITCH_INTEGRATION_LIMIT);		/*pitch �ǶȻ����޷�����*/
	pidSetIntegralLimit(&pidAngleYaw, PID_ANGLE_YAW_INTEGRATION_LIMIT);			/*yaw   �ǶȻ����޷�����*/
	
	pidInit(&pidRateRoll,  configParam.pidRate.roll);				/*roll  ���ٶ�PID��ʼ��*/
	pidInit(&pidRatePitch, configParam.pidRate.pitch);			/*pitch ���ٶ�PID��ʼ��*/
	pidInit(&pidRateYaw,   configParam.pidRate.yaw);				/*yaw   ���ٶ�PID��ʼ��*/
	pidSetIntegralLimit(&pidRateRoll,  PID_RATE_ROLL_INTEGRATION_LIMIT);			/*roll  ���ٶȻ����޷�����*/
	pidSetIntegralLimit(&pidRatePitch, PID_RATE_PITCH_INTEGRATION_LIMIT);		/*pitch ���ٶȻ����޷�����*/
	pidSetIntegralLimit(&pidRateYaw,   PID_RATE_YAW_INTEGRATION_LIMIT);			/*yaw   ���ٶȻ����޷�����*/
	
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
void attitudeRatePID(Axis3f *actualRate,attitude_t *desiredRate,control_t *output,float dt)	/* ���ٶȻ�PID */
{
	output->roll = pidUpdate(&pidRateRoll, desiredRate->roll + actualRate->y,dt);//0.5f��������  
	output->pitch = pidUpdate(&pidRatePitch, desiredRate->pitch + actualRate->x,dt);
	output->yaw = pidUpdate(&pidRateYaw, desiredRate->yaw + actualRate->z,dt);
}






/*
 * ��������Control
 * ����  ��������̬��Ϣ����������
 * ����  ��Now_Angle����ǰ��̬��Exp_Angle��������̬
 * ���  ����
 */ 
attitude_t rateDesired;
control_t 	control;	/*������Ʋ���*/
uint32_t LastTick,NowTick;



void Control(void)
{		
	float dt;
	NowTick  = RTE_RoundRobin_GetTick();                              //���뺯����ȡ��ǰʱ��
  if(NowTick<LastTick)                                               //��ʱ�����
		{ 
		  dt =  ((float)(NowTick + (0xffff- LastTick))); 
		}
  else	                                                           //��ʱ��δ���
		{
		  dt =  ((float)(NowTick -LastTick));          
		}
	LastTick = NowTick;
		
//	//˫������
	attitudeAnglePID(&rateDesired,dt/1000);
	attitudeRatePID(&gyroRaw,&rateDesired,&control,dt/1000);
		
//	float  cosTilt = accRaw.z/ 9.80665f;
//	Outter_Loop_Control();//�⻷
//	Inner_Loop_Control();//�ڻ�

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
		motor[1]  = 0;                  //��ȫ���� �ر����е��
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
	
	//�����޷�
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
	
	//����޷�
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
