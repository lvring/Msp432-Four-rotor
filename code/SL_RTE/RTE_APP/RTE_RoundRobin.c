#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.4 2018.10.8
*** History: 1.0 �������޸���tivaware
             2.0 ΪRTE�����������䣬����ģ������
						 2.1 ����̬��Ϸ�ʽ����
						 2.2 ����RTE_Vec����ͳһ����
						 2.3 ���̻߳���������ALONE���ƣ�ȷ��ĳЩ�ؼ�timer�����ڶ������߳�������
						 2.4 ������̶߳�Timer���ƣ���ͬ�̵߳�Timer����ͬ�߳̽��й��������еĲ�ѯtimer����
*****************************************************************************/
#if RTE_USE_ROUNDROBIN == 1
#define RTE_DEBUG_TXT "[RR]"
#include "RTE_Components.h"
#include CMSIS_device_header
/*************************************************
*** ����RoundRobin�Ľṹ���������̬����
*************************************************/
static RTE_RoundRobin_t RoundRobinHandle;
/*************************************************
*** Args:   NULL
*** Function: RoundRobin��ʼ��
*************************************************/
void RTE_RoundRobin_Init(void)
{
	RoundRobinHandle.TimerGroup = (RTE_RoundRobin_TimerGroup_t *)
		RTE_BGetz(MEM_RTE,sizeof(RTE_RoundRobin_TimerGroup_t)*HI_ROUNDROBIN_MAX_GROUP_NUM);
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick = 0;
#endif
	RoundRobinHandle.TimerGroupCnt = 0;
	/* Enable TRC */
	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |=  0x01000000;
	/* Enable counter */
	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |=  0x00000001;
	/* Reset counter */
	DWT->CYCCNT = 0;	
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
}
/*************************************************
*** Args:   NULL
*** Function: RoundRobin TimerGroup��ʼ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_CreateGroup(const char *GroupName)
{
	if(RoundRobinHandle.TimerGroupCnt>=HI_ROUNDROBIN_MAX_GROUP_NUM)
		return RR_NOSPACEFORNEW;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		if(!strcmp(GroupName,RoundRobinHandle.TimerGroup[i].TimerGroupName))
			return RR_ALREADYEXIST;
	}
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupID = RoundRobinHandle.TimerGroupCnt;
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupName = GroupName;
	vec_init(&RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].SoftTimerTable);
	RoundRobinHandle.TimerGroupCnt++;
	return RR_NOERR;
}
/*************************************************
*** Args:   NULL
*** Function: ��ȡRoundRobin TimerGroup ID
*************************************************/
int8_t RTE_RoundRobin_GetGroupID(const char *GroupName)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		if(!ustrcmp(GroupName,RoundRobinHandle.TimerGroup[i].TimerGroupName))
		{
			idx = i;
			break;
		}
	}
	return idx;
}
/*************************************************
*** Args:   
          GroupID ��ʱ������Group��ID
					*TimerName ����Ӷ�ʱ������
					ReloadValue ��װ��ֵ
          ReloadEnable ��װ��ʹ��
          ReloadEnable ��ʱ������ʹ��
          *TimerCallback ��ʱ���ص�����
          *UserParameters �ص������������
*** Function: Ϊ��ǰRoundRobin������ĳһGroup���һ����ʱ��
*************************************************/
extern RTE_RoundRobin_Err_e RTE_RoundRobin_CreateTimer(
	uint8_t GroupID,
	const char *TimerName,
	uint32_t ReloadValue, 
	uint8_t ReloadEnable, 
	uint8_t RunEnable,
	void (*TimerCallback)(void *), 
	void* UserParameters)
{
	if(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length >= HI_ROUNDROBIN_MAX_NUM)
		return RR_NOSPACEFORNEW;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;i++)
	{
		if(!strcmp(TimerName,RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[i].TimerName))
			return RR_ALREADYEXIST;
	}
	RTE_SoftTimer_t v = {0};
	v.TimerID = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;
	v.TimerName = TimerName;
	v.Flags.F.AREN = ReloadEnable;
	v.Flags.F.CNTEN = RunEnable;
	v.ARR = ReloadValue;
	v.CNT = ReloadValue;
	v.Callback = TimerCallback;
	v.UserParameters = UserParameters;
	vec_push(&RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, v);
	return RR_NOERR;
}
/*************************************************
*** Args:   NULL
*** Function: ��ȡRoundRobin Timer ID
*************************************************/
int8_t RTE_RoundRobin_GetTimerID(uint8_t GroupID,const char *TimerName)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;i++)
	{
		if(!ustrcmp(TimerName,RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	return idx;
}
/*************************************************
*** Args:   
					*Name ��ɾ����ʱ������
*** Function: Ϊ��ǰRoundRobin����ɾ��һ����ʱ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_RemoveTimer(uint8_t GroupID,uint8_t TimerID)
{
	vec_splice(&RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID, 1);
	return RR_NOERR;
}

/*************************************************
*** Args:   Timer ������ʱ��
*** Function: SoftTimer��ʱ����
*************************************************/
inline static void RTE_RoundRobin_CheckTimer(uint8_t GroupID,uint8_t TimerID)
{
	/* Check if count is zero */
	if(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT == 0) 
	{
		/* Call user callback function */
		RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Callback(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].UserParameters);
		/* Set new counter value */
		RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].ARR;
		/* Disable timer if auto reload feature is not used */
		if (!RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.AREN)
			/* Disable counter */
			RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 0;
	}
}
/*************************************************
*** Args:   Null
*** Function: RoundRobinʱ������
*************************************************/
void RTE_RoundRobin_TickHandler(void)
{
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick++;
#endif
	// Loop through each group in the group table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroupCnt; i++)
	{
		// Loop through each task in the task table.
		for(uint8_t j = 0; j < RoundRobinHandle.TimerGroup[i].SoftTimerTable.length; j++)
		{
			/* Check if timer is enabled */
			if (RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.CNTEN)  /*!< Timer is enabled */
			{
				/* Decrease counter if needed */
				if (RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT) 
					RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT--;
			}
		}
	}
#if RTE_USE_OS == 1
	RTE_RoundRobin_Run(0);
#endif
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin���к��� �ڷǲ���ϵͳ�����µ���
*************************************************/
void RTE_RoundRobin_Run(uint8_t GroupID)
{
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length; i++)
	{
		RTE_RoundRobin_CheckTimer(GroupID,i);
	}
}
/*************************************************
*** Args:   
					*Name ��������ʱ������
*** Function: ��λ��ǰRoundRobin�����е�һ����ʱ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ReadyTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = 0;
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name ����λ��ʱ������
*** Function: ��λ��ǰRoundRobin�����е�һ����ʱ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ResetTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].ARR;
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name ����ͣ��ʱ������
*** Function: ��ͣ��ǰRoundRobin�����е�һ����ʱ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_PauseTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 0;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name ����ͣ��ʱ������
*** Function: �ָ���ǰRoundRobin�����е�һ����ʱ��
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ResumeTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					Null
*** Function: ��ȡ��ǰRoundRobin������Ϣ
*************************************************/
void RTE_RoundRobin_Demon(void)
{
	RTE_Printf("--------------------------------------------------\r\n");
	for(uint8_t i=0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		RTE_Printf("-------------------------\r\n");
		RTE_Printf("%10s    ��%02d�飺%16s ��ת����TIMERʹ����Ŀ��%d �����Ŀ��%d VEC������%d\r\n",
		RTE_DEBUG_TXT,
		RoundRobinHandle.TimerGroup[i].TimerGroupID,
		RoundRobinHandle.TimerGroup[i].TimerGroupName,
		RoundRobinHandle.TimerGroup[i].SoftTimerTable.length,
		HI_ROUNDROBIN_MAX_NUM,
		RoundRobinHandle.TimerGroup[i].SoftTimerTable.capacity);
		for(uint8_t j=0;j<RoundRobinHandle.TimerGroup[i].SoftTimerTable.length;j++)
		{
			RTE_Printf("%10s    %02d.TIMER:%16s----�Զ����أ�%x ����ֵ��%6d ��ǰֵ��%6d ���У�%x\r\n",
			RTE_DEBUG_TXT,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].TimerID,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].TimerName,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.AREN,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].ARR,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.CNTEN);
		}
	}
}
/*************************************************
*** Args:   
					Null
*** Function: ��ȡ��ǰRoundRobin��������ʱ��
*************************************************/
uint32_t RTE_RoundRobin_GetTick(void) 
{
	/* Return current time in milliseconds */
#if RTE_USE_OS == 1
	if (osKernelGetState () == osKernelRunning) 
	{
    return osKernelGetTickCount();
  }
	else
	{
		static uint32_t ticks = 0U;
					 uint32_t i;
		/* If Kernel is not running wait approximately 1 ms then increment 
			 and return auxiliary tick counter value */
		for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		}
		return ++ticks;
	}
