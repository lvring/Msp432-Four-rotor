#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2018.10.09
*** History: 1.0 创建，修改自lvgl：https://littlevgl.com/
*****************************************************************************/
#if RTE_USE_GUI == 1
/*************************************************
*** VDB的不同状态
*************************************************/
typedef enum {
	GUI_VDB_STATE_FREE = 0,
	GUI_VDB_STATE_ACTIVE,
	GUI_VDB_STATE_FLUSH,
} vdb_state_t;
/*************************************************
*** 管理VDB的结构体变量
*************************************************/
#if GUI_VDB_DOUBLE == 0
   /*Simple VDB*/
   static volatile vdb_state_t vdb_state = GUI_VDB_STATE_ACTIVE;
   static vdb_t vdb = {0};
#else
   /*Double VDB*/
   static volatile vdb_state_t vdb_state[2] = {GUI_VDB_STATE_FREE, GUI_VDB_STATE_FREE};
   static vdb_t vdb[2] = {0,0};
#endif
/*************************************************
*** Args:   NULL
*** Function: 申请用于VDB的内存空间
*************************************************/
void vdb_init(void)
{
#if GUI_VDB_DOUBLE == 0
	vdb.buf = (color_t *)RTE_BGetz(MEM_RTE,GUI_VDB_SIZE*2);
#else
#endif
}
/*************************************************
*** Args:   NULL
*** Function: 获取VDB的内存空间的实际地址
*************************************************/
vdb_t * vdb_get(void)
{
#if GUI_VDB_DOUBLE == 0
	/* Wait until VDB become ACTIVE from FLUSH by the
	 * user call of 'flush_ready()' in display drivers's flush function*/
	while(vdb_state != GUI_VDB_STATE_ACTIVE);
	return &vdb;
#else
	/*If already there is an active do nothing*/
	if(vdb_state[0] == GUI_VDB_STATE_ACTIVE) return &vdb[0];
	if(vdb_state[1] == GUI_VDB_STATE_ACTIVE) return &vdb[1];
	/*Try to allocate a free VDB*/
	if(vdb_state[0] == GUI_VDB_STATE_FREE) {
			vdb_state[0] = GUI_VDB_STATE_ACTIVE;
			return &vdb[0];
	}
	if(vdb_state[1] == GUI_VDB_STATE_FREE) {
			vdb_state[1] = GUI_VDB_STATE_ACTIVE;
			return &vdb[1];
	}
	return NULL;   /*There wasn't free VDB (never happen)*/
#endif
}
/*************************************************
*** Args:   NULL
*** Function: 
				调用硬件接口 把vdb中的内容按照area限制
				输出到物理屏幕上
*************************************************/
void vdb_flush(bool reverse)
{
	vdb_t * vdb_act = vdb_get();
	if(vdb_act == NULL) 
		return;
#if GUI_VDB_DOUBLE == 0
	vdb_state = GUI_VDB_STATE_FLUSH;     /*User call to 'flush_ready()' will set to ACTIVE 'disp_flush'*/
#else
	/* Wait the pending flush before starting this one
	 * (Don't forget: 'flush_ready()' has to be called when flushing is ready)*/
	while(vdb_state[0] == GUI_VDB_STATE_FLUSH || vdb_state[1] == GUI_VDB_STATE_FLUSH);
	/*Turn the active VDB to flushing*/
	if(vdb_state[0] == GUI_VDB_STATE_ACTIVE) vdb_state[0] = GUI_VDB_STATE_FLUSH;
	if(vdb_state[1] == GUI_VDB_STATE_ACTIVE) vdb_state[1] = GUI_VDB_STATE_FLUSH;
#endif
	/*Flush the rendered content to the display*/
	disp_flush(vdb_act->area.x1, vdb_act->area.y1, vdb_act->area.x2, vdb_act->area.y2, vdb_act->buf,reverse);
}
/*************************************************
*** Args:   NULL
*** Function: 
				告知GUI输出完毕
*************************************************/
void flush_ready(void)
{
#if GUI_VDB_DOUBLE == 0
	vdb_state = GUI_VDB_STATE_ACTIVE;
#else
	if(vdb_state[0] == GUI_VDB_STATE_FLUSH)  vdb_state[0] = GUI_VDB_STATE_FREE;
	if(vdb_state[1] == GUI_VDB_STATE_FLUSH)  vdb_state[1] = GUI_VDB_STATE_FREE;
#endif
}
#endif
