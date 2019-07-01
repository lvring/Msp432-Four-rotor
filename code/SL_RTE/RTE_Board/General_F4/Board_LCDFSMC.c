#include "Board_LCDFSMC.h"
#include "Board_Font.h"   
LCD_Control_t LCD_Handle;
uint16_t BACK_COLOR  = 0;
uint16_t BRUSH_COLOR = RED;
uint16_t lcd_height = 240;
uint16_t lcd_width = 320;
void delay_ms(uint32_t ms)
{
	RTE_RoundRobin_DelayMS(ms);
}
/****************************************************************************
* ��    ��: void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value)
* ��    �ܣ�LCDд�Ĵ���
* ��ڲ�����LCD_Reg: �Ĵ�����ַ
*           LCD_RegValue: Ҫд�������
* ���ز�������
* ˵    ����       
****************************************************************************/				   
void LCD_WriteReg(volatile uint16_t LCD_Reg, volatile uint16_t LCD_Value)
{	
	LCD_CMD = LCD_Reg;		 //д��Ҫд�ļĴ������	 
	LCD_DATA = LCD_Value;  //��Ĵ���д�������	    		 
}
/****************************************************************************
* ��    ��: uint16_t LCD_ReadReg(uint16_t LCD_Reg)
* ��    �ܣ�LCD���Ĵ���
* ��ڲ�����LCD_Reg:�Ĵ�����ַ
* ���ز����������üĴ���������ֵ
* ˵    ����       
****************************************************************************/	
uint16_t LCD_ReadReg(volatile uint16_t LCD_Reg)
{										   
	LCD_CMD=LCD_Reg;		//д��Ҫ���ļĴ������
	RTE_RoundRobin_DelayUS(5);		  
	return LCD_DATA;		//���ض�����ֵ
}   
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD_CMD=LCD_Handle.WriteGramCmd;	  
}
//lcd��ʱ����
void lcdm_delay(volatile uint8_t i)
{
	while(i--);
}
/****************************************************************************
* ��    ��: uint16_t LCD_BGRtoRGB(uint16_t bgr)
* ��    �ܣ�ͨ���ú�����GBRת��RGB
* ��ڲ�����bgr:GBR��ʽ����ɫֵ
* ���ز�����RGB��ʽ����ɫֵ
* ˵    ������ILI93xx����������ΪGBR��ʽ��������д���ʱ��ΪRGB��ʽ       
****************************************************************************/	
uint16_t LCD_BGRtoRGB(uint16_t bgr)
{
	uint16_t  r,g,b,rgb;   
	b=(bgr>>0)&0x1f;
	g=(bgr>>5)&0x3f;
	r=(bgr>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 
/****************************************************************************
* ��    ��: uint16_t LCD_GetPoint(uint16_t x,uint16_t y)
* ��    �ܣ���ȡĳ�����ɫֵ	
* ��ڲ�����x��x����
            y��y����
* ���ز������˵����ɫ
* ˵    ����     
****************************************************************************/
uint16_t LCD_GetPoint(uint16_t x,uint16_t y)
{
 	volatile uint16_t r=0,g=0,b=0;

	if(x>=LCD_Handle.Width||y>=LCD_Handle.Height)
		return 0;	 //�����˷�Χ,ֱ�ӷ���		   
	LCD_SetCursor(x,y);	    
	if(LCD_Handle.ID==0x9341)
		  LCD_CMD=0X2E;   //9341 ���Ͷ�GRAMָ��
	else
	    LCD_CMD=R34;    //����IC���Ͷ�GRAMָ��   		 				 
	if(LCD_DATA)
		r=0;						
	lcdm_delay(2);	  
 	r=LCD_DATA;  		  						 //ʵ��������ɫ
	
	if(LCD_Handle.ID==0X9341)		//9341Ҫ��2�ζ���
 	{
		lcdm_delay(2);	  
		b=LCD_DATA; 
		g=r&0XFF;		 //����9341��һ�ζ�ȡ����RG��ֵ,R��ǰ,G�ں�,��ռ8λ
		g<<=8;
	} 
	
	if(LCD_Handle.ID==0x9325)
		   return r;
	else if(LCD_Handle.ID==0x9341)
		   return (((r>>11)<<11)|((g>>10)<<5)|(b>>11)); //ILI9341��Ҫ��ʽת��һ��
	else
	     return LCD_BGRtoRGB(r);		  	
}	

//LCD������ʾ
void LCD_DisplayOn(void)
{					   
	 if(LCD_Handle.ID==0x9341)
		   LCD_CMD=0x29;
	 else
	     LCD_WriteReg(R7,0x0173);  //������ʾ
}	

//LCD�ر���ʾ
void LCD_DisplayOff(void)
{	
	 if(LCD_Handle.ID==0x9341)
		   LCD_CMD=0x28;
	 else	
	     LCD_WriteReg(R7,0x0);    //�ر���ʾ 
}   
/****************************************************************************
* ��    ��: void LCD_SetCursor(uint16_t Xaddr, uint16_t Yaddr)
* ��    �ܣ����ù��λ��
* ��ڲ�����x��x����
            y��y����
* ���ز�������
* ˵    ����     
****************************************************************************/
void LCD_SetCursor(uint16_t Xaddr, uint16_t Yaddr)
{	 
	if(LCD_Handle.ID==0X9341)
		{		    
			LCD_CMD=LCD_Handle.SetXCmd; 
			LCD_DATA=(Xaddr>>8); 
			LCD_DATA=(Xaddr&0XFF);	 
			LCD_CMD=LCD_Handle.SetYCmd; 
			LCD_DATA=(Yaddr>>8); 
			LCD_DATA=(Yaddr&0XFF);
		}
	else
	{
		if(LCD_Handle.DisplayDirection==1)Xaddr=LCD_Handle.Width-1-Xaddr;//�������ǵ�תx,y����
		LCD_WriteReg(LCD_Handle.SetXCmd, Xaddr);
		LCD_WriteReg(LCD_Handle.SetYCmd, Yaddr); 
	}
} 
/****************************************************************************
* ��    ��: void LCD_AUTOScan_Dir(uint8_t dir)
* ��    �ܣ�����LCD���Զ�ɨ�跽��
* ��ڲ�����dir��ɨ�跽��
* ���ز�������
* ˵    ����     
****************************************************************************/  	   
void LCD_AUTOScan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	uint16_t temp; 
	
	if(LCD_Handle.DisplayDirection==1)//����ʱ
	{			   
			switch(dir)//����ת��
			{
				case 0:dir=6;break;
				case 1:dir=7;break;
				case 2:dir=4;break;
				case 3:dir=5;break;
				case 4:dir=1;break;
				case 5:dir=0;break;
				case 6:dir=3;break;
				case 7:dir=2;break;	     
			}
  }
	if(LCD_Handle.ID==0x9341) //9341������
	{
		switch(dir)
		{
			case L2R_U2D://������,���ϵ���
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://������,���µ���
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://���ҵ���,���ϵ���
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://���ҵ���,���µ���
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://���ϵ���,������
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://���ϵ���,���ҵ���
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://���µ���,������
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://���µ���,���ҵ���
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;	 
		}
     dirreg=0X36;  
     regval|=0X08;		
		 LCD_WriteReg(dirreg,regval);
     
		 if(regval&0X20)
		{
			if(LCD_Handle.Width<LCD_Handle.Height)//����X,Y
			{
				temp=LCD_Handle.Width;
				LCD_Handle.Width=LCD_Handle.Height;
				LCD_Handle.Height=temp;
 			}
		}
			LCD_CMD=LCD_Handle.SetXCmd; 
			LCD_DATA=0;LCD_DATA=0;
			LCD_DATA=(LCD_Handle.Width-1)>>8;LCD_DATA=(LCD_Handle.Width-1)&0XFF;
			LCD_CMD=LCD_Handle.SetYCmd; 
			LCD_DATA=0;LCD_DATA=0;
			LCD_DATA=(LCD_Handle.Height-1)>>8;LCD_DATA=(LCD_Handle.Height-1)&0XFF;  
  }			   
	else
	{		
	 switch(dir)
		{
			case L2R_U2D://������,���ϵ���
				regval|=(1<<5)|(1<<4)|(0<<3); 
				break;
			case L2R_D2U://������,���µ���
				regval|=(0<<5)|(1<<4)|(0<<3); 
				break;
			case R2L_U2D://���ҵ���,���ϵ���
				regval|=(1<<5)|(0<<4)|(0<<3);
				break;
			case R2L_D2U://���ҵ���,���µ���
				regval|=(0<<5)|(0<<4)|(0<<3); 
				break;	
			case D2U_L2R://���µ���,������
				regval|=(0<<5)|(1<<4)|(1<<3); 
				break;
			case D2U_R2L://���µ���,���ҵ���
				regval|=(0<<5)|(0<<4)|(1<<3); 
				break;				
			case U2D_L2R://���ϵ���,������
				regval|=(1<<5)|(1<<4)|(1<<3); 
				break;
			case U2D_R2L://���ϵ���,���ҵ���
				regval|=(1<<5)|(0<<4)|(1<<3); 
				break; 
		}
		dirreg=0X03;
		regval|=1<<12;  
		LCD_WriteReg(dirreg,regval);
	}
}
/****************************************************************************
* ��    ��: void LCD_Display_Dir(uint8_t dir)
* ��    �ܣ�����LCD��ʾ����
* ��ڲ�����dir: 0,����
                 1,����
* ���ز�������
* ˵    �����B
****************************************************************************/
void LCD_Display_Dir(uint8_t dir)
{
	if(dir==0)		  //����
	{
		LCD_Handle.DisplayDirection=0;	
		LCD_Handle.Width=240;
		LCD_Handle.Height=320;	
    if(LCD_Handle.ID==0X9341)
			{
				LCD_Handle.WriteGramCmd=0X2C;
				LCD_Handle.SetXCmd=0X2A;
				LCD_Handle.SetYCmd=0X2B;  	 
			}	
    else	
			{			
				LCD_Handle.WriteGramCmd=R34;
				LCD_Handle.SetXCmd=R32;
				LCD_Handle.SetYCmd=R33;  
			}
	}
	else 				  //����
	{	  				
      LCD_Handle.DisplayDirection=1;
			LCD_Handle.Width=320;
   		LCD_Handle.Height=240;
		if(LCD_Handle.ID==0X9341)
			{
				LCD_Handle.WriteGramCmd=0X2C;
				LCD_Handle.SetXCmd=0X2A;
				LCD_Handle.SetYCmd=0X2B;  	 
			}
		else
		{
			LCD_Handle.WriteGramCmd=R34;
	 		LCD_Handle.SetXCmd=R33;
			LCD_Handle.SetYCmd=R32; 
    }			
	} 
	LCD_AUTOScan_Dir(INIT_SCAN_DIR);	//����ɨ�跽��
}	
/****************************************************************************
* ��    ��: void LCD_DrawPoint(uint16_t x,uint16_t y)
* ��    �ܣ����㣨�ڸõ�д�뻭�ʵ���ɫ��
* ��ڲ�����x��x����
            y��y����
* ���ز�������
* ˵    ���LCD_Handle.BrushColor:�˵����ɫֵ
****************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		    //���ù��λ�� 
	LCD_WriteRAM_Prepare();	  //��ʼд��GRAM
	LCD_DATA=LCD_Handle.BrushColor; 
}
/****************************************************************************
* ��    ��: void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
* ��    �ܣ������õ����괦����Ӧ��ɫ���ڸõ�д���Զ�����ɫ��
* ��ڲ�����x��x����
            y��y����
            color �˵����ɫֵ
* ���ز�������
* ˵    ����color:д��˵����ɫֵ   UCGUI���øú���
****************************************************************************/
void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	       
	 if(LCD_Handle.ID==0X9341)
		{
			LCD_CMD=LCD_Handle.SetXCmd; 
			LCD_DATA=(x>>8); 
			LCD_DATA=(x&0XFF);	 
			LCD_CMD=LCD_Handle.SetYCmd; 
			LCD_DATA=(y>>8); 
			LCD_DATA=(y&0XFF);
		}
	  else
		{
			if(LCD_Handle.DisplayDirection==1)x=LCD_Handle.Width-1-x;  //�������ǵ�תx,y����
			LCD_WriteReg(LCD_Handle.SetXCmd,x);
			LCD_WriteReg(LCD_Handle.SetYCmd,y);
    }			
	  LCD_CMD=LCD_Handle.WriteGramCmd; 
	  LCD_DATA=color; 
}	 
/****************************************************************************
* ��    ��: void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
* ��    �ܣ����ô���,������û������굽�������Ͻ�(sx,sy)
* ��ڲ�����sx,sy:������ʼ����(���Ͻ�)
            width,height:���ڿ�Ⱥ͸߶�
* ���ز�������
* ˵    ���������С:width*height.�B
****************************************************************************/
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{   
	uint8_t  h1reg,h2reg,v1reg,v2reg;
	uint16_t h1val,h2val,v1val,v2val; 
	width=sx+width-1;
	height=sy+height-1;
  
	if(LCD_Handle.ID==0X9341)
	{
		LCD_CMD=LCD_Handle.SetXCmd; 
		LCD_DATA=(sx>>8); 
		LCD_DATA=(sx&0XFF);	 
		LCD_DATA=(width>>8); 
		LCD_DATA=(width&0XFF);  
		LCD_CMD=LCD_Handle.SetYCmd; 
		LCD_DATA=(sy>>8); 
		LCD_DATA=(sy&0XFF); 
		LCD_DATA=(height>>8); 
		LCD_DATA=(height&0XFF); 
	}
	else
	{	
		if(LCD_Handle.DisplayDirection==1)  //����
		{
			h1val=sy;				
			h2val=height;
			v1val=LCD_Handle.Width-width-1;
			v2val=LCD_Handle.Width-sx-1;				
		}else           //����
		{ 
			h1val=sx;				
			h2val=width;
			v1val=sy;
			v2val=height;
		}
		h1reg=0X50;h2reg=0X51;     //ˮƽ���򴰿ڼĴ���
		v1reg=0X52;v2reg=0X53;     //��ֱ���򴰿ڼĴ���	  

		LCD_WriteReg(h1reg,h1val);
		LCD_WriteReg(h2reg,h2val);
		LCD_WriteReg(v1reg,v1val);
		LCD_WriteReg(v2reg,v2val);		
		LCD_SetCursor(sx,sy);	      //���ù��λ��
	}
} 
//����FSMC
void LCD_FSMC_Config()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��PD,PE,PF,PGʱ��  
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//ʹ��FSMCʱ��  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;        //PF10 �������,���Ʊ���
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //����
  GPIO_Init(GPIOF, &GPIO_InitStructure);            //��ʼ�� //PF10 �������,���Ʊ���
	
  GPIO_InitStructure.GPIO_Pin = (3<<0)|(3<<4)|(7<<8)|(3<<14); 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //�������
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //����
  GPIO_Init(GPIOD, &GPIO_InitStructure);            //��ʼ��  
	
  GPIO_InitStructure.GPIO_Pin = (0X1FF<<7);         //PE7~15,AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //�������
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //����
  GPIO_Init(GPIOE, &GPIO_InitStructure);            //��ʼ��  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;         //PG2,FSMC_A12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //�������
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //����
  GPIO_Init(GPIOG, &GPIO_InitStructure);            //��ʼ��  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;        //PG12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      //�������
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      //����
  GPIO_Init(GPIOG, &GPIO_InitStructure);            //��ʼ�� 

  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);//PD0,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);//PD1,AF12
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);//PD15,AF12
 
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC); //PE7,AF12
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//PE15,AF12
 
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource2,GPIO_AF_FSMC);//PF12,AF12
  GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);

  readWriteTiming.FSMC_AddressSetupTime = 0XF;	 //��ַ����ʱ�䣨ADDSET��Ϊ16��HCLK 1/168M=6ns*16=96ns	
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ�	
  readWriteTiming.FSMC_DataSetupTime = 60;			 //���ݱ���ʱ��Ϊ60��HCLK	=6*60=360ns
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 
    
	writeTiming.FSMC_AddressSetupTime =9;	     //��ַ����ʱ�䣨ADDSET��Ϊ9��HCLK =54ns 
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨A		
  writeTiming.FSMC_DataSetupTime = 8;		     //���ݱ���ʱ��Ϊ6ns*9��HCLK=54ns
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//   ʹ��NE4 ��Ҳ�Ͷ�ӦBTCR[6],[7]��
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // ���������ݵ�ַ
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//�洢�����ݿ��Ϊ16bit   
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  �洢��дʹ��
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // ��дʹ�ò�ͬ��ʱ��
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //��дʱ��
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //дʱ��
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //��ʼ��FSMC����
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // ʹ��BANK1 
				 
  RTE_RoundRobin_DelayMS(50); // delay 50 ms 
}
//��ʼ��lcd
void LCD_Init(void)
{ 			
 	volatile uint32_t i=0;
	LCD_FSMC_Config();
	LCD_WriteReg(0x0000,0x0001);
	RTE_RoundRobin_DelayMS(50); 
  LCD_Handle.ID = LCD_ReadReg(0x0000);    //��ȡLCD ID
	if(LCD_Handle.ID<0XFF||LCD_Handle.ID==0XFFFF||LCD_Handle.ID==0X9300) //����ID����ȷ 
	{	
 		//����9341 ID�Ķ�ȡ		
		LCD_CMD=0XD3;				   
		LCD_Handle.ID=LCD_DATA;	//dummy read 	
 		LCD_Handle.ID=LCD_DATA; 	//����0X00
  	LCD_Handle.ID=LCD_DATA;   	//��ȡ93								   
 		LCD_Handle.ID<<=8;
		LCD_Handle.ID|=LCD_DATA;  	//��ȡ41 
	}
  if(LCD_Handle.ID==0X9341) //�����������IC,������WRʱ��Ϊ���
	{
		//��������дʱ����ƼĴ�����ʱ��   	 							    
		FSMC_Bank1E->BWTR[6]&=~(0XF<<0); //��ַ����ʱ�䣨ADDSET������ 	 
		FSMC_Bank1E->BWTR[6]&=~(0XF<<8); //���ݱ���ʱ������
		FSMC_Bank1E->BWTR[6]|=3<<0;		   //��ַ����ʱ�䣨ADDSET��Ϊ3��HCLK =18ns  	 
		FSMC_Bank1E->BWTR[6]|=2<<8;    	 //���ݱ���ʱ��Ϊ6ns*3��HCLK=18ns
	}
if(LCD_Handle.ID==0x9328) //ILI9328��ʼ��  
	{
  		LCD_WriteReg(0x00EC,0x108F);   
 		  LCD_WriteReg(0x00EF,0x1234); // ADD        
      LCD_WriteReg(0x0001,0x0100);     
      LCD_WriteReg(0x0002,0x0700); //��Դ����                    
			LCD_WriteReg(0x0003,(1<<12)|(3<<4)|(0<<3) ); 
			LCD_WriteReg(0x0004,0x0000);                                   
			LCD_WriteReg(0x0008,0x0202);	           
			LCD_WriteReg(0x0009,0x0000);         
			LCD_WriteReg(0x000a,0x0000);//��ʾ����     
			LCD_WriteReg(0x000c,0x0001);        
			LCD_WriteReg(0x000d,0x0000);        
			LCD_WriteReg(0x000f,0x0000);   
			LCD_WriteReg(0x0010,0x0000); //��Դ����  
			LCD_WriteReg(0x0011,0x0007);
			LCD_WriteReg(0x0012,0x0000);                                                                 
			LCD_WriteReg(0x0013,0x0000);                 
			LCD_WriteReg(0x0007,0x0001);                 
			RTE_RoundRobin_DelayMS(50); 
			LCD_WriteReg(0x0010,0x1490);   
			LCD_WriteReg(0x0011,0x0227);
			RTE_RoundRobin_DelayMS(50); 
			LCD_WriteReg(0x0012,0x008A);                  
			RTE_RoundRobin_DelayMS(50); 
			LCD_WriteReg(0x0013,0x1a00);   
			LCD_WriteReg(0x0029,0x0006);
			LCD_WriteReg(0x002b,0x000d);
			RTE_RoundRobin_DelayMS(50); 
			LCD_WriteReg(0x0020,0x0000);                                                            
			LCD_WriteReg(0x0021,0x0000);           
			RTE_RoundRobin_DelayMS(50); 	
			LCD_WriteReg(0x0030,0x0000); //٤��У��
			LCD_WriteReg(0x0031,0x0604);   
			LCD_WriteReg(0x0032,0x0305);
			LCD_WriteReg(0x0035,0x0000);
			LCD_WriteReg(0x0036,0x0C09); 
			LCD_WriteReg(0x0037,0x0204);
			LCD_WriteReg(0x0038,0x0301);        
			LCD_WriteReg(0x0039,0x0707);     
			LCD_WriteReg(0x003c,0x0000);
			LCD_WriteReg(0x003d,0x0a0a);
			RTE_RoundRobin_DelayMS(50); 
			LCD_WriteReg(0x0050,0x0000); //ˮƽGRAM��ʼλ�� 
			LCD_WriteReg(0x0051,0x00ef); //ˮƽGRAM��ֹλ��                    
			LCD_WriteReg(0x0052,0x0000); //��ֱGRAM��ʼλ��                    
			LCD_WriteReg(0x0053,0x013f); //��ֱGRAM��ֹλ��  
			LCD_WriteReg(0x0060,0xa700);        
			LCD_WriteReg(0x0061,0x0001); 
			LCD_WriteReg(0x006a,0x0000);
			LCD_WriteReg(0x0080,0x0000);
			LCD_WriteReg(0x0081,0x0000);
			LCD_WriteReg(0x0082,0x0000);
			LCD_WriteReg(0x0083,0x0000);
			LCD_WriteReg(0x0084,0x0000);
			LCD_WriteReg(0x0085,0x0000);	
			LCD_WriteReg(0x0090,0x0010);     
			LCD_WriteReg(0x0092,0x0600);  		
			LCD_WriteReg(0x0007,0x0133);//������ʾ����     
	}
		else if(LCD_Handle.ID==0x9325)//9325
	{
		LCD_WriteReg(0x00E5,0x78F0); 
		LCD_WriteReg(0x0001,0x0100); 
		LCD_WriteReg(0x0002,0x0700); 
		LCD_WriteReg(0x0003,0x1030); 
		LCD_WriteReg(0x0004,0x0000); 
		LCD_WriteReg(0x0008,0x0202);  
		LCD_WriteReg(0x0009,0x0000);
		LCD_WriteReg(0x000A,0x0000); 
		LCD_WriteReg(0x000C,0x0000); 
		LCD_WriteReg(0x000D,0x0000);
		LCD_WriteReg(0x000F,0x0000);
    //power on sequence VGHVGL
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);  
		LCD_WriteReg(0x0012,0x0000);  
		LCD_WriteReg(0x0013,0x0000); 
		LCD_WriteReg(0x0007,0x0000); 
		//vgh 
		LCD_WriteReg(0x0010,0x1690);   
		LCD_WriteReg(0x0011,0x0227);
		//delayms(100);
		//vregiout 
		LCD_WriteReg(0x0012,0x009D); //0x001b
		//delayms(100); 
		//vom amplitude
		LCD_WriteReg(0x0013,0x1900);
		//delayms(100); 
		//vom H
		LCD_WriteReg(0x0029,0x0025); 
		LCD_WriteReg(0x002B,0x000D); 
		//gamma
		LCD_WriteReg(0x0030,0x0007);
		LCD_WriteReg(0x0031,0x0303);
		LCD_WriteReg(0x0032,0x0003);// 0006
		LCD_WriteReg(0x0035,0x0206);
		LCD_WriteReg(0x0036,0x0008);
		LCD_WriteReg(0x0037,0x0406); 
		LCD_WriteReg(0x0038,0x0304);//0200
		LCD_WriteReg(0x0039,0x0007); 
		LCD_WriteReg(0x003C,0x0602);// 0504
		LCD_WriteReg(0x003D,0x0008); 
		//ram
		LCD_WriteReg(0x0050,0x0000); 
		LCD_WriteReg(0x0051,0x00EF);
		LCD_WriteReg(0x0052,0x0000); 
		LCD_WriteReg(0x0053,0x013F);  
		LCD_WriteReg(0x0060,0xA700); 
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006A,0x0000); 
		//
		LCD_WriteReg(0x0080,0x0000); 
		LCD_WriteReg(0x0081,0x0000); 
		LCD_WriteReg(0x0082,0x0000); 
		LCD_WriteReg(0x0083,0x0000); 
		LCD_WriteReg(0x0084,0x0000); 
		LCD_WriteReg(0x0085,0x0000); 
		//
		LCD_WriteReg(0x0090,0x0010); 
		LCD_WriteReg(0x0092,0x0600); 
		
		LCD_WriteReg(0x0007,0x0133);
		LCD_WriteReg(0x00,0x0022);//
	}
	else if(LCD_Handle.ID==0X9341)	//9341��ʼ��
	{	 
		LCD_CMD=0xCF;  
		LCD_DATA=0x00;
		LCD_DATA=0xC1; 
		LCD_DATA=0X30; 
		LCD_CMD=0xED;  
		LCD_DATA=0x64; 
		LCD_DATA=0x03; 
		LCD_DATA=0X12; 
		LCD_DATA=0X81; 
		LCD_CMD=0xE8;  
		LCD_DATA=0x85; 
		LCD_DATA=0x10; 
		LCD_DATA=0x7A; 
		LCD_CMD=0xCB;  
		LCD_DATA=0x39; 
		LCD_DATA=0x2C; 
		LCD_DATA=0x00; 
		LCD_DATA=0x34; 
		LCD_DATA=0x02; 
		LCD_CMD=0xF7;  
		LCD_DATA=0x20; 
		LCD_CMD=0xEA;  
		LCD_DATA=0x00; 
		LCD_DATA=0x00; 
		LCD_CMD=0xC0;    //Power control 
		LCD_DATA=0x1B;   //VRH[5:0] 
		LCD_CMD=0xC1;    //Power control 
		LCD_DATA=0x01;   //SAP[2:0];BT[3:0] 
		LCD_CMD=0xC5;    //VCM control 
		LCD_DATA=0x30; 	 //3F
		LCD_DATA=0x30; 	 //3C
		LCD_CMD=0xC7;    //VCM control2 
		LCD_DATA=0XB7; 
		LCD_CMD=0x36;    // Memory Access Control 
		LCD_DATA=0x48; 
		LCD_CMD=0x3A;   
		LCD_DATA=0x55; 
		LCD_CMD=0xB1;   
		LCD_DATA=0x00;   
		LCD_DATA=0x1A; 
		LCD_CMD=0xB6;    // Display Function Control 
		LCD_DATA=0x0A; 
		LCD_DATA=0xA2; 
		LCD_CMD=0xF2;    // 3Gamma Function Disable 
		LCD_DATA=0x00; 
		LCD_CMD=0x26;    //Gamma curve selected 
		LCD_DATA=0x01; 
		LCD_CMD=0xE0;    //Set Gamma 
		LCD_DATA=0x0F; 
		LCD_DATA=0x2A; 
		LCD_DATA=0x28; 
		LCD_DATA=0x08; 
		LCD_DATA=0x0E; 
		LCD_DATA=0x08; 
		LCD_DATA=0x54; 
		LCD_DATA=0XA9; 
		LCD_DATA=0x43; 
		LCD_DATA=0x0A; 
		LCD_DATA=0x0F; 
		LCD_DATA=0x00; 
		LCD_DATA=0x00; 
		LCD_DATA=0x00; 
		LCD_DATA=0x00; 		 
		LCD_CMD=0XE1;    //Set Gamma 
		LCD_DATA=0x00; 
		LCD_DATA=0x15; 
		LCD_DATA=0x17; 
		LCD_DATA=0x07; 
		LCD_DATA=0x11; 
		LCD_DATA=0x06; 
		LCD_DATA=0x2B; 
		LCD_DATA=0x56; 
		LCD_DATA=0x3C; 
		LCD_DATA=0x05; 
		LCD_DATA=0x10; 
		LCD_DATA=0x0F; 
		LCD_DATA=0x3F; 
		LCD_DATA=0x3F; 
		LCD_DATA=0x0F; 
		LCD_CMD=0x2B; 
		LCD_DATA=0x00;
		LCD_DATA=0x00;
		LCD_DATA=0x01;
		LCD_DATA=0x3f;
		LCD_CMD=0x2A; 
		LCD_DATA=0x00;
		LCD_DATA=0x00;
		LCD_DATA=0x00;
		LCD_DATA=0xef;	 
		LCD_CMD=0x11; //Exit Sleep
		RTE_RoundRobin_DelayMS(120);
		LCD_CMD=0x29; //display on	
	}
	LCD_Display_Dir(1);		 //��ʼ��Ϊ����
	GPIO_SetBits(GPIOF,GPIO_Pin_10);//��������
	LCD_Clear(WHITE);
}
/****************************************************************************
* ��    ��: void LCD_Clear(uint16_t color)
* ��    �ܣ���������
* ��ڲ�����color: Ҫ���������ɫ
* ���ز�������
* ˵    �����B
****************************************************************************/
void LCD_Clear(uint16_t color)
{
	uint32_t i=0;      
	uint32_t pointnum=0;
	
	pointnum=LCD_Handle.Width*LCD_Handle.Height; 	 //�õ�LCD�ܵ���
	LCD_SetCursor(0x00,0x00);	       //���ù��λ�� 
	LCD_WriteRAM_Prepare();     		 //��ʼд��GRAM	 	  
	for(i=0;i<pointnum;i++)
	{
		LCD_DATA=color;	   
	}
} 
/****************************************************************************
* ��    ��: void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
* ��    �ܣ���ָ����������䵥����ɫ
* ��ڲ�����(sx,sy),(ex,ey):�����ζԽ�����
            color:Ҫ������ɫ
* ���ز�������
* ˵    ���������СΪ:(ex-sx+1)*(ey-sy+1)  �B
****************************************************************************/
void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
	uint16_t i,j;
	uint16_t nlen=0;

		nlen=ex-sx+1;	 
		for(i=sy;i<=ey;i++)
		{
		 	LCD_SetCursor(sx,i);      				  //���ù��λ�� 
			LCD_WriteRAM_Prepare();     			  //��ʼд��GRAM	  
			for(j=0;j<nlen;j++)LCD_DATA=color;	//���ù��λ�� 	    
		}
} 
/****************************************************************************
* ��    ��: void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
* ��    �ܣ���ָ�������ڻ���ͼƬ
* ��ڲ�����(sx,sy),(ex,ey):�����ζԽ�����
            color:Ҫ����ͼƬ������ɫ����
* ���ز�������
* ˵    ���������СΪ:(ex-sx+1)*(ey-sy+1)  �B
****************************************************************************/
void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	uint16_t height,width;
	uint16_t i,j;
	width=ex-sx+1; 			    //�õ�ͼƬ�Ŀ��
	height=ey-sy+1;			    //�õ�ͼƬ�ĸ߶�
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//���ù��λ�� 
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM
		for(j=0;j<width;j++)LCD_DATA=color[i*height+j];//д����ɫֵ
	}	  
}  
/****************************************************************************
* ��    ��: void LCD_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
* ��    �ܣ�����
* ��ڲ�����x1,y1:�������
            x2,y2:�յ�����
* ���ز�������
* ˵    ��������������Ļ��ߣ�ˮƽ�ߡ���ֱ����б��(���ߡ��������뻭Բ�ο����ϴ���)  �B
************************2****************************************************/  
void LCD_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t i; 
	int xm1=0,ym2=0,model_x,model_y,model,  mcx,mcy,mRow,mCol;  

	model_x=x2-x1;                  //����ֱ�ߵ�ģ ������������ 
	model_y=y2-y1; 
	mRow=x1; 
	mCol=y1; 
	if(model_x>0)mcx=1;       
	else if(model_x==0)mcx=0;      //��ֱ�� 
	else {mcx=-1;model_x=-model_x;} 
	if(model_y>0)mcy=1; 
	else if(model_y==0)mcy=0;      //ˮƽ�� 
	else{mcy=-1;model_y=-model_y;} 
	if( model_x>model_y)model=model_x;  
	else model=model_y; 
	for(i=0;i<=model+1;i++ )       //������� 
	{  
		LCD_DrawPoint(mRow,mCol);     
		xm1+=model_x ; 
		ym2+=model_y ; 
		if(xm1>model) 
		{ 
			xm1-=model; 
			mRow+=mcx; 
		} 
		if(ym2>model) 
		{ 
			ym2-=model; 
			mCol+=mcy; 
		} 
	}  
}
/****************************************************************************
* ��    ��: void LCD_Draw_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
* ��    �ܣ�������	  
* ��ڲ�����(x1,y1),(x2,y2):���εĶԽ�����
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_Draw_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Draw_Line(x1,y1,x2,y1);
	LCD_Draw_Line(x1,y1,x1,y2);
	LCD_Draw_Line(x1,y2,x2,y2);
	LCD_Draw_Line(x2,y1,x2,y2);
}
/****************************************************************************
* ��    ��: void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
* ��    �ܣ���ָ��λ�û�һ��ָ����С��Բ
* ��ڲ�����(x,y):���ĵ�
            r    :�뾶
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             
 		LCD_DrawPoint(x0+b,y0-a);                    
		LCD_DrawPoint(x0+b,y0+a);                       
		LCD_DrawPoint(x0+a,y0+b);             
		LCD_DrawPoint(x0-a,y0+b);             
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);                      
  	LCD_DrawPoint(x0-b,y0-a);               	         
		a++;	
		if(di<0)di +=4*a+6;	  //ʹ��Bresenham�㷨��Բ     
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 
/****************************************************************************
* ��    ��: void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size)
* ��    �ܣ���ָ��λ����ʾһ���ַ�
* ��ڲ�����x,y:��ʼ����
            word:Ҫ��ʾ���ַ�:abcdefg1234567890...
            size:�����С 12/16/24
* ���ز�������
* ˵    ����ȡ��ģ�ο�����ȡ��ģ��ʽ������ģȡģ����Ϊ�ȴ��ϵ��£��ٴ�����  �B
****************************************************************************/
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t word,uint8_t size)
{  							  
  uint8_t  bytenum,bytedata, a,b;
	uint16_t ymid=y;   			     
	 
	if(size==12) bytenum=12;        // �жϸ����������ֿ�������ռ���ֽ���
	else if(size==16) bytenum=16;
	else if(size==24) bytenum=36;
	else return;
	
	word=word-' ';  //�õ�ƫ�ƺ��ֵ ��Ϊ�ո�֮ǰ���ַ�û��font.h�е���������

	    for(b=0;b<bytenum;b++)
	    {   
					if(size==12)bytedata=char_1206[word][b]; 	 	  //����1206����
					else if(size==16)bytedata=char_1608[word][b];	//����1608����
					else if(size==24)bytedata=char_2412[word][b];	//����2412����
					else return;								                  //û�е��ַ����飬û�ַ��ֿ�
					for(a=0;a<8;a++)
					{			    
						if(bytedata&0x80)LCD_Color_DrawPoint(x,y,LCD_Handle.BrushColor);
						else LCD_Color_DrawPoint(x,y,LCD_Handle.BackGroundColor);
						bytedata<<=1;
						y++;
						if(y>=LCD_Handle.Height)return;		//�������˳�����
						if((y-ymid)==size)
						{
							y=ymid;
							x++;
							if(x>=LCD_Handle.Width)return;	  //�������˳�����
							break;
						}
		      }   	 
	    }       	 	  
}   
//m^n����
//����ֵ:m^n�η�.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t mid=1;	 
	while(n--)mid*=m;    
	return mid;
}
/****************************************************************************
* ��    ��: void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* ��    �ܣ���ָ��λ����ʾһ������
* ��ڲ�����x,y:�������
            num:��ֵ(0~999999999);	 
            len:����(��Ҫ��ʾ��λ��)
            size:�����С
            mode: 0����λΪ0����ʾ
                  1����λΪ0��ʾ0
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
	uint8_t t,numtemp;
	uint8_t end0=0;						   
	for(t=0;t<len;t++)
	{
		numtemp=(num/LCD_Pow(10,len-t-1))%10;
		if(end0==0&&t<(len-1))
		{
			if(numtemp==0)
			{
				if(mode)LCD_DisplayChar(x+(size/2)*t,y,'0',size);  
				else LCD_DisplayChar(x+(size/2)*t,y,' ',size);  
 				continue;
			}else end0=1; 
		 	 
		}
	 	LCD_DisplayChar(x+(size/2)*t,y,numtemp+'0',size); 
	}
} 
/****************************************************************************
* ��    ��: void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
* ��    �ܣ���ָ��λ����ʾһ���Զ�����ɫ������
* ��ڲ�����x,y:�������
            num:��ֵ(0~999999999);	 
            len:����(��Ҫ��ʾ��λ��)
            size:�����С
            mode: 0����λΪ0����ʾ
                  1����λΪ0��ʾ0
            brushcolor���Զ��廭����ɫ
            backcolor�� �Զ��屳����ɫ
* ���ز�������
* ˵    ����  �B
****************************************************************************/
void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode,uint16_t brushcolor,uint16_t backcolor)
{
 uint16_t bh_color,bk_color;
	
 	bh_color=LCD_Handle.BrushColor;  //�ݴ滭����ɫ
	bk_color=LCD_Handle.BackGroundColor;   //�ݴ汳����ɫ
	
	LCD_Handle.BrushColor=brushcolor;
	LCD_Handle.BackGroundColor=backcolor;
	
	LCD_DisplayNum(x,y,num,len,size,mode);
	
	LCD_Handle.BrushColor=bh_color;   //���ı�ϵͳ��ɫ
	LCD_Handle.BackGroundColor=bk_color;
}
/****************************************************************************
* ��    ��: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
* ��    �ܣ���ʾ�ַ���
* ��ڲ�����x,y:�������
*           size:�����С
*           *p:�ַ�����ʼ��ַ	
* ���ز�������
* ˵    ����  �B
****************************************************************************/	  
void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
{         
    while((*p<='~')&&(*p>=' ')) //�ж��ǲ��ǷǷ��ַ�!
    {       
        LCD_DisplayChar(x,y,*p,size);
        x+=size/2;
			  if(x>=LCD_Handle.Width) break;
        p++;
    }  
}
/****************************************************************************
* ��    ��: void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p)
* ��    �ܣ���ʾ�Զ����ַ���
* ��ڲ�����x,y:�������
*           width,height:�����С  
*           size:�����С
*           *p:�ַ�����ʼ��ַ	
*           brushcolor���Զ��廭����ɫ
*           backcolor�� �Զ��屳����ɫ
* ���ز�������
* ˵    ����  �B
****************************************************************************/	  
void LCD_DisplayString_color(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint16_t brushcolor,uint16_t backcolor)
{
   uint16_t bh_color,bk_color;
	
 	bh_color=LCD_Handle.BrushColor;  //�ݴ滭����ɫ
	bk_color=LCD_Handle.BackGroundColor;   //�ݴ汳����ɫ
	
	LCD_Handle.BrushColor=brushcolor;
	LCD_Handle.BackGroundColor=backcolor;
	
	LCD_DisplayString(x,y,size,p);
	
	LCD_Handle.BrushColor=bh_color;   //���ı�ϵͳ��ɫ
	LCD_Handle.BackGroundColor=bk_color;
}
