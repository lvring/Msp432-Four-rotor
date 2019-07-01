#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.10.02
*** History: 1.0 ����
             2.0 ΪRTE������������
						 2.1 �����˼���������
						 2.2 Ϊ���ӵ�GUI�ں�������
*****************************************************************************/
/*************************************************
*** Args:   *file  ����ʧ�ܵ��ļ�;
            line ����ʧ�ܵ���;
*** Function: ���Թ���
*************************************************/
void RTE_Assert(char *file, uint32_t line)
{ 
	RTE_Printf("[ASSERT]    Wrong parameters value: file %s on line %d\r\n", file, line);
	while (1)
	{
		
	}
}
/*************************************************
*** RTE��������ڴ棬��̬���䣬32λ����
*************************************************/
#if RTE_USE_BGET == 1
RTE_ALIGN_32BYTES (uint8_t RTE_RAM[RTE_MEM_SIZE * 1024]) = {0};
#endif
/*************************************************
*** RTE_Shell�Ļص�����
*************************************************/
#if HI_USE_SHELL == 1
#include "Board_Usart.h"
static void Shell_TimerCallBack(void *Params)
{
	uint8_t *ShellBuffer = RTE_BGetz(MEM_RTE,HI_SHELL_MAX_BUFSIZE);
	uint16_t BufferLenth= 0;
	Board_Usart_Data_t *ShellData;
	ShellData = Board_Usart_ReturnQue(COM_1);
	if(RTE_MessageQuene_Out(&ShellData->ComQuene,(uint8_t *)ShellBuffer,&BufferLenth) == MSG_NO_ERR)
	{
		if(BufferLenth)
		{
			RTE_Shell_Poll((char *)ShellBuffer);
		}
	}
	RTE_BRel(MEM_RTE,ShellBuffer);
}
#endif
/*************************************************
*** Args:   NULL
*** Function: RTE��ʼ��
*************************************************/
void RTE_Init(void)
{
#if RTE_USE_BGET == 1
	RTE_BPool(MEM_RTE,RTE_RAM,RTE_MEM_SIZE*1024);
#endif
#if RTE_USE_ROUNDROBIN == 1
	RTE_RoundRobin_Init();
	RTE_RoundRobin_CreateGroup("RTEGroup");
	#if RTE_USE_HUMMANINTERFACE == 1
		#if HI_USE_SHELL == 1
			RTE_Shell_Init();
			RTE_RoundRobin_CreateTimer(0,"ShellTimer",20,1,1,Shell_TimerCallBack,(void *)0);
		#endif
	#endif
#endif
#if RTE_USE_STDIO != 1
	#if RTE_USE_OS == 1
	static const osMutexAttr_t MutexIDStdioAttr = {
		"StdioMutex",     // human readable mutex name
		0U,    					// attr_bits
		NULL,                // memory for control block   
		0U                   // size for control block
	};
	MutexIDStdio = osMutexNew(&MutexIDStdioAttr);
	#endif
#endif
}
