#ifndef  _TELECONTROL_H_
#define  _TELECONTROL_H_


#include <driverlib.h>

typedef struct
{
	uint8_t rc_channel_5__status;	  //遥控通道五状态，为0拨到下面，为1拨到上面 右上方
  uint8_t rc_channel_6__status;	  //遥控通道六状态，为0拨到下面，为1拨到上面 左上方
}TeleControl_Data_t;          //1不占内存


uint8_t Is_WFLY_Connected(void);
void WFLY_PWM_Convert(void);


extern volatile TeleControl_Data_t TeleControl_Data;
#endif


