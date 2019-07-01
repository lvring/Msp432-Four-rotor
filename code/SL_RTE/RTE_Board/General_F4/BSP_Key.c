#include "BSP_Key.h"
static uint8_t BSP_KEY_Press(BSP_KEY_NAME_t KEY_NAME);
static BSP_KEY_SoftHandle_t KeyArg[KEY_NUM] = {
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
};
static BSP_KEY_HardHandle_t Key[KEY_NUM] = {
  // Name    ,PORT , PIN  , PressState ,Arg 
  {KEY_0 , GPIOF  , GPIO_Pin_9 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_1 , GPIOF  , GPIO_Pin_8 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_2 , GPIOF  , GPIO_Pin_7 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_3 , GPIOF  , GPIO_Pin_6 ,RCC_AHB1Periph_GPIOF,  0  , 0},
};
volatile BSP_KEY_FIFO_t KEY_StatePool;		/* ����FIFO����,�ṹ�� */
/*
*********************************************************************************************************
*	�� �� ��: BSP_BSP_PressKey
*	����˵��: �жϰ����Ƿ��¡�
*	��    ��:  KEY_NAME : ��������
*	�� �� ֵ: 1��0
*********************************************************************************************************
*/
static uint8_t BSP_KEY_Press(BSP_KEY_NAME_t KEY_NAME)
{
	if (GPIO_ReadInputDataBit(Key[KEY_NAME].KEY_PORT, Key[KEY_NAME].KEY_PIN) == Key[KEY_NAME].KEY_PRESS)
	{
		return 1;
	}
	else
		return 0;
}
/*
*********************************************************************************************************
*	�� �� ��: BSP_BSP_InitKey
*	����˵��: ��ʼ������Ӳ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BSP_KEY_InitvaluePool(void);
void BSP_KEY_Init(void)
{
	BSP_KEY_InitvaluePool();
  BSP_KEY_NAME_t key_name;
	for(key_name=KEY_0;key_name<KEY_NUM;key_name++)
	{
	  GPIO_InitTypeDef  GPIO_InitStructure;
#ifdef STM32F1
    // Clock Enable
    RCC_APB2PeriphClockCmd(Key[key_name].KEY_CLK, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = Key[key_name].KEY_PIN;
		if(key_name == KEY_3)
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		else
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Key[key_name].KEY_PORT, &GPIO_InitStructure);		
#endif
#ifdef STM32F4
    // Clock Enable
    RCC_AHB1PeriphClockCmd(Key[key_name].KEY_CLK, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = Key[key_name].KEY_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
    GPIO_Init(Key[key_name].KEY_PORT, &GPIO_InitStructure);		
#endif
		if(&KeyArg[key_name]==(void *)0)
			APP_Assert((char *)__FILE__, __LINE__);
		Key[key_name].KEY_ARG = (BSP_KEY_SoftHandle_t *)&KeyArg[key_name];
	}
}
/*
*********************************************************************************************************
*	�� �� ��: BSP_BSP_PutKey
*	����˵��: ��1����ֵѹ�밴��FIFO��������������ģ��һ��������
*	��    ��:  _KeyCode : ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void BSP_KEY_Put(uint8_t _KeyCode)
{
	KEY_StatePool.Buf[KEY_StatePool.Write] = _KeyCode;
	if (++KEY_StatePool.Write  >= KEY_POOL_SIZE)
	{
		KEY_StatePool.Write = 0;
	}
}
/*
*********************************************************************************************************
*	�� �� ��: BSP_BSP_GetKey
*	����˵��: �Ӱ���FIFO��������ȡһ����ֵ��
*	��    ��:  ��
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
BSP_KEY_State_t BSP_KEY_Get(void)
{
	BSP_KEY_State_t ret;
	
	if (KEY_StatePool.Read == KEY_StatePool.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = (BSP_KEY_State_t)KEY_StatePool.Buf[KEY_StatePool.Read];

		if (++KEY_StatePool.Read >= KEY_POOL_SIZE)
		{
			KEY_StatePool.Read = 0;
		}
		return ret;
	}
}

/**************************************************************************************
* FunctionName   : BSP_BSP_InitKeyvaluePoll()
* Description    : ��ֵ����س�ʼ��
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void BSP_KEY_InitvaluePool(void)
{
	/* �԰���FIFO��дָ������ */
	memset((void *)KEY_StatePool.Buf,0,KEY_POOL_SIZE);
	KEY_StatePool.Read = 0;
	KEY_StatePool.Write = 0;
}
/*
*********************************************************************************************************
*	�� �� ��: BSP_BSP_KeyPoll
*	����˵��: ��ⰴ�����С�������״̬�����뱻�����Եĵ��á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BSP_KEY_Poll(void)
{
	BSP_KEY_NAME_t i;
	for (i = KEY_0; i < KEY_NUM ; i++)
	{
		//���û�г�ʼ�����������������˰������
		if (BSP_KEY_Press(i))
		{
			if (Key[i].KEY_ARG->Count < KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count = KEY_FILTER_TIME;
			}
			else if(Key[i].KEY_ARG->Count < 2 * KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count++;
			}
			else
			{
				if (Key[i].KEY_ARG->State == 0)
				{
					Key[i].KEY_ARG->State = 1;

					/* ���Ͱ�ť���µ���Ϣ */
					BSP_KEY_Put((uint8_t)(3 * i + 1));
				}

				if (Key[i].KEY_ARG->LongTime > 0)
				{
					if (Key[i].KEY_ARG->LongCount < Key[i].KEY_ARG->LongTime)
					{
						/* ���Ͱ�ť�������µ���Ϣ */
						if (++Key[i].KEY_ARG->LongCount == Key[i].KEY_ARG->LongTime)
						{
							/* ��ֵ���밴��FIFO */
							BSP_KEY_Put((uint8_t)(3 * i + 3));
						}
					}
					else
					{
						if (Key[i].KEY_ARG->RepeatSpeed > 0)
						{
							if (++Key[i].KEY_ARG->RepeatCount >= Key[i].KEY_ARG->RepeatSpeed)
							{
								Key[i].KEY_ARG->RepeatCount = 0;
								/* ��������ÿ��10ms����1������ */
								BSP_KEY_Put((uint8_t)(3 * i + 3));
							}
						}
					}
				}
			}
		}
		else
		{
			if(Key[i].KEY_ARG->Count > KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count = KEY_FILTER_TIME;
			}
			else if(Key[i].KEY_ARG->Count != 0)
			{
				Key[i].KEY_ARG->Count--;
			}
			else
			{
				if (Key[i].KEY_ARG->State == 1)
				{
					Key[i].KEY_ARG->State = 0;

					/* ���Ͱ�ť�������Ϣ */
					BSP_KEY_Put((uint8_t)(3 * i + 2));
				}
			}

			Key[i].KEY_ARG->LongCount = 0;
			Key[i].KEY_ARG->RepeatCount = 0;
		}
	}
}

