/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_KEY_H
#define __APP_KEY_H
/* Includes ------------------------------------------------------------------*/
#include "APP_Include.h"
#include "BSP_Include.h"
/* Exported types ------------------------------------------------------------*/
//ÿ��������ID
typedef enum 
{
  KEY_0 = 0,  
	KEY_1 = 1,  
	KEY_2 = 2, 
	KEY_3 = 3, 
	KEY_NUM,
}BSP_KEY_NAME_t;
typedef struct
{
	volatile uint8_t Buf[KEY_POOL_SIZE];													/* ��ֵ������ */
	volatile uint8_t Read;																				/* ��������ָ��1 */
	volatile uint8_t Write;																			/* ������дָ�� */
}BSP_KEY_FIFO_t;	
typedef struct																				//ÿ��������Ӧ1��ȫ�ֵĽṹ�����
{																									
	volatile uint8_t  Count;			                                /* �˲��������� */
	volatile uint16_t LongCount;		                              /* ���������� */
	volatile uint16_t LongTime;		                              	/* �������³���ʱ��, 0��ʾ����ⳤ�� */
	volatile uint8_t  State;			                                /* ������ǰ״̬�����»��ǵ��� */
	volatile uint8_t  RepeatSpeed;	                              /* ������������ */
	volatile uint8_t  RepeatCount;	                              /* �������������� */
}BSP_KEY_SoftHandle_t;
typedef struct {
	BSP_KEY_NAME_t KEY_NAME;    // Name
  GPIO_TypeDef* KEY_PORT; // Port
  const uint16_t KEY_PIN; // Pin
	const uint32_t KEY_CLK; // Clock
	const uint8_t  KEY_PRESS;
	BSP_KEY_SoftHandle_t *KEY_ARG;
}BSP_KEY_HardHandle_t;
//ÿ�������ļ�ֵ
typedef enum                                          //�����ֵ����, ���밴���´���ʱÿ�����İ��¡�����ͳ����¼�
{
	KEY_NONE = 0,			                                  //0 ��ʾ�ް����¼�
	
	KEY_1_DOWN,				                                  /* 1������ */
	KEY_1_UP,				                                    /* 1������ */
	KEY_1_LONG,				                                  /* 1������ */
	
	KEY_2_DOWN,				                                  /* 2������ */
	KEY_2_UP,					                                  /* 2������ */
	KEY_2_LONG,				                                  /* 2������ */

	KEY_3_DOWN,				                                  /* 3������ */
	KEY_3_UP,					                                  /* 3������ */
	KEY_3_LONG,				                                  /* 3������ */

	KEY_4_DOWN,				                                  /* 4������ */
	KEY_4_UP,					                                  /* 4������ */
	KEY_4_LONG,				                                  /* 4������ */

	KEY_5_DOWN,				                                  /* 5������ */
	KEY_5_UP,					                                  /* 5������ */
	KEY_5_LONG,				                                  /* 5������ */
	
}BSP_KEY_State_t;
extern void BSP_KEY_Init(void);
extern BSP_KEY_State_t BSP_KEY_Get(void);
extern void BSP_KEY_Poll(void);
#endif
/****************** (C) COPYRIGHT SuChow University Shannon*****END OF FILE****/
