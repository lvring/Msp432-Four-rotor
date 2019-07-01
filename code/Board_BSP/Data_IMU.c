/*
//��̬����
//SJ 2018.1.18
*/ 
#include "DATA_IMU.h"
#include "Data_Sensor.h"


volatile IMU_Data_t IMU_Data;
  

/******************************************************************************
*�������ƣ�invSqrt
*�������ܣ����ټ��� �����ĵ��� Ϊ��λ������
*���������Ҫ�����ֵ
*������������
*******************************************************************************/
float invSqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;						// evil floating point bit level hacking
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));// 1st iteration
//	y = y * (1.5f - (halfx * y * y));// 2nd iteration, this can be removed
	return y;
}
/******************************************************************************
*�������ƣ�IMU_init
*�������ܣ���ʼ��IMU��ز���
			    ��ʼ����Ԫ��
			    ����������
			    ����ϵͳʱ��
�����������
�����������
*******************************************************************************/
void IMU_init(volatile IMU_Data_t *thisIMUData)
{	 
	thisIMUData->q0 = 1.0f;  //��ʼ����Ԫ��
	thisIMUData->q1 = 0.0f;
	thisIMUData->q2 = 0.0f;
	thisIMUData->q3 = 0.0f;
	thisIMUData->exInt = 0.0f;
	thisIMUData->eyInt = 0.0f;
	thisIMUData->ezInt = 0.0f;
	//������ʼ��׼ʱ��ʱ�� Ϊ������Ԫ������������׼��
	thisIMUData->LastTick = RTE_RoundRobin_GetTick()*1000;
	thisIMUData->NowTick = RTE_RoundRobin_GetTick()*1000;
}





#define twoKpDef	(2.0f * 2.f)	// 2 * proportional gain
#define twoKiDef	(2.0f * 0.0f)	// 2 * integral gain
volatile float twoKp = twoKpDef;											// 2 * proportional gain (Kp)
volatile float twoKi = twoKiDef;											// 2 * integral gain (Ki)


