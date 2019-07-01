#ifndef  _TELECONTROL_H_
#define  _TELECONTROL_H_


#include <driverlib.h>

typedef struct
{
	uint8_t rc_channel_5__status;	  //ң��ͨ����״̬��Ϊ0�������棬Ϊ1�������� ���Ϸ�
  uint8_t rc_channel_6__status;	  //ң��ͨ����״̬��Ϊ0�������棬Ϊ1�������� ���Ϸ�
}TeleControl_Data_t;          //1��ռ�ڴ�


uint8_t Is_WFLY_Connected(void);
void WFLY_PWM_Convert(void);


extern volatile TeleControl_Data_t TeleControl_Data;
#endif


