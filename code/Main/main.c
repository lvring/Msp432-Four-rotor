#include "RTE_Include.h"
#include "Board_Usart.h"
#include "Thread_System.h"
void RTE_Puts (const char *pcString,uint16_t length)
{
#if RTE_USE_OS == 1
	osMutexAcquire(MutexIDStdio,osWaitForever);
	Board_Usart_Puts(COM_1,(char *)pcString,length);
	osMutexRelease(MutexIDStdio);
#else
	Board_Usart_Puts(COM_1,(char *)pcString,length);
#endif
}
int main(void)
{

	MAP_Interrupt_enableMaster();
	
	CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
	CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
	CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4 );//SMCLK 12M
	CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	/* Enabling the FPU for floating point operation */
	FPU_enableModule();
	FPU_enableLazyStacking();

#if RTE_USE_OS == 0
	while(1)
	{
		RTE_RoundRobin_Run(0);
	}
#else
	osKernelInitialize();                 // Initialize CMSIS-RTOS
	ThreadIDSYS = osThreadNew(ThreadTaskSYS, NULL, &SystemThreadControl); 

  osKernelStart();                      // Start thread execution
#endif
}

//	    //![Simple CS Config]
//    /* Configuring pins for peripheral/crystal usage and LED for output */
//  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
//            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
//	
//	/* Just in case the user wants to use the getACLK, getMCLK, etc. functions,
//     * let's set the clock frequency in the code. 
//	 */
//	CS_setExternalClockSourceFrequency(32000,6000000);

//	/* Starting HFXT in non-bypass mode without a timeout. Before we start
//	 * we have to change VCORE to 1 to support the 48MHz frequency */
//	PCM_setCoreVoltageLevel(PCM_VCORE1);
//	FlashCtl_setWaitState(FLASH_BANK0, 1);
//	MAP_FlashCtl_setWaitState(FLASH_BANK1, 1);
//	CS_startHFXT(false);
//	SystemCoreClockUpdate();