#else
	return RoundRobinHandle.RoundRobinRunTick;
#endif 
}
/*************************************************
*** Args:   
					prev_tick a previous time stamp (return value of systick_get() )
*** Function: ��ȡ����tick֮��ʱ���
*************************************************/
uint32_t RTE_RoundRobin_TickElaps(uint32_t prev_tick)
{
	uint32_t act_time = RTE_RoundRobin_GetTick();
	/*If there is no overflow in sys_time simple subtract*/
	if(act_time >= prev_tick) {
		prev_tick = act_time - prev_tick;
	} else {
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}
	return prev_tick;
}
/*************************************************
*** Args:   Delay
					Null
*** Function: ��ʱһ�κ���
*************************************************/
void RTE_RoundRobin_DelayMS(uint32_t Delay) {
	/* Delay for amount of milliseconds */
	/* Check if we are called from ISR */
	if (__get_IPSR() == 0) {
		/* Called from thread mode */
		uint32_t tickstart = RTE_RoundRobin_GetTick();
		/* Count interrupts */
		while ((RTE_RoundRobin_GetTick() - tickstart) < Delay) {
#if RTE_USE_OS == 0
			/* Go sleep, wait systick interrupt */
			__WFI();
#endif
		}
	} else {
		/* Called from interrupt mode */
		while (Delay) {
			/* Check if timer reached zero after we last checked COUNTFLAG bit */
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
				Delay--;
			}
		}
	}
}
/*************************************************
*** Args:   micros ΢��
*** Function: ��ʱ΢�룬��Ӱ��ϵͳ����
*************************************************/
__inline void RTE_RoundRobin_DelayUS(volatile uint32_t micros) {
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}
#endif