void AHRSupdate(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
	float norm;
	float hx, hy, hz, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float halfT;
	float tempq0,tempq1,tempq2,tempq3;
	
	thisIMUData->NowTick  = RTE_RoundRobin_GetTick()*1000;                              //���뺯����ȡ��ǰʱ��
  if(thisIMUData->NowTick<thisIMUData->LastTick)                                               //��ʱ�����
		{ 
		  halfT =  ((float)(thisIMUData->NowTick + (0xffff- thisIMUData->LastTick)) / 2000000.0f); //�õ�����ʱ���ټ���ʱ���������ڣ���һ��
		}
  else	                                                           //��ʱ��δ���
		{
		  halfT =  ((float)(thisIMUData->NowTick - thisIMUData->LastTick) / 2000000.0f);           //����ʱ���������ڣ���һ��
		}
	//printf("halfT%f\r\n",halfT);
  thisIMUData->LastTick = thisIMUData->NowTick;	//����ʱ��
	
	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MahonyAHRSupdateIMU(thisIMUData,gx, gy, gz, ax, ay, az,halfT);
		return;
	}
	
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
	{
		
		// normalise the measurements
		norm = invSqrt(ax*ax + ay*ay + az*az); 
		ax = ax * norm;
		ay = ay * norm;
		az = az * norm;
		norm = invSqrt(mx*mx + my*my + mz*mz); 
		mx = mx * norm;
		my = my * norm;
		mz = mz * norm;
		
					// auxiliary variables to reduce number of repeated operations
		//�����м�������׼�� 
		float q0q0 = thisIMUData->q0*thisIMUData->q0;
		float q0q1 = thisIMUData->q0*thisIMUData->q1;
		float q0q2 = thisIMUData->q0*thisIMUData->q2;
		float q0q3 = thisIMUData->q0*thisIMUData->q3;
		float q1q1 = thisIMUData->q1*thisIMUData->q1;
		float q1q2 = thisIMUData->q1*thisIMUData->q2;
		float q1q3 = thisIMUData->q1*thisIMUData->q3;
		float q2q2 = thisIMUData->q2*thisIMUData->q2;   
		float q2q3 = thisIMUData->q2*thisIMUData->q3;
		float q3q3 = thisIMUData->q3*thisIMUData->q3; 
		
			
	/*�ӻ�������ϵ�ĵ������̲⵽��ʸ��ת�ɵ�������ϵ�µĴų�ʸ��hxyz������ֵ������������Ǵӷ���������ϵ����������ϵ��ת����ʽ*/
		 // compute reference direction of magnetic field
		hx = 2*mx*(0.5f - q2q2 - q3q3) + 2*my*(q1q2 - q0q3) + 2*mz*(q1q3 + q0q2);
		hy = 2*mx*(q1q2 + q0q3) + 2*my*(0.5f - q1q1 - q3q3) + 2*mz*(q2q3 - q0q1);
		hz = 2*mx*(q1q3 - q0q2) + 2*my*(q2q3 + q0q1) + 2*mz*(0.5f - q1q1 - q2q2);
	/*�����������ϵ�µĴų�ʸ��bxyz���ο�ֵ����
		��Ϊ����ش�ˮƽ�нǣ�������֪��0�ȣ���ȥ��ƫ�ǵ����أ��̶��򱱣����Ҷ���byָ������������by=ĳֵ��bx=0
		������ο��ش�ʸ���ڴ�ֱ����Ҳ�з���bz��������ÿ���ط����ǲ�һ���ġ�
		�����޷���֪��Ҳ���޷������ںϣ��и��ʺ�����ֱ���������ںϵļ��ٶȼƣ�������ֱ�ӴӲ���ֵhz�ϸ��ƹ�����bz=hz��
		�ų�ˮƽ�������ο�ֵ�Ͳ���ֵ�Ĵ�СӦ����һ�µ�(bx*bx) + (by*by)) = ((hx*hx) + (hy*hy))��
		��Ϊbx=0�����Ծͼ򻯳�(by*by)  = ((hx*hx) + (hy*hy))�������by�������޸�by��bxָ����Զ����ĸ���ָ������*/	
		bx = sqrt((hx*hx) + (hy*hy));
		bz = hz; 
		
		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
		
		halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
		halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
		halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);  
		
		// Error is sum of cross product between estimated direction and measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);
		
		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			thisIMUData->exInt += twoKi * halfex * halfT*2.f;	// integral error scaled by Ki
			thisIMUData->eyInt += twoKi * halfey * halfT*2.f;
			thisIMUData->ezInt += twoKi * halfez * halfT*2.f;
			gx += thisIMUData->exInt;	// apply integral feedback
			gy += thisIMUData->eyInt;
			gz += thisIMUData->ezInt;
		}
		else {
			thisIMUData->exInt = 0.0f;	// prevent integral windup
			thisIMUData->eyInt = 0.0f;
			thisIMUData->ezInt = 0.0f;
		}
		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
		
	} 
		// integrate quaternion rate and normalize
		//һ�����������������Ԫ��
		tempq0 = thisIMUData->q0 + (-thisIMUData->q1*gx - thisIMUData->q2*gy - thisIMUData->q3*gz)*halfT;
		tempq1 = thisIMUData->q1 + (thisIMUData->q0*gx + thisIMUData->q2*gz - thisIMUData->q3*gy)*halfT;
		tempq2 = thisIMUData->q2 + (thisIMUData->q0*gy - thisIMUData->q1*gz + thisIMUData->q3*gx)*halfT;
		tempq3 = thisIMUData->q3 + (thisIMUData->q0*gz + thisIMUData->q1*gy - thisIMUData->q2*gx)*halfT;  
	 
		// normalise quaternion
		norm = invSqrt(tempq0*tempq0 + tempq1*tempq1 + tempq2*tempq2 + tempq3*tempq3);
		thisIMUData->q0 = tempq0 * norm;
		thisIMUData->q1 = tempq1 * norm;
		thisIMUData->q2 = tempq2 * norm;
		thisIMUData->q3 = tempq3 * norm;

} 

void MahonyAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT) {
	float recipNorm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float tempq0,tempq1,tempq2,tempq3;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;        

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = thisIMUData->q1 * thisIMUData->q3 - thisIMUData->q0 * thisIMUData->q2;
		halfvy = thisIMUData->q0 * thisIMUData->q1 + thisIMUData->q2 * thisIMUData->q3;
		halfvz = thisIMUData->q0 * thisIMUData->q0 - 0.5f + thisIMUData->q3 * thisIMUData->q3;
	
		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			thisIMUData->exInt += twoKi * halfex * halfT*2.f;	// integral error scaled by Ki
			thisIMUData->eyInt += twoKi * halfey * halfT*2.f;
			thisIMUData->ezInt += twoKi * halfez * halfT*2.f;
			gx += thisIMUData->exInt;	// apply integral feedback
			gy += thisIMUData->eyInt;
			gz += thisIMUData->ezInt;
		}
		else {
			thisIMUData->exInt = 0.0f;	// prevent integral windup
			thisIMUData->eyInt = 0.0f;
			thisIMUData->ezInt = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}
	
	
	// ��Ԫ��΢�ַ��� ���½��������Ԫ��
  tempq0 = thisIMUData->q0 + (-thisIMUData->q1*gx - thisIMUData->q2*gy - thisIMUData->q3*gz)*halfT;
  tempq1 = thisIMUData->q1 + (thisIMUData->q0*gx + thisIMUData->q2*gz - thisIMUData->q3*gy)*halfT;
  tempq2 = thisIMUData->q2 + (thisIMUData->q0*gy - thisIMUData->q1*gz + thisIMUData->q3*gx)*halfT;
  tempq3 = thisIMUData->q3 + (thisIMUData->q0*gz + thisIMUData->q1*gy - thisIMUData->q2*gx)*halfT;  
		
	// Normalise quaternion
  recipNorm = invSqrt(tempq0*tempq0 + tempq1*tempq1 + tempq2*tempq2 + tempq3*tempq3);
  thisIMUData->q0 = tempq0 * recipNorm;
  thisIMUData->q1 = tempq1 * recipNorm;
  thisIMUData->q2 = tempq2 * recipNorm;
  thisIMUData->q3 = tempq3 * recipNorm;
}


#define betaDef		0.50f		// 2 * proportional gain
volatile float beta = betaDef;								// 2 * proportional gain (Kp)


void MadgwickAHRSupdate(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float q0=thisIMUData->q0,q1=thisIMUData->q1,q2=thisIMUData->q2,q3=thisIMUData->q3;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float halfT;
	
	
	thisIMUData->NowTick  = RTE_RoundRobin_GetTick()*1000;                              //���뺯����ȡ��ǰʱ��
  if(thisIMUData->NowTick<thisIMUData->LastTick)                                               //��ʱ�����
		{ 
		  halfT =  ((float)(thisIMUData->NowTick + (0xffff- thisIMUData->LastTick)) / 2000000.0f); //�õ�����ʱ���ټ���ʱ���������ڣ���һ��
		}
  else	                                                           //��ʱ��δ���
		{
		  halfT =  ((float)(thisIMUData->NowTick - thisIMUData->LastTick) / 2000000.0f);           //����ʱ���������ڣ���һ��
		}
  thisIMUData->LastTick = thisIMUData->NowTick;	//����ʱ��
	//printf("halfT %f\r\n",halfT);
	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MadgwickAHRSupdateIMU(thisIMUData,gx, gy, gz, ax, ay, az , halfT);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * q0 * mx;
		_2q0my = 2.0f * q0 * my;
		_2q0mz = 2.0f * q0 * mz;
		_2q1mx = 2.0f * q1 * mx;
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_2q0q2 = 2.0f * q0 * q2;
		_2q2q3 = 2.0f * q2 * q3;
		q0q0 = q0 * q0;
		q0q1 = q0 * q1;
		q0q2 = q0 * q2;
		q0q3 = q0 * q3;
		q1q1 = q1 * q1;
		q1q2 = q1 * q2;
		q1q3 = q1 * q3;
		q2q2 = q2 * q2;
		q2q3 = q2 * q3;
		q3q3 = q3 * q3;

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
		hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
		_2bx = sqrt(hx * hx + hy * hy);
		_2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * halfT * 2;
	q1 += qDot2 * halfT * 2;
	q2 += qDot3 * halfT * 2;
	q3 += qDot4 * halfT * 2;

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
	
	thisIMUData->q0 = q0;
	thisIMUData->q1 = q1;
	thisIMUData->q2 = q2;
	thisIMUData->q3 = q3;
	
	
}

