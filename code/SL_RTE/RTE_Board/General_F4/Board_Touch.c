#include "Board_Touch.h"
#include "Board_E2PROM.h"
#include "Board_LCDFSMC.h"
#include "R_Touch_Adjust.h"
//默认为touchtype=0的数据.
uint8_t touchtype=0;  //触摸屏类型 0为电阻屏  1为电容屏 后续会推出电容屏
	
uint16_t Xdown=0; 		 
uint16_t Ydown=0;	   //触摸屏被按下就返回的的坐标值
uint16_t Xup=0;
uint16_t Yup=0; 			 //触摸屏被按下之后抬起返回的的坐标值 

uint8_t  CMD_RDX=0XD0;
uint8_t  CMD_RDY=0X90;

/****************************************************************************
* 名    称: void TP_Write_Byte(uint8_t num) 
* 功    能：SPI写数据,向触摸屏IC写入1byte数据 
* 入口参数：num:要写入的数据
* 返回参数：无
* 说    明：       
****************************************************************************/
void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0; 
		RTE_RoundRobin_DelayUS(1);
		TCLK=1;		 //上升沿有效	        
	}		 			    
} 	

/****************************************************************************
* 名    称: uint16_t TP_Read_AD(uint8_t CMD)
* 功    能：SPI读数据 ,从触摸屏IC读取adc值
* 入口参数：CMD:指令
* 返回参数: 读到的数据	   
* 说    明：       
****************************************************************************/  
uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count=0; 	  
	uint16_t Num=0; 
	TCLK=0;		//先拉低时钟 	 
	TDIN=0; 	//拉低数据线
	TCS=0; 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	RTE_RoundRobin_DelayUS(6); 
	TCLK=0; 	     	    
	RTE_RoundRobin_DelayUS(1);    	   
	TCLK=1;		//给1个时钟，清除BUSY
	RTE_RoundRobin_DelayUS(1);    
	TCLK=0; 	     	    
	for(count=0;count<16;count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		TCLK=0;	//下降沿有效  	    	   
		RTE_RoundRobin_DelayUS(1);    
 		TCLK=1;
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//只有高12位有效.
	TCS=1;		//释放片选	 
	return(Num);   
}

/****************************************************************************
* 名    称: uint16_t TP_Read_XorY(uint8_t xy)
* 功    能：读取一个坐标值(x或者y)
* 入口参数：xoy:指令（CMD_RDX/CMD_RDY
* 返回参数: 读到的数据	   
* 说    明：连续读取5次数据,对这些数据升序排列, 然后去掉最低和最1个数,取平均值       
****************************************************************************/ 
uint16_t TP_Read_XorY(uint8_t xoy)
{
	uint16_t i, j;
	uint16_t buf[5];
	uint16_t sum=0;
	uint16_t temp;
	
	for(i=0;i<5;i++)buf[i]=TP_Read_AD(xoy);		 		    
	for(i=0;i<5-1; i++)  //排序
	{
		for(j=i+1;j<5;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=1;i<5-1;i++)sum+=buf[i];
	temp=sum/(5-2*1);
	return temp;   
} 

/****************************************************************************
* 名    称: uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
* 功    能：读取x,y坐标
* 入口参数：x,y:读取到的坐标值
* 返回参数: 返回值:0,失败;1,成功   
* 说    明：最小值不能少于50.       
****************************************************************************/ 
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XorY(CMD_RDX);
	ytemp=TP_Read_XorY(CMD_RDY);	  												   
	if(xtemp<50||ytemp<50)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}

/****************************************************************************
* 名    称: uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y)
* 功    能：连续2次读取触摸屏IC
* 入口参数：x,y:读取到的坐标值
* 返回参数: 返回值:0,失败;1,成功   
* 说    明：        
****************************************************************************/

uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+50)||(x1<=x2&&x2<x1+50))  //前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+50)||(y1<=y2&&y2<y1+50)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}  

/****************************************************************************
* 名    称: uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y)
* 功    能：画一个点(2*2的点)	
* 入口参数：x,y:坐标
            color:颜色
* 返回参数: 无  
* 说    明：        
****************************************************************************/
void TP_Draw_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	LCD_Handle.BrushColor = color;
	LCD_DrawPoint(x,y);  //中心点 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}	


uint16_t x;
uint16_t y;
uint8_t TPtime;

/****************************************************************************
* 名    称: void TP_Scan(uint8_t tp)
* 功    能：触摸按键扫描	
* 入口参数：type:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
* 返回参数: 无  
* 说    明：        
****************************************************************************/
void TP_Scan(uint8_t type)
{			   
		Xup=0xffff;
    Yup=0xffff;	 
	if(PEN==0)//有按键按下
	{
		if(type)TP_Read_XY2(&x,&y);//读取物理坐标
		else if(TP_Read_XY2(&x,&y))//读取屏幕坐标
		{
	 		x=xfac*x+xoff;//将结果转换为屏幕坐标
			y=yfac*y+yoff;  
	 	} 	
		Xdown=320-y;
		Ydown=x;
		TPtime++;		   
	}else
	{    
		 if(TPtime>2)
		 {
			 		Xup=320-y;
		      Yup=x;	 
		 }
		 TPtime=0;
		 Xdown=0xffff;
     Ydown=0xffff;	 
	}
}	

//触摸屏初始化  		    
//返回值:0,没有进行校准
//       1,进行过校准
uint8_t Touch_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;	  
		
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOB,C,F时钟

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;//PB1/PB2 设置为上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PF11设置推挽输出
	  GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化	
		
		GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;//PC13设置为推挽输出
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化	
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化	
   
		TP_Read_XY(&Xdown,&Ydown);//第一次读取初始化	 
		AT24C02_Init();		  //初始化24CXX
		if(TP_Get_Adjdata())return 0;//已经校准
		else			   //未校准
		{ 										    
			LCD_Clear(WHITE);//清屏
			TP_Adjust();  	//屏幕校准 
			TP_Save_Adjdata();	 
		}			
		TP_Get_Adjdata();	
	  return 1; 									 
}
