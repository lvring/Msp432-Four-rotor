#include "Board_mpu9250.h"

/*��ͨ�˲�����*/
#define GYRO_LPF_CUTOFF_FREQ  50
#define ACCEL_LPF_CUTOFF_FREQ 30
#define MAG_LPF_CUTOFF_FREQ 30

lpf2pData accLpf[3];
lpf2pData gyroLpf[3];
lpf2pData magLpf[3];



//��ʼ��MPU9250
//����ֵ:0,�ɹ�
//    ����,�������
float magCalibration[3] = {0, 0, 0};
uint8_t MPU9250_Init(void)
{
    uint8_t res=0;
	
    MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X80);//��λMPU9250
    RTE_RoundRobin_DelayMS(100);  //��ʱ100ms
		res=MPU_Read_Byte(MPU9250_ADDR,MPU_DEVICE_ID_REG);  //��ȡMPU6500��ID
		RTE_Printf("MPU6500_ID:%d\r\n",res);
	  if(res != MPU6500_ID) //����ID��ȷ
		{
			RTE_Printf("MPU9250 I2C connection [FAIL].\n");
			return 1;
		}
		else
		{
			RTE_Printf("MPU9250 I2C connection [OK].\n");
		}
		MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X00);//��λMPU9250
		RTE_RoundRobin_DelayMS(100);  //��ʱ100ms
		MPU_Write_Byte(MPU9250_ADDR,MPU_SIGPATH_RST_REG,0X07);//��λGYRO ACCEL TEMP
		RTE_RoundRobin_DelayMS(100);  //��ʱ100ms
		
		MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X01);  	//����CLKSEL,PLL X��Ϊ�ο�
    MPU_Write_Byte(MPU9250_ADDR,MPU_PWR_MGMT2_REG,0X00);  	//���ٶ��������Ƕ�����
		MPU_Write_Byte(MPU9250_ADDR,MPU_CFG_REG,0X03);//���ٶȼ� ��ͨ�˲��� 0x03 41hz MPU6500_ACCEL_DLPF_BW_41  
    MPU_Write_Byte(MPU9250_ADDR,MPU_GYRO_CFG_REG,0X18);//���������������̷�Χ  
		MPU_Write_Byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG,0x10);//���ü��ٶȴ����������̷�Χ  
		
		MPU_Write_Byte(MPU9250_ADDR,MPU_SAMPLE_RATE_REG,0X00);//������������Ϊ1kHz
		MPU_Write_Byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG_2,0x02);//�������������ֵ�ͨ�˲���   98Hz
    MPU_Write_Byte(MPU9250_ADDR,MPU_INT_EN_REG,0X00);   //�ر������ж�
	  MPU_Write_Byte(MPU9250_ADDR,MPU_USER_CTRL_REG,0X00);//I2C��ģʽ�ر�
	  MPU_Write_Byte(MPU9250_ADDR,MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
		MPU_Write_Byte(MPU9250_ADDR,MPU_INTBP_CFG_REG,0X82);//INT���ŵ͵�ƽ��Ч������bypassģʽ������ֱ�Ӷ�ȡ������
//	res=MPU_Read_Byte(AK8963_ADDR,MAG_WIA);    			//��ȡAK8963 ID   
//	RTE_Printf("AK8963 ID:%d\r\n",res);
//	if(res==AK8963_ID)
//	{
//		initAK8963(magCalibration);
////        MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x11);		//����AK8963Ϊ���β���ģʽ
//	}else return 1;
		
		for (uint8_t i = 0; i < 3; i++)// ��ʼ�����ټƺ����ݶ��׵�ͨ�˲�
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
	 RTE_RoundRobin_DelayMS(10);  //��ʱ100ms
	 MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x0f);
	 RTE_RoundRobin_DelayMS(10);  //��ʱ100ms
	 MPU_Read_Len(AK8963_ADDR,AK8963_ASAX,3,&rawData[0]);
	 destination[0] =  (float)(rawData[0] - 128)/256.0f + 1.0f;   // Return x-axis sensitivity adjustment values, etc.
   destination[1] =  (float)(rawData[1] - 128)/256.0f + 1.0f;  
   destination[2] =  (float)(rawData[2] - 128)/256.0f + 1.0f; 
	 MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x00);
	 RTE_RoundRobin_DelayMS(10);  //��ʱ100ms
   MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0x11);		//����AK8963Ϊ���β���ģʽ
	 RTE_RoundRobin_DelayMS(10);  //��ʱ100ms
}




//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
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
//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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
//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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

//�õ�������ֵ(ԭʼֵ)
//mx,my,mz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
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
    MPU_Write_Byte(AK8963_ADDR,MAG_CNTL1,0X11); //AK8963ÿ�ζ����Ժ���Ҫ��������Ϊ���β���ģʽ
    return res;;
}

//IIC����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    uint8_t i;
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //����������ַ+д����
    if(!IIC_Wait_Ack())          //�ȴ�Ӧ��
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //д�Ĵ�����ַ
    IIC_Wait_Ack();             //�ȴ�Ӧ��
    for(i=0;i<len;i++)
    {
        IIC_Send_Byte(buf[i]);  //��������
        if(!IIC_Wait_Ack())      //�ȴ�ACK
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_Stop();
    return 0;
} 

//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{ 
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //����������ַ+д����
    if(!IIC_Wait_Ack())          //�ȴ�Ӧ��
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //д�Ĵ�����ַ
    IIC_Wait_Ack();             //�ȴ�Ӧ��
		IIC_Start();                
    IIC_Send_Byte((addr<<1)|1); //����������ַ+������
    IIC_Wait_Ack();             //�ȴ�Ӧ��
    while(len)
    {
      if(len==1)*buf=IIC_Read_Byte(0);//������,����nACK 
			else *buf=IIC_Read_Byte(1);		//������,����ACK  
			len--;
			buf++;  
    }
    IIC_Stop();                 //����һ��ֹͣ����
    return 0;       
}

//IICдһ���ֽ� 
//devaddr:����IIC��ַ
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
uint8_t MPU_Write_Byte(uint8_t addr,uint8_t reg,uint8_t data)
{
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //����������ַ+д����
    if(!IIC_Wait_Ack())          //�ȴ�Ӧ��
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //д�Ĵ�����ַ
    IIC_Wait_Ack();             //�ȴ�Ӧ��
    IIC_Send_Byte(data);        //��������
    if(!IIC_Wait_Ack())          //�ȴ�ACK
    {
        IIC_Stop();
        return 1;
    }
    IIC_Stop();
    return 0;
}

//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
uint8_t MPU_Read_Byte(uint8_t addr,uint8_t reg)
{
    uint8_t res;
    IIC_Start();
    IIC_Send_Byte((addr<<1)|0); //����������ַ+д����
    IIC_Wait_Ack();             //�ȴ�Ӧ��
    IIC_Send_Byte(reg);         //д�Ĵ�����ַ
    IIC_Wait_Ack();             //�ȴ�Ӧ��
	  IIC_Start();                
    IIC_Send_Byte((addr<<1)|1); //����������ַ+������
    IIC_Wait_Ack();             //�ȴ�Ӧ��
    res=IIC_Read_Byte(0);		//������,����nACK  
    IIC_Stop();                 //����һ��ֹͣ����
    return res;  
}
