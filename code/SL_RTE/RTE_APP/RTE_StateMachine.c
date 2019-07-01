#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.9.30
*** History: 1.0 ����
             2.0 ΪRTE�����������䣬����ģ������
						 2.1 �����˲���ϵͳ�����µĻ���������ɾ����
						 2.2 ����RTE_Vec����ͳһ����
*****************************************************************************/
#if RTE_USE_STATEMACHINE == 1
/*************************************************
*** Args:   
					thisStateMachine ������״̬��
					State ״̬���
					thisFunction ״̬����
*** Function: Ϊ״̬���Ĳ�ͬ״̬����״̬����
*************************************************/
RTE_SM_Err_e StateMachine_Add(RTE_StateMachine_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *))
{
	for(uint8_t i = 0;i<thisStateMachine->SMTable.length;i++)
	{
		if(thisStateMachine->SMTable.data[i].StateName == State)
			return SM_ALREADYEXIST;
	}
	RTE_State_t v;
	v.StateName = State;
	v.StateFunction = StateFunction;
	vec_push(&thisStateMachine->SMTable,v);
	return SM_NOERR;
}
/*************************************************
*** Args:   
					thisStateMachine ������״̬��
					InputArgs ״̬���������
*** Function: ����һ��״̬��
*************************************************/
void StateMachine_Run(RTE_StateMachine_t *thisStateMachine,void * InputArgs)
{
	thisStateMachine->RunningState = thisStateMachine->SMTable.data[thisStateMachine->RunningState].StateFunction(InputArgs);
}
/*************************************************
*** Args:   
					thisStateMachine ������״̬��
*** Function: ɾ��һ��״̬��
*************************************************/
RTE_SM_Err_e StateMachine_Remove(RTE_StateMachine_t *thisStateMachine,uint8_t State)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<thisStateMachine->SMTable.length;i++)
	{
		if(thisStateMachine->SMTable.data[i].StateName == State)
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		vec_splice(&thisStateMachine->SMTable, idx, 1);
		return SM_NOERR;
	}
	return SM_NOSUCHSM;
}
/*************************************************
*** Args:   
					thisStateMachine ������״̬��
					StateNum ״̬��Ŀ
*** Function: ��ʼ��һ��״̬��
*************************************************/
void StateMachine_Init(RTE_StateMachine_t *thisStateMachine)
{
	vec_init(&thisStateMachine->SMTable);
}
#endif
