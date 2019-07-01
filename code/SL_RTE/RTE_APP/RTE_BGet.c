#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.28
*** History: 1.0 �������޸���BGet��https://www.fourmilab.ch/bget/
             2.0 ΪRTE������������
						 3.0 ��д
*****************************************************************************/
#if RTE_USE_BGET == 1 
#define RTE_DEBUG_TXT "[MEMORY]"
/*************************************************
*** ����BGet�Ľṹ���������̬����
*************************************************/
static RTE_BGet_t BGet_ControlHandle[MEM_N] = 
{
	{
		.MemName = MEM_RTE,
		.work_mem = (void *)0,
		.totalsize = 0,
	}
};
static RTE_BGet_Ent_t  * ent_get_next(RTE_BGet_Name_e mem_name,RTE_BGet_Ent_t * act_e);
static void * ent_alloc(RTE_BGet_Ent_t * e, uint32_t size);
static RTE_BGet_Ent_t * ent_trunc(RTE_BGet_Ent_t * e, uint32_t size);
static uint32_t zero_mem;       /*Give the address of this variable if 0 byte should be allocated*/ 
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
            *buf ʵ��ռ�õ��ڴ��ַ��
            len ʵ��ռ�õ��ڴ��С��
*** Function: ���ݻ�ȡ���ڴ��Լ���С��ʼ�������
*************************************************/
void RTE_BPool(RTE_BGet_Name_e mem_name,void *buf,uint32_t len)
{
	BGet_ControlHandle[mem_name].work_mem = buf;
	BGet_ControlHandle[mem_name].totalsize = len;
	RTE_BGet_Ent_t * full = (RTE_BGet_Ent_t *)BGet_ControlHandle[mem_name].work_mem;
	full->header.used = 0;
	/*The total mem size id reduced by the first header and the close patterns */
	full->header.d_size = len - sizeof(RTE_BGet_Header_t);
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
            requested_size ������ڴ��С��
*** Function: ��BGet�����ڴ�
*************************************************/
void *RTE_BGet(RTE_BGet_Name_e mem_name,uint32_t size)
{
	if(size == 0) {
		return &zero_mem;
	}
	/*Round the size up to 4*/
	if(size & 0x3 ) { 
		size = size & (~0x3);
		size += 4;
	}    
  void * alloc = NULL;
	RTE_BGet_Ent_t * e = NULL;
	//Search for a appropriate entry
	do {
		//Get the next entry
		e = ent_get_next(mem_name,e);
		//If there is next entry then try to allocate there
		if(e != NULL) {
				alloc = ent_alloc(e, size);
		}
	//End if there is not next entry OR the alloc. is successful
	}while(e != NULL && alloc == NULL); 
	return alloc;
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
            requested_size ������ڴ��С��
*** Function: ��BGet�����ڴ棬�����
*************************************************/
void *RTE_BGetz(RTE_BGet_Name_e mem_name,uint32_t size)
{
	void * alloc = NULL;
	alloc = RTE_BGet(mem_name,size);
	if(alloc != NULL) memset(alloc, 0, size);
	return alloc;
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
            *buf ��ǰ������ڴ��ַ��
*** Function: �ͷ���BGet������ڴ�
*************************************************/
void RTE_BRel(RTE_BGet_Name_e mem_name,void *data)
{
	if(data == &zero_mem) return;
	if(data == NULL) return;
	/*e points to the header*/
	RTE_BGet_Ent_t * e = (RTE_BGet_Ent_t *)((uint8_t *) data - sizeof(RTE_BGet_Header_t));
	e->header.used = 0;
#if GUI_MEM_AUTO_DEFRAG == 1
	/* Make a simple defrag.
	 * Join the following free entries after this*/
	RTE_BGet_Ent_t * e_next;
	e_next = ent_get_next(mem_name,e);
	while(e_next != NULL) {
		if(e_next->header.used == 0) {
			e->header.d_size += e_next->header.d_size + sizeof(e->header);
		} else {
			break;
		}
		e_next = ent_get_next(mem_name,e_next);
	}
#endif
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
						*buf ��ǰ������ڴ��ַ
            size ��������ڴ��С��
*** Function: �����µĴ�С���ڴ�ռ�
*************************************************/
void *RTE_BGetr(RTE_BGet_Name_e mem_name,void *data_p,uint32_t new_size)
{
	/*data_p could be previously freed pointer (in this case it is invalid)*/
	if(data_p != NULL) {
		RTE_BGet_Ent_t * e = (RTE_BGet_Ent_t *)((uint8_t *) data_p - sizeof(RTE_BGet_Header_t));
		if(e->header.used == 0) data_p = NULL;
	}
	uint32_t old_size = RTE_BGet_BufSize(data_p);
	if(old_size == new_size) return data_p;
	void * new_p;
	new_p = RTE_BGet(mem_name,new_size);
	if(new_p != NULL && data_p != NULL) {
		/*Copy the old data to the new. Use the smaller size*/
		if(old_size != 0) {
			memcpy(new_p, data_p, MEM_MIN(new_size, old_size));
			RTE_BRel(mem_name,data_p);
		}
	}
	return new_p; 
}
/*************************************************
*** Args:   *buf BGet������ڴ��ַ;
*** Function: ��ȡ�������ڴ��ʵ�ʿռ��С
*************************************************/
uint32_t RTE_BGet_BufSize(void *data)
{
	if(data == NULL) return 0;
	if(data == &zero_mem) return 0;
	RTE_BGet_Ent_t * e = (RTE_BGet_Ent_t *)((uint8_t *) data - sizeof(RTE_BGet_Header_t));
	return e->header.d_size;
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
*** Function: ��Ƭ����
*************************************************/
void RTE_BGet_Defrag(RTE_BGet_Name_e mem_name)
{
	RTE_BGet_Ent_t * e_free;
	RTE_BGet_Ent_t * e_next;
	e_free = ent_get_next(mem_name,NULL);
	while(1) {
		/*Search the next free entry*/
		while(e_free != NULL) {
			if(e_free->header.used != 0) {
					e_free = ent_get_next(mem_name,e_free);
			} else {
					break;
			}
		}
		if(e_free == NULL) return;
		/*Joint the following free entries to the free*/
		e_next = ent_get_next(mem_name,e_free);
		while(e_next != NULL) {
			if(e_next->header.used == 0) {
					e_free->header.d_size += e_next->header.d_size + sizeof(e_next->header);
			} else {
					break;
			}
			e_next = ent_get_next(mem_name,e_next);
		}
		if(e_next == NULL) return;
		/*Continue from the lastly checked entry*/
		e_free = e_next;
	}
}
/*************************************************
*** Args:   mem_name BGet�ƶ���MEMNAME;
*** Function: ��Ϣͳ��
*************************************************/
void RET_BGet_Monitor(RTE_BGet_Name_e mem_name,RTE_BGet_Monitor_t *mon_infor)
{
	/*Init the data*/
	memset(mon_infor, 0, sizeof(RTE_BGet_Monitor_t));
	RTE_BGet_Ent_t * e;
	e = NULL;
	e = ent_get_next(mem_name,e);
	while(e != NULL)  {
		if(e->header.used == 0) {
			mon_infor->cnt_free++;
			mon_infor->size_free += e->header.d_size;
			if(e->header.d_size > mon_infor->size_free_big) {
					mon_infor->size_free_big = e->header.d_size;
			}
		} else {
			mon_infor->cnt_used++;
		}
		e = ent_get_next(mem_name,e);
	}
	mon_infor->size_total = BGet_ControlHandle[mem_name].totalsize;
	mon_infor->pct_used = 100 - (100U * mon_infor->size_free) / mon_infor->size_total;
	mon_infor->pct_frag = (uint32_t)mon_infor->size_free_big * 100U / mon_infor->size_free;
	mon_infor->pct_frag = 100 - mon_infor->pct_frag;
}
/**
 * Give the next entry after 'act_e'
 * @param act_e pointer to an entry
 * @return pointer to an entry after 'act_e'
 */
static RTE_BGet_Ent_t * ent_get_next(RTE_BGet_Name_e mem_name,RTE_BGet_Ent_t * act_e)
{
	RTE_BGet_Ent_t * next_e = NULL;
	if(act_e == NULL) { /*NULL means: get the first entry*/ 
		next_e = (RTE_BGet_Ent_t * ) BGet_ControlHandle[mem_name].work_mem;
	}
	else /*Get the next entry */
	{
		uint8_t * data = &act_e->first_data;
		next_e = (RTE_BGet_Ent_t * )&data[act_e->header.d_size];
		if(&next_e->first_data >= &BGet_ControlHandle[mem_name].work_mem[BGet_ControlHandle[mem_name].totalsize]) 
			next_e = NULL;
	}
	return next_e;
}
/**
 * Try to do the real allocation with a given size
 * @param e try to allocate to this entry
 * @param size size of the new memory in bytes
 * @return pointer to the allocated memory or NULL if not enough memory in the entry
 */
static void * ent_alloc(RTE_BGet_Ent_t * e, uint32_t size)
{
	void * alloc = NULL;
	/*If the memory is free and big enough ten use it */
	if(e->header.used == 0 && e->header.d_size >= size) {
		/*Truncate the entry to the desired size */
		ent_trunc(e, size),
		e->header.used = 1;
		/*Save the allocated data*/
		alloc = &e->first_data;
	}
	return alloc;
}
/**
 * Truncate the data of entry to the given size
 * @param e Pointer to an entry
 * @param size new size in bytes
 * @return the new entry created from the remaining memory
 */
static RTE_BGet_Ent_t * ent_trunc(RTE_BGet_Ent_t * e, uint32_t size)
{
	RTE_BGet_Ent_t * new_e;
	/*Do let empty space  only for a header withot data*/
	if(e->header.d_size == size + sizeof(RTE_BGet_Header_t)) {
			size += sizeof(RTE_BGet_Header_t);
	}
	/* Create the new entry after the current if there is space for it */
	if(e->header.d_size != size) {
		uint8_t * e_data = &e->first_data;
		RTE_BGet_Ent_t * new_e = (RTE_BGet_Ent_t *)&e_data[size];
		new_e->header.used = 0;
		new_e->header.d_size = e->header.d_size - size - sizeof(RTE_BGet_Header_t);
	}
	/* Set the new size for the original entry */
	e->header.d_size = size;
	new_e = e;
	return new_e;
}
void RTE_BGet_Demon(void)
{
	RTE_BGet_Monitor_t mon_infor = {0};
	RET_BGet_Monitor(MEM_RTE,&mon_infor);
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    ��ǰ�ڴ�ʹ����� ����/ȫ��:%d%%/%d ������/����:%d/%d ��Ƭ����:%d%%\r\n",
			RTE_DEBUG_TXT,
			mon_infor.pct_used,
			mon_infor.size_total,
			mon_infor.size_free_big,
			mon_infor.size_free,
			mon_infor.pct_frag);
}
uint32_t RTE_BGet_MaxFree(RTE_BGet_Name_e mem_name)
{
	RTE_BGet_Monitor_t mon_infor = {0};
	RET_BGet_Monitor(mem_name,&mon_infor);
	return mon_infor.size_free_big;
}
#endif
