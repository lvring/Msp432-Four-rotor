#ifndef __BOARD_TOUCH_H
#define __BOARD_TOUCH_H
#include "stm32f4xx.h"
#include "RTE_Include.h"
#include "common.h"
//////////////////////////////////////////////////////////////////////////////////	 
extern uint8_t  CMD_RDX;
extern uint8_t  CMD_RDY;
//电阻屏芯片连接引脚	   
#define PEN  		PFin(11)  	//T_PEN
#define DOUT 		PBin(2)   	//T_MISO
#define TDIN 		PBout(1)  	//T_MOSI
#define TCLK 		PAout(5)  	//T_SCK
#define TCS  		PBout(0)  	//T_CS 
//触摸屏控制器参数
extern	uint8_t touchtype;  //触摸屏类型 0为电阻屏  1为电容屏 后续会推出电容屏
extern	uint16_t Xdown; 		 
extern	uint16_t Ydown;	   //触摸屏被按下就返回的的坐标值
extern  uint16_t Xup;
extern  uint16_t Yup;       //触摸屏被按下之后抬起返回的的坐标值  
//电阻屏函数
/*********电阻屏SPI通信与读取触摸点AD值*********************/
void TP_Write_Byte(uint8_t num);				  //向控制芯片写入一个数据
uint16_t TP_Read_AD(uint8_t CMD);							//读取AD转换值
uint16_t TP_Read_XorY(uint8_t xy);							//带滤波的坐标读取(X/Y)
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);				//双方向读取(X+Y)
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);			//两次读取的双方向坐标读取

/*********电阻屏初始化函数*********************/
uint8_t Touch_Init(void);						 	//初始化
/*********电阻屏扫描按键函数*********************/
void TP_Scan(uint8_t tp);	        //tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
void TP_Draw_Point(uint16_t x,uint16_t y,uint16_t color);	//在按下处画一个点				


#endif
