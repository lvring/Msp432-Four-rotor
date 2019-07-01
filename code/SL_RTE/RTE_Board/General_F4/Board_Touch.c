#include "Board_Touch.h"
#include "Board_E2PROM.h"
#include "Board_LCDFSMC.h"
#include "R_Touch_Adjust.h"
//Ĭ��Ϊtouchtype=0������.
uint8_t touchtype=0;  //���������� 0Ϊ������  1Ϊ������ �������Ƴ�������
	
uint16_t Xdown=0; 		 
uint16_t Ydown=0;	   //�����������¾ͷ��صĵ�����ֵ
uint16_t Xup=0;
uint16_t Yup=0; 			 //������������֮��̧�𷵻صĵ�����ֵ 

uint8_t  CMD_RDX=0XD0;
uint8_t  CMD_RDY=0X90;

/****************************************************************************
* ��    ��: void TP_Write_Byte(uint8_t num) 
* ��    �ܣ�SPIд����,������ICд��1byte���� 
* ��ڲ�����num:Ҫд�������
* ���ز�������
* ˵    ����       
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
		TCLK=1;		 //��������Ч	        
	}		 			    
} 	

/****************************************************************************
* ��    ��: uint16_t TP_Read_AD(uint8_t CMD)
* ��    �ܣ�SPI������ ,�Ӵ�����IC��ȡadcֵ
* ��ڲ�����CMD:ָ��
* ���ز���: ����������	   
* ˵    ����       
****************************************************************************/  
uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count=0; 	  
	uint16_t Num=0; 
	TCLK=0;		//������ʱ�� 	 
	TDIN=0; 	//����������
	TCS=0; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	RTE_RoundRobin_DelayUS(6); 
	TCLK=0; 	     	    
	RTE_RoundRobin_DelayUS(1);    	   
	TCLK=1;		//��1��ʱ�ӣ����BUSY
	RTE_RoundRobin_DelayUS(1);    
	TCLK=0; 	     	    
	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		TCLK=0;	//�½�����Ч  	    	   
		RTE_RoundRobin_DelayUS(1);    
 		TCLK=1;
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	TCS=1;		//�ͷ�Ƭѡ	 
	return(Num);   
}

/****************************************************************************
* ��    ��: uint16_t TP_Read_XorY(uint8_t xy)
* ��    �ܣ���ȡһ������ֵ(x����y)
* ��ڲ�����xoy:ָ�CMD_RDX/CMD_RDY
* ���ز���: ����������	   
* ˵    ����������ȡ5������,����Щ������������, Ȼ��ȥ����ͺ���1����,ȡƽ��ֵ       
****************************************************************************/ 
uint16_t TP_Read_XorY(uint8_t xoy)
{
	uint16_t i, j;
	uint16_t buf[5];
	uint16_t sum=0;
	uint16_t temp;
	
	for(i=0;i<5;i++)buf[i]=TP_Read_AD(xoy);		 		    
	for(i=0;i<5-1; i++)  //����
	{
		for(j=i+1;j<5;j++)
		{
			if(buf[i]>buf[j])//��������
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
* ��    ��: uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
* ��    �ܣ���ȡx,y����
* ��ڲ�����x,y:��ȡ��������ֵ
* ���ز���: ����ֵ:0,ʧ��;1,�ɹ�   
* ˵    ������Сֵ��������50.       
****************************************************************************/ 
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XorY(CMD_RDX);
	ytemp=TP_Read_XorY(CMD_RDY);	  												   
	if(xtemp<50||ytemp<50)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}

/****************************************************************************
* ��    ��: uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y)
* ��    �ܣ�����2�ζ�ȡ������IC
* ��ڲ�����x,y:��ȡ��������ֵ
* ���ز���: ����ֵ:0,ʧ��;1,�ɹ�   
* ˵    ����        
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
    if(((x2<=x1&&x1<x2+50)||(x1<=x2&&x2<x1+50))  //ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+50)||(y1<=y2&&y2<y1+50)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}  

/****************************************************************************
* ��    ��: uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y)
* ��    �ܣ���һ����(2*2�ĵ�)	
* ��ڲ�����x,y:����
            color:��ɫ
* ���ز���: ��  
* ˵    ����        
****************************************************************************/
void TP_Draw_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	LCD_Handle.BrushColor = color;
	LCD_DrawPoint(x,y);  //���ĵ� 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}	


uint16_t x;
uint16_t y;
uint8_t TPtime;

/****************************************************************************
* ��    ��: void TP_Scan(uint8_t tp)
* ��    �ܣ���������ɨ��	
* ��ڲ�����type:0,��Ļ����;1,��������(У׼�����ⳡ����)
* ���ز���: ��  
* ˵    ����        
****************************************************************************/
void TP_Scan(uint8_t type)
{			   
		Xup=0xffff;
    Yup=0xffff;	 
	if(PEN==0)//�а�������
	{
		if(type)TP_Read_XY2(&x,&y);//��ȡ��������
		else if(TP_Read_XY2(&x,&y))//��ȡ��Ļ����
		{
	 		x=xfac*x+xoff;//�����ת��Ϊ��Ļ����
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

//��������ʼ��  		    
//����ֵ:0,û�н���У׼
//       1,���й�У׼
uint8_t Touch_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;	  
		
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOB,C,Fʱ��

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;//PB1/PB2 ����Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PF11�����������
	  GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��	
		
		GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;//PC13����Ϊ�������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
	  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��	
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��	
   
		TP_Read_XY(&Xdown,&Ydown);//��һ�ζ�ȡ��ʼ��	 
		AT24C02_Init();		  //��ʼ��24CXX
		if(TP_Get_Adjdata())return 0;//�Ѿ�У׼
		else			   //δУ׼
		{ 										    
			LCD_Clear(WHITE);//����
			TP_Adjust();  	//��ĻУ׼ 
			TP_Save_Adjdata();	 
		}			
		TP_Get_Adjdata();	
	  return 1; 									 
}
