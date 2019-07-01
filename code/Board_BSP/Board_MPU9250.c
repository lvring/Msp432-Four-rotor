#include "Board_mpu9250.h"

/*低通滤波参数*/
#define GYRO_LPF_CUTOFF_FREQ  50
#define ACCEL_LPF_CUTOFF_FREQ 30
#define MAG_LPF_CUTOFF_FREQ 30

lpf2pData accLpf[3];
lpf2pData gyroLpf[3];
lpf2pData magLpf[3];



//初始化MPU9250
//返回值:0,成功
//    其他,错误代码
float magCalibration[3] = {0, 0, 0};
uint8_t MPU9250_Init(void)
{
    uint8_t res=0;
	
    MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X80);//复位MPU9250
    RTE_RoundRobin_DelayMS(100);  //延时100ms
		res=MPU_Read_Byte(MPU9250_ADDR,MPU_DEVICE_ID_REG);  //读取MPU6500的ID
		RTE_Printf("MPU6500_ID:%d\r\n",res);
	  if(res != MPU6500_ID) //器件ID正确
		{
			RTE_Printf("MPU9250 I2C connection [FAIL].\n");
			return 1;
		}
		else
		{
			RTE_Printf("MPU9250 I2C connection [OK].\n");
		}
		MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X00);//复位MPU9250
		RTE_RoundRobin_DelayMS(100);  //延时100ms
		MPU_Write_Byte(MPU9250_ADDR,MPU_SIGPATH_RST_REG,0X07);//复位GYRO ACCEL TEMP
		RTE_RoundRobin_DelayMS(100);  //延时100ms
		
		MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X01);  	//设置CLKSEL,PLL X轴为参考
    MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT2_REG,0X00);  	//加速度与陀螺仪都工作
		MPU_Write_Byte(MPU9250_ADDR,MPU_CFG_REG,0X03);//加速度计 低通滤波器 0x03 41hz MPU6500_ACCEL_DLPF_BW_41  
    MPU_Write_Byte(MPU9250_ADDR,MPU_GYRO_CFG_REG,0X18);//设置陀螺仪满量程范围  
		MPU_Write_Byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG,0x10);//设置加速度传感器满量程范围  
		
		MPU_Write_Byte(MPU9250_ADDR,MPU_SAMPLE_RATE_REG,0X00);//采样数据速率为1kHz
		MPU_Write_Byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG_2,0x02);//设置陀螺仪数字低通滤波器   98Hz
    MPU_Write_Byte(MPU9250_ADDR,MPU_INT_EN_REG,0X00);   //关闭所有中断
	  MPU_Write_Byte(MPU9250_ADDR,MPU_USER_CTRL_REG,0X00);//I2C主模式关闭
	  MPU_Write_Byte(MPU9250_ADDR,MPU_FIFO_EN_REG,0X00);	//关闭FIFO
		MPU_Write_Byte(MPU9250_ADDR,MPU_INTBP_CFG_REG,0X82);//INT引脚低电平有效，开启bypass模式，可以直接读取磁力计
//	res=MPU_Read_Byte(AK8963_ADDR,MAG_WIA);    			//读取AK8963 ID   
//	RTE_Printf("AK8963 ID:%d\r\n",res);
//	if(res==AK8963_ID)
//	{
//		initAK8963(magCalibration);
////        MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x11);		//设置AK8963为单次测量模式
//	}else return 1;
		
		for (uint8_t i = 0; i < 3; i++)// 初始化加速计和陀螺二阶低通滤波
		{
			lpf2pInit(&gyroLpf[i], 1000, GYRO_LPF_CUTOFF_FREQ);
			lpf2pInit(&accLpf[i],  1000, ACCEL_LPF_CUTOFF_FREQ);
			lpf2pInit(&magLpf[i],  1000, MAG_LPF_CUTOFF_FREQ);
		}

		

    return 0;
}