//---------------------------------------------------------------------------------------------------
// IMU algorithm update

void MadgwickAHRSupdateIMU(volatile IMU_Data_t *thisIMUData,float gx, float gy, float gz, float ax, float ay, float az,float halfT) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;
	float q0=thisIMUData->q0,q1=thisIMUData->q1,q2=thisIMUData->q2,q3=thisIMUData->q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_4q0 = 4.0f * q0;
		_4q1 = 4.0f * q1;
		_4q2 = 4.0f * q2;
		_8q1 = 8.0f * q1;
		_8q2 = 8.0f * q2;
		q0q0 = q0 * q0;
		q1q1 = q1 * q1;
		q2q2 = q2 * q2;
		q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * halfT * 2;
	q1 += qDot2 * halfT * 2;
	q2 += qDot3 * halfT * 2;
	q3 += qDot4 * halfT * 2;
	
	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
	
	thisIMUData->q0 = q0;
	thisIMUData->q1 = q1;
	thisIMUData->q2 = q2;
	thisIMUData->q3 = q3;
}





/******************************************************************************
*�������ƣ�IMU_getQ
*�������ܣ���ȡ��ǰ�˲���������������ٶȺͽ��ٶȵ�λת����6500��ֵ ��ɼ��ٶȵ�λ�����һ����Ϊ�����õ���ǰ��Ԫ�� 
*�����������Ҫ�����Ԫ���������׵�ַ
*���������û��
*******************************************************************************/
#define SENSOR_MAX_G 8.0f		//constant g		// tobe fixed to 8g. but IMU need to  correct at the same time
#define SENSOR_MAX_W 2000.0f	//deg/s
#define ACC_SCALE  (SENSOR_MAX_G/32768.0f)
#define GYRO_SCALE  (SENSOR_MAX_W/32768.0f)
#define M_PI_F 3.1415926f
#define CONSTANTS_ONE_G					9.80665f		/* m/s^2		*/
void IMU_getAngle(volatile IMU_Data_t *thisIMUData) 
{
	//AHRSupdate(thisIMUData,gyroRaw.x, gyroRaw.y, gyroRaw.z,  accRaw.x, accRaw.y, accRaw.z, 0,0,0);
	//AHRSupdate(thisIMUData,gyroRaw.x, gyroRaw.y, gyroRaw.z,  accRaw.x, accRaw.y, accRaw.z, magRaw.x,magRaw.y,magRaw.z);
	//MadgwickAHRSupdate(thisIMUData,gyroRaw.x, gyroRaw.y, gyroRaw.z,  accRaw.x, accRaw.y, accRaw.z, magRaw.x,magRaw.y,magRaw.z);
	MadgwickAHRSupdate(thisIMUData,gyroRaw.x* M_PI_F /180.f, gyroRaw.y* M_PI_F /180.f, gyroRaw.z* M_PI_F /180.f,  accRaw.x, accRaw.y, accRaw.z, 0,0,0);
	thisIMUData->Result[0] = -atan2(2 * thisIMUData->q1 * thisIMUData->q2 + 2 * thisIMUData->q0 * thisIMUData->q3, -2 * thisIMUData->q2*thisIMUData->q2 - 2 * thisIMUData->q3 * thisIMUData->q3 + 1)* 180/M_PI; //ƫ��yaw                                                         //����ԣ�� 
	thisIMUData->Result[1] = -asin(-2 * thisIMUData->q1 * thisIMUData->q3 + 2 * thisIMUData->q0 *thisIMUData->q2)* 180/M_PI;   // ���roll
  thisIMUData->Result[2] = atan2(2 * thisIMUData->q2 * thisIMUData->q3 + 2 * thisIMUData->q0 * thisIMUData->q1, -2 * thisIMUData->q1 * thisIMUData->q1 - 2 * thisIMUData->q2 * thisIMUData->q2 + 1)* 180/M_PI; // ���� pitch
}




//------------------End of File----------------------------
