#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.9.30
*** History: 1.0 �������޸���tivaware
             2.0 ΪRTE�����������䣬����ģ������
						 2.1 ����̬��Ϸ�ʽ����
						 2.2 ����RTE_Vec����ͳһ����
*****************************************************************************/
#if HI_USE_SHELL == 1
#define RTE_DEBUG_TXT "[SHELL]"
/*************************************************
*** ����Shell�Ľṹ���������̬����
*************************************************/
static RTE_Shell_Control_t ShellHandle = {0};
/*************************************************
*** Args:   *pcCmdLine ������������
*** Function: Shell�����д���
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CommandProcess(char *pcCmdLine)
{
	char *pcChar;
	uint_fast8_t ui8Argc;
	bool bFindArg = true;
	// Initialize the argument counter, and point to the beginning of the
	// command line string.
	ui8Argc = 0;
	pcChar = pcCmdLine;
	// Advance through the command line until a zero character is found.
	while(*pcChar)
	{
		// If there is a space, then replace it with a zero, and set the flag
		// to search for the next argument.
		if(*pcChar == ' ')
		{
			*pcChar = 0;
			bFindArg = true;
		}
		// Otherwise it is not a space, so it must be a character that is part
		// of an argument.
		else
		{
			// If bFindArg is set, then that means we are looking for the start
			// of the next argument.
			if(bFindArg)
			{
				// As long as the maximum number of arguments has not been
				// reached, then save the pointer to the start of this new arg
				// in the argv array, and increment the count of args, argc.
				if(ui8Argc < HI_SHELL_MAX_ARGS)
				{
						ShellHandle.g_ppcArgv[ui8Argc] = pcChar;
						ui8Argc++;
						bFindArg = false;
				}
				// The maximum number of arguments has been reached so return
				// the error.
				else
				{
						return(SHELL_TOOMANYARGS);
				}
			}
		}
		// Advance to the next character in the command line.
		pcChar++;
	}
	// If one or more arguments was found, then process the command.
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		// If this command entry command string matches argv[0], then call
		// the function for this command, passing the command line
		// arguments.
		if(!ustrcmp(ShellHandle.g_ppcArgv[0], ShellHandle.g_psCmdTable.data[i].pcCmd))
		{
			
			RTE_Shell_Err_e retval;
			if(ShellHandle.g_psCmdTable.data[i].pfnCmdLine !=(void *)0)
			{
				retval = (RTE_Shell_Err_e)ShellHandle.g_psCmdTable.data[i].pfnCmdLine(ui8Argc, ShellHandle.g_ppcArgv);
			}
			return retval;
		}
	}
	return(SHELL_NOVALIDCMD);
}
/*************************************************
*** Args:   *cmd �������ַ���
            *func ������ִ�к���
            *help �����ַ���
*** Function: ����һ��Shell�����д���
*************************************************/
RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,RTE_Shell_Err_e (*func)(int argc, char *argv[]),const char *help)
{
	if(ShellHandle.g_psCmdTable.length >= HI_SHELL_MAX_NUM)
		return SHELL_NOSPACEFORNEW;
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		if(!ustrcmp(cmd,ShellHandle.g_psCmdTable.data[i].pcCmd))
			return SHELL_ALREADYEXIST;
	}
	RTE_Shell_t v;
	v.pcCmd = cmd;
	v.pfnCmdLine = func;
	v.pcHelp = help;
	vec_push(&ShellHandle.g_psCmdTable, v);
	return SHELL_NOERR;
}
/*************************************************
*** Args:   *pcCmdLine ������������
*** Function: ɾ��һ��Shell�����д���
*************************************************/
RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		if(!ustrcmp(cmd,ShellHandle.g_psCmdTable.data[i].pcCmd))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		vec_splice(&ShellHandle.g_psCmdTable, idx, 1);
		return SHELL_NOERR;
	}
	return SHELL_NOSUCHCMD;
}
/*************************************************
*** Args:   NULL
*** Function: shell�Դ��İ�������
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_Help(int argc, char *argv[])
{
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    ����ָ��\r\n",RTE_DEBUG_TXT);
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		RTE_Printf("%10s    ָ����:%16s  ����:%s\r\n", 
			RTE_DEBUG_TXT,
			ShellHandle.g_psCmdTable.data[i].pcCmd,
			ShellHandle.g_psCmdTable.data[i].pcHelp);
	}
	return(SHELL_NOERR);
}
#if RTE_USE_BGET == 1
/*************************************************
*** Args:   NULL
*** Function: shell�Դ�����Ϣ����
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_RTEInfor(int argc, char *argv[])
{
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("RTE�汾�ţ�%s\r\n",RTE_VERSION);
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    ��ǰSHELLָ��ʹ����Ŀ��%d �����Ŀ��%d VEC������%d\r\n",
		RTE_DEBUG_TXT,
		ShellHandle.g_psCmdTable.length,
		HI_SHELL_MAX_NUM,
		ShellHandle.g_psCmdTable.capacity);
	RTE_BGet_Demon();
	RTE_RoundRobin_Demon();
	return(SHELL_NOERR);
}
#endif
/*************************************************
*** Args:   NULL
*** Function: ��ʼ��shell����
*************************************************/
void RTE_Shell_Init(void)
{
	vec_init(&ShellHandle.g_psCmdTable);
	RTE_AssertParam(RTE_Shell_AddCommand("����",RTE_Shell_CMD_Help,"SHELLʹ�ð���") == SHELL_NOERR);
	RTE_AssertParam(RTE_Shell_AddCommand("ϵͳ��Ϣ",RTE_Shell_CMD_RTEInfor,"��ʾRTEϵͳ��Ϣ") == SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: shell��ѯ
*************************************************/
void RTE_Shell_Poll(char *ShellBuffer)
{
	int iStatus;
	iStatus = RTE_Shell_CommandProcess(ShellBuffer);
	if(iStatus == SHELL_NOVALIDCMD)
	{
		RTE_Printf("%10s    ����ʶ���CMD!\r\n",RTE_DEBUG_TXT);
	}
	else if(iStatus == SHELL_TOOMANYARGS)
	{
		RTE_Printf("%10s    ���������Ŀ����!\r\n",RTE_DEBUG_TXT);
	}
	memset(ShellBuffer,0,strlen(ShellBuffer));
}
#endif