void initAK8963(float * destination)
{
	 uint8_t rawData[3];  // x/y/z gyro calibration data stored here
	 MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x00);
	 RTE_RoundRobin_DelayMS(10);  //延时100ms
	 MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x0f);
	 RTE_RoundRobin_DelayMS(10);  //延时100ms
	 MPU_Read_Len(AK8963_ADDR,AK8963_ASAX,3,&rawData[0]);
	 destination[0] =  (float)(rawData[0] - 128)/256.0f + 1.0f;   // Return x-axis sensitivity adjustment values, etc.
   destination[1] =  (float)(rawData[1] - 128)/256.0f + 1.0f;  
   destination[2] =  (float)(rawData[2] - 128)/256.0f + 1.0f; 
	 MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x00);
	 RTE_RoundRobin_DelayMS(10);  //延时100ms
   MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x11);		//设置AK8963为单次测量模式
	 RTE_RoundRobin_DelayMS(10);  //延时100ms
}




//得到温度值
//返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
    uint8_t buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU9250_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((uint16_t)buf[0]<<8)|buf[1];  
    temp=21+((double)raw)/333.87;  
    return temp*100;;
}
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
  uint8_t buf[6],res; 
	res=MPU_Read_Len(MPU9250_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((uint16_t)buf[0]<<8)|buf[1];  
		*gy=((uint16_t)buf[2]<<8)|buf[3];  
		*gz=((uint16_t)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    uint8_t buf[6],res;  
	res=MPU_Read_Len(MPU9250_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((uint16_t)buf[0]<<8)|buf[1];  
		*ay=((uint16_t)buf[2]<<8)|buf[3];  
		*az=((uint16_t)buf[4]<<8)|buf[5];
	} 	
    return res;;
}

//得到磁力计值(原始值)
//mx,my,mz:磁力计x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Magnetometer(short *mx,short *my,short *mz)
{
  uint8_t buf[6],res;  
	res=MPU_Read_Len(AK8963_ADDR,MAG_XOUT_L,6,buf);
	if(res==0)
	{
		*mx=((uint16_t)buf[1]<<8)|buf[0];  
		*my=((uint16_t)buf[3]<<8)|buf[2];  
		*mz=((uint16_t)buf[5]<<8)|buf[4];
	} 	
    MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0X11); //AK8963每次读完以后都需要重新设置为单次测量模式
    return res;;
}

//IIC连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    uint8_t i;
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    if(!IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
    for(i=0;i<len;i++)
    {
        IIC_Send_Byte(buf[i]);  //发送数据
        if(!IIC_Wait_Ack())      //等待ACK
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_Stop();
    return 0;
} 

//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{ 
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    if(!IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
		IIC_Start();                
    IIC_Send_Byte((addr<<1)|1); //发送器件地址+读命令
    IIC_Wait_Ack();             //等待应答
    while(len)
    {
      if(len==1)*buf=IIC_Read_Byte(0);//读数据,发送nACK 
			else *buf=IIC_Read_Byte(1);		//读数据,发送ACK  
			len--;
			buf++;  
    }
    IIC_Stop();                 //产生一个停止条件
    return 0;       
}

//IIC写一个字节 
//devaddr:器件IIC地址
//reg:寄存器地址
//data:数据
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Write_Byte(uint8_t addr,uint8_t reg,uint8_t data)
{
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    if(!IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
    IIC_Send_Byte(data);        //发送数据
    if(!IIC_Wait_Ack())          //等待ACK
    {
        IIC_Stop();
        return 1;
    }
    IIC_Stop();
    return 0;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
uint8_t MPU_Read_Byte(uint8_t addr,uint8_t reg)
{
    uint8_t res;
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //发送器件地址+写命令
    IIC_Wait_Ack();             //等待应答
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
	  IIC_Start();                
    IIC_Send_Byte((addr<<1)|1); //发送器件地址+读命令
    IIC_Wait_Ack();             //等待应答
    res=IIC_Read_Byte(0);		//读数据,发送nACK  
    IIC_Stop();                 //产生一个停止条件
    return res;  
}
