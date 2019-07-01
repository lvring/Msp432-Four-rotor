/*
//遥控器信号转换 接收机接收到遥控器信号 转化为PWM输出给飞控 
//飞控根据对接收机的输入捕获并分析捕获值来解析遥控器指令
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

extern volatile int16_t throttle;           //油门  在control中定义 给control使用

volatile TeleControl_Data_t TeleControl_Data;  //3申请结构体实体 下面直接用

//float angle_z;//偏航角
/*
 * 函数名：WFLY_PWM_Convert
 * 描述  ：遥控器信号转换
 * 输入  ：无
 * 输出  ：无
 */ 
void WFLY_PWM_Convert(void)
{
	static uint16_t cnt;    //偏航角转换频率
	cnt++;
	Exp_Angle.X = (Rc_Channel_Val[2] - 1487) * 0.15f; 
	if((Exp_Angle.X > -4) && (Exp_Angle.X < 4))	Exp_Angle.X = 0;
	Exp_Angle.Y = (Rc_Channel_Val[0] - 1487) * 0.15f;              //0.05f 25度   0.15f 75度
	if((Exp_Angle.Y > -4) && (Exp_Angle.Y < 4))	Exp_Angle.Y = 0;
	
	throttle	=	(volatile int16_t)(Rc_Channel_Val[1] - 1007) * 3;             //由遥控器信号转换得到的油门
  //printf("Exp_Angle.pitch%f\r\n",Exp_Angle.X);		            //俯仰 pitch
	//printf("Exp_Angle.roll%f\r\n",Exp_Angle.Y);	              	//横滚 roll
	if(cnt == 50)                                                 //偏航角转换频率
	{
		cnt = 0;	
		if((Rc_Channel_Val[3] > 1600) || (Rc_Channel_Val[3] < 1300))              //防止推油门导致偏航角误差累计
		{
      if(throttle > 400)		
			{
				if(fabs((Rc_Channel_Val[3]- 1487) * 0.006f)>0.3f)
				{
					Exp_Angle.Z += (Rc_Channel_Val[3]- 1480) * 0.006f;
				}
				
			}				//throttle>400最低油门保护 累加不归零 飞机航向已经改变yaw值保存不变 负值由于遥控器通道方向
		}
		if(Exp_Angle.Z > 180)
		{
			Exp_Angle.Z += -360;    //180到360 转化为-180到0	
		}
		else if(Exp_Angle.Z < -180)
		{
			Exp_Angle.Z += 360;     //-180到-360 转化为180到0;	
		}	

  //printf("Exp_Angle.Z%f\r\n",Exp_Angle.Z);		//偏航 遥控器输出一直在-180到180
	}
	if(Rc_Channel_Val[4] < 1800) 
	{
		TeleControl_Data.rc_channel_5__status = 1;//遥控通道五状态，为0拨到下面，为1拨到上面
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
 * 函数名：Is_WFLY_Connected
 * 描述  ：判断遥控器是否正常连接 通过读取遥控器断开时的输入捕获决定大小阈值参数
 * 输入  ：无
 * 输出  ：0：连接失败；1：正常连接
 */  
uint8_t Is_WFLY_Connected(void)
{

	if(Rc_Channel_Val[2] < 950) 
	{	
		BSP_LED_Off(LED_B);//蓝色灯熄灭，提示遥控器连接断开
		return 0;
	}
	else 
	{
		BSP_LED_On(LED_B);//蓝色灯亮起，提示遥控器连接上
		return 1;
	}
}
      



