/*
//ң�����ź�ת�� ���ջ����յ�ң�����ź� ת��ΪPWM������ɿ� 
//�ɿظ��ݶԽ��ջ������벶�񲢷�������ֵ������ң����ָ��
//SJ 2018.1.18
VCC5V
//////////////PA3
//////////////PA2
//////////////PA1
//////////////PA0 VCC GND
//////////////PB4
//////////////PB5
*/
#include "telecontrol.h"
#include "control.h"
#include "pwm_in.h"
#include "BSP_Led.h"

extern volatile int16_t throttle;           //����  ��control�ж��� ��controlʹ��

volatile TeleControl_Data_t TeleControl_Data;  //3����ṹ��ʵ�� ����ֱ����

//float angle_z;//ƫ����
/*
 * ��������WFLY_PWM_Convert
 * ����  ��ң�����ź�ת��
 * ����  ����
 * ���  ����
 */ 
void WFLY_PWM_Convert(void)
{
	static uint16_t cnt;    //ƫ����ת��Ƶ��
	cnt++;
	Exp_Angle.X = (Rc_Channel_Val[2] - 1487) * 0.15f; 
	if((Exp_Angle.X > -4) && (Exp_Angle.X < 4))	Exp_Angle.X = 0;
	Exp_Angle.Y = (Rc_Channel_Val[0] - 1487) * 0.15f;              //0.05f 25��   0.15f 75��
	if((Exp_Angle.Y > -4) && (Exp_Angle.Y < 4))	Exp_Angle.Y = 0;
	
	throttle	=	(volatile int16_t)(Rc_Channel_Val[1] - 1007) * 3;             //��ң�����ź�ת���õ�������
  //printf("Exp_Angle.pitch%f\r\n",Exp_Angle.X);		            //���� pitch
	//printf("Exp_Angle.roll%f\r\n",Exp_Angle.Y);	              	//��� roll
	if(cnt == 50)                                                 //ƫ����ת��Ƶ��
	{
		cnt = 0;	
		if((Rc_Channel_Val[3] > 1600) || (Rc_Channel_Val[3] < 1300))              //��ֹ�����ŵ���ƫ��������ۼ�
		{
      if(throttle > 400)		
			{
				if(fabs((Rc_Channel_Val[3]- 1487) * 0.006f)>0.3f)
				{
					Exp_Angle.Z += (Rc_Channel_Val[3]- 1480) * 0.006f;
				}
				
			}				//throttle>400������ű��� �ۼӲ����� �ɻ������Ѿ��ı�yawֵ���治�� ��ֵ����ң����ͨ������
		}
		if(Exp_Angle.Z > 180)
		{
			Exp_Angle.Z += -360;    //180��360 ת��Ϊ-180��0	
		}
		else if(Exp_Angle.Z < -180)
		{
			Exp_Angle.Z += 360;     //-180��-360 ת��Ϊ180��0;	
		}	

  //printf("Exp_Angle.Z%f\r\n",Exp_Angle.Z);		//ƫ�� ң�������һֱ��-180��180
	}
	if(Rc_Channel_Val[4] < 1800) 
	{
		TeleControl_Data.rc_channel_5__status = 1;//ң��ͨ����״̬��Ϊ0�������棬Ϊ1��������
//		if(Rc_Channel_Val[5] < 1200)
//			TeleControl_Data.rc_channel_5__status = 1;
//		else
//			TeleControl_Data.rc_channel_5__status = 0;
	}
	else 
	{
		TeleControl_Data.rc_channel_5__status = 0;
	}
}

/*
 * ��������Is_WFLY_Connected
 * ����  ���ж�ң�����Ƿ��������� ͨ����ȡң�����Ͽ�ʱ�����벶�������С��ֵ����
 * ����  ����
 * ���  ��0������ʧ�ܣ�1����������
 */  
uint8_t Is_WFLY_Connected(void)
{

	if(Rc_Channel_Val[2] < 950) 
	{	
		BSP_LED_Off(LED_B);//��ɫ��Ϩ����ʾң�������ӶϿ�
		return 0;
	}
	else 
	{
		BSP_LED_On(LED_B);//��ɫ��������ʾң����������
		return 1;
	}
}
      



