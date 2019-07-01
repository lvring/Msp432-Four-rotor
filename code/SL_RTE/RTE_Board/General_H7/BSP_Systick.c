#include "BSP_Systick.h"
void BSP_Systick_Init(void)
{
	SysTick_Config(SystemCoreClock / 1000);	//Set SysTick Timer for 1ms interrupts	
}
#if RTE_USE_OS == 0
void SysTick_Handler(void)
{
	RTE_RoundRobin_TickHandler();
}
#endif
#ifdef USE_HAL_DRIVER
uint32_t HAL_GetTick(void)
{
  return RTE_RoundRobin_GetTick();
}
//note:��ʹ�ò���ϵͳ������µ��ô˺���������ʱ Ϊ������ʽ�ӳ�
void HAL_Delay(uint32_t Delay)
{
	RTE_RoundRobin_DelayMS(Delay);
}
#endif
