#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2018.10.09
*** History: 1.0 创建，修改自lvgl：https://littlevgl.com/
*****************************************************************************/
#if RTE_USE_GUI == 1
#define RTE_GUI_REFRESH_DEBUG  0
/*************************************************
*** 重叠管理结构体
*************************************************/
typedef struct
{
	bool reverse;
	uint8_t joined;
	area_t area;
}join_t;
/*************************************************
*** 内部函数
*************************************************/
static void refr_join_area(void);
static void refr_areas(void);
static void refr_area_with_vdb(const area_t * area_p,bool reverse);
static void refr_area_part_vdb(const area_t * area_p,bool reverse);
static obj_t * refr_get_top_obj(const area_t * area_p, obj_t * obj);
static void refr_obj_and_children(obj_t * top_p, const area_t * mask_p);
static void refr_obj(obj_t * obj, const area_t * mask_ori_p);
/*************************************************
*** 内部变量
*************************************************/
static join_t *inv_buf = NULL;          //保存无效区域的缓存(无效意味着需要重新刷新，即刷屏)
static uint16_t inv_buf_p = 0;          //无效区域缓存空闲指针
static void (*monitor_cb)(uint32_t, uint32_t); /*Monitor the rendering time*/
static void (*round_cb)(area_t*);           /*If set then called to modify invalidated areas for special display controllers*/
static uint32_t px_num = 0;
/*************************************************
*** Args:   NULL
*** Function: 刷新一次当前GUI界面
*************************************************/
void GUI_Refresh(void)
{
	uint32_t start = RTE_RoundRobin_GetTick();
	refr_join_area();
	refr_areas();
	bool refr_done = false;
	if(inv_buf_p != 0) 
		refr_done = true;
	memset(inv_buf, 0, RTE_BGet_BufSize(inv_buf));
	inv_buf_p = 0;
	/* In the callback obj_inv can occur
	 * therefore be sure the inv_buf is cleared prior to it*/
	if(refr_done != false) 
	{
		if(monitor_cb != NULL) 
		{
			monitor_cb(RTE_RoundRobin_TickElaps(start), px_num);
		}
	}
}
/*************************************************
*** Args:   NULL
*** Function: 初始化GUI的刷新系统
*************************************************/
void refr_init(void)
{    
	inv_buf = RTE_BGetz(MEM_RTE,GUI_INV_FIFO_SIZE*sizeof(join_t));
	RTE_AssertParam(inv_buf);
}
/*************************************************
*** Args:   
			*area_p 指向一块area的指针
*** Function: 无效化一块area 并保存到无效缓存中
*************************************************/
void inv_area(const area_t * area_p,bool reverse)
{
	/*Clear the invalidate buffer if the parameter is NULL*/
	if(area_p == NULL) {
		inv_buf_p = 0;
		return;
	}
	area_t scr_area;
	scr_area.x1 = 0;
	scr_area.y1 = 0;
	scr_area.x2 = GUI_HOR_RES - 1;
	scr_area.y2 = GUI_VER_RES - 1;
	area_t com_area;    
	bool suc;
	suc = area_union(&com_area, area_p, &scr_area);
	/*The area is truncated to the screen*/
	if(suc != false) 
	{
		if(round_cb) 
			round_cb(&com_area);
		/*Save only if this area is not in one of the saved areas*/
		uint16_t i;
		for(i = 0; i < inv_buf_p; i++) 
		{
			if(area_is_in(&com_area, &inv_buf[i].area) != false) 
				return;
		}
		/*Save the area*/
		if(inv_buf_p < GUI_INV_FIFO_SIZE) 
			area_copy(&inv_buf[inv_buf_p].area,&com_area);
		else 
		{/*If no place for the area add the screen*/
			inv_buf_p = 0;
			area_copy(&inv_buf[inv_buf_p].area,&scr_area);
		}
		inv_buf[inv_buf_p].reverse = reverse;
		inv_buf_p ++;
	}
}
/*************************************************
*** Args:   
			*cb 待设置回调函数
			time_ms: refresh time in [ms]
			px_num: not the drawn pixels but the number of affected pixels of the screen
                   (more pixels are drawn because of overlapping objects)
*** Function: 为monitor设置回调函数
*************************************************/
void refr_set_monitor_cb(void (*cb)(uint32_t, uint32_t))
{
	monitor_cb = cb;
}
/*************************************************
*** Args:   
			*cb 待设置回调函数
			time_ms: refresh time in [ms]
*** Function: 为无效化操作设置回调函数
*************************************************/
void refr_set_round_cb(void(*cb)(area_t*))
{
	round_cb = cb;
}
/*************************************************
*** Args:   
			NULL
*** Function: 获取当前无效缓冲区的大小
*************************************************/
uint16_t refr_get_buf_size(void)
{
	return inv_buf_p;
}
/*************************************************
*** Args:   
			num
*** Function: 删除一个无效区域
*************************************************/
void refr_pop_from_buf(uint16_t num)
{
	if(inv_buf_p < num) 
		inv_buf_p = 0;
	else 
		inv_buf_p -= num;
}
/*************************************************
*** Args:   
			num
*** Function: 把重叠的部分重叠
*************************************************/
static void refr_join_area(void)
{
	uint32_t join_from;
	uint32_t join_in;
	area_t joined_area;
	for(join_in = 0; join_in < inv_buf_p; join_in++) 
	{
		if(inv_buf[join_in].joined != 0) 
			continue;
		/*Check all areas to join them in 'join_in'*/
		for(join_from = 0; join_from < inv_buf_p; join_from++) 
		{
			/*Handle only unjoined areas and ignore itself*/
			if(inv_buf[join_from].joined != 0 || join_in == join_from)
				continue;
			/*Check if the areas are on each other*/
			if(area_is_on(&inv_buf[join_in].area,
										&inv_buf[join_from].area) == false)
				continue;
			area_join(&joined_area, &inv_buf[join_in].area,
															&inv_buf[join_from].area);
			/*Join two area only if the joined area size is smaller*/
			if(area_get_size(&joined_area) < (area_get_size(&inv_buf[join_in].area) + area_get_size(&inv_buf[join_from].area))) 
			{
				area_copy(&inv_buf[join_in].area, &joined_area);
				/*Mark 'join_form' is joined into 'join_in'*/
				inv_buf[join_from].joined = 1;
			}
		}   
	}
}
/*************************************************
*** Args:   
			num
*** Function: 刷新非重叠的部分
*************************************************/
static void refr_areas(void)
{
	px_num = 0;
	uint32_t i;
	for(i = 0; i < inv_buf_p; i++) 
	{
		/*Refresh the unjoined areas*/
		if(inv_buf[i].joined == 0) 
		{
			/*If VDB is used...*/
			refr_area_with_vdb(&inv_buf[i].area,inv_buf[i].reverse);
			if(monitor_cb != NULL) 
				px_num += area_get_size(&inv_buf[i].area);
		}
	}
}
/*************************************************
*** Args:   
			area_p 一块area
*** Function: 刷新一块area到vdb上
*************************************************/
static void refr_area_with_vdb(const area_t * area_p,bool reverse)
{
	/*Calculate the max row num*/
	coord_t w = area_get_width(area_p);
	coord_t h = area_get_height(area_p);
	coord_t y2 = area_p->y2 >= GUI_VER_RES ? y2 = GUI_VER_RES - 1 : area_p->y2;
	uint32_t max_row = (uint32_t) GUI_VDB_SIZE / w;
	if(max_row > h) max_row = h;
	/*Always use the full row*/
	uint32_t row;
	coord_t row_last = 0;
	for(row = area_p->y1; row  + max_row - 1 <= y2; row += max_row)  
	{
		vdb_t * vdb_p = vdb_get();
		/*Calc. the next y coordinates of VDB*/
		vdb_p->area.x1 = area_p->x1;
		vdb_p->area.x2 = area_p->x2;
		vdb_p->area.y1 = row;
		vdb_p->area.y2 = row + max_row - 1;
		if(vdb_p->area.y2 > y2) vdb_p->area.y2 = y2;
		row_last = vdb_p->area.y2;
		refr_area_part_vdb(area_p,reverse);
	}
	/*If the last y coordinates are not handled yet ...*/
	if(y2 != row_last) 
	{
		vdb_t * vdb_p = vdb_get();
		/*Calc. the next y coordinates of VDB*/
		vdb_p->area.x1 = area_p->x1;
		vdb_p->area.x2 = area_p->x2;
		vdb_p->area.y1 = row;
		vdb_p->area.y2 = y2;
		/*Refresh this part too*/
		refr_area_part_vdb(area_p,reverse);
	}
}
/*************************************************
*** Args:   
			area_p 一块area
*** Function: 刷新一块vdb到物理接口上
*************************************************/
static void refr_area_part_vdb(const area_t * area_p,bool reverse)
{
	vdb_t * vdb_p = vdb_get();
	obj_t * top_p;
	/*Get the new mask from the original area and the act. VDB
	 It will be a part of 'area_p'*/
	area_t start_mask;
	area_union(&start_mask, area_p, &vdb_p->area);
	/*Get the most top object which is not covered by others*/
	top_p = refr_get_top_obj(&start_mask, scr_act());
	/*Do the refreshing from the top object*/
	refr_obj_and_children(top_p, &start_mask);
	/*Also refresh top and sys layer unconditionally*/
	refr_obj_and_children(layer_top(), &start_mask);
	refr_obj_and_children(layer_sys(), &start_mask);
	/*Flush the content of the VDB*/ 
	vdb_flush(reverse);
}
/*************************************************
*** Args:   
			area_p* 一块area
			obj_t* 目标obj（一般是GUI的屏幕区）
*** Function: 获取目标obj的一块顶层区域
*************************************************/
static obj_t * refr_get_top_obj(const area_t * area_p, obj_t * obj)
{
	obj_t * i;
	obj_t * found_p = NULL;
	/*If this object is fully cover the draw area check the children too */
	if(area_is_in(area_p, &obj->coords) && obj->hidden == 0)
	{
		LL_READ(obj->child_ll, i)        
		{
			found_p = refr_get_top_obj(area_p, i);
			/*If a children is ok then break*/
			if(found_p != NULL) 
				break;
		}
		/*If no better children check this object*/
		if(found_p == NULL) 
		{
			style_t * style = obj_get_style(obj);
			if(style->body.opa == GUI_OPA_COVER &&
				 obj->design_func(obj, area_p, GUI_DESIGN_COVER_CHK) != false) 
				found_p = obj;
		}
	}
  return found_p;
}
/*************************************************
*** Args:   
			area_p* 掩模area
			obj_t* 目标obj（一般是GUI的屏幕区）
*** Function: 在掩模内刷新目标obj和它的子obj
*************************************************/
static void refr_obj_and_children(obj_t * top_p, const area_t * mask_p)
{
	/* Normally always will be a top_obj (at least the screen)
	 * but in special cases (e.g. if the screen has alpha) it won't.
	 * In this case use the screen directly */
	if(top_p == NULL) 
		top_p = scr_act();
	/*Refresh the top object and its children*/
	refr_obj(top_p, mask_p);
	/*Draw the 'younger' sibling objects because they can be on top_obj */
	obj_t * par;
	obj_t * i;
	obj_t * border_p = top_p;
	par = obj_get_parent(top_p);
	/*Do until not reach the screen*/
	while(par != NULL) 
	{
		/*object before border_p has to be redrawn*/
		i = ll_get_prev(&(par->child_ll), border_p);
		while(i != NULL) 
		{ 
			/*Refresh the objects*/
			refr_obj(i, mask_p);
			i = ll_get_prev(&(par->child_ll), i);
		}
		/*The new border will be there last parents,
		 *so the 'younger' brothers of parent will be refreshed*/
		border_p = par;
		/*Go a level deeper*/
		par = obj_get_parent(par);
  }
	/*Call the post draw design function of the parents of the to object*/
	par = obj_get_parent(top_p);
	while(par != NULL) 
	{
		par->design_func(par, mask_p, GUI_DESIGN_DRAW_POST);
		par = obj_get_parent(par);
	}
}
/*************************************************
*** Args:   
			area_p* 掩模area
			obj_t* 目标obj（一般是GUI的屏幕区）
*** Function: 在掩模内刷新目标obj和它的子obj 用于refr_obj_and_children递归调用
*************************************************/
static void refr_obj(obj_t * obj, const area_t * mask_ori_p)
{
	/*Do not refresh hidden objects*/
	if(obj->hidden != 0) return;
	bool union_ok;  /* Store the return value of area_union */
	/* Truncate the original mask to the coordinates of the parent
	 * because the parent and its children are visible only here */
	area_t obj_mask;
	area_t obj_ext_mask;
	area_t obj_area;
	coord_t ext_size = obj->ext_size;
	obj_get_coords(obj, &obj_area);
	obj_area.x1 -= ext_size;
	obj_area.y1 -= ext_size;
	obj_area.x2 += ext_size;
	obj_area.y2 += ext_size;
	union_ok = area_union(&obj_ext_mask, mask_ori_p, &obj_area);
	/*Draw the parent and its children only if they ore on 'mask_parent'*/
	if(union_ok != false) 
	{
		/* Redraw the object */
		style_t * style = obj_get_style(obj);
		if(style->body.opa != GUI_OPA_TRANSP) 
		{
			obj->design_func(obj, &obj_ext_mask, GUI_DESIGN_DRAW_MAIN);
#if RTE_GUI_REFRESH_DEBUG == 1
			osDelay(100);  /*DEBUG: Wait after every object draw to see the order of drawing*/
#endif
		}
		/*Create a new 'obj_mask' without 'ext_size' because the children can't be visible there*/
		obj_get_coords(obj, &obj_area);
		union_ok = area_union(&obj_mask, mask_ori_p, &obj_area);
		if(union_ok != false) 
		{
			area_t mask_child; /*Mask from obj and its child*/
			obj_t * child_p;
			area_t child_area;
			LL_READ_BACK(obj->child_ll, child_p)
			{
				obj_get_coords(child_p, &child_area);
				ext_size = child_p->ext_size;
				child_area.x1 -= ext_size;
				child_area.y1 -= ext_size;
				child_area.x2 += ext_size;
				child_area.y2 += ext_size;
				/* Get the union (common parts) of original mask (from obj)
				 * and its child */
				union_ok = area_union(&mask_child, &obj_mask, &child_area);
				/*If the parent and the child has common area then refresh the child */
				if(union_ok) 
					/*Refresh the next children*/
					refr_obj(child_p, &mask_child);
			}
		}
		/* If all the children are redrawn make 'post draw' design */
		if(style->body.opa != GUI_OPA_TRANSP) 
		{
			obj->design_func(obj, &obj_ext_mask, GUI_DESIGN_DRAW_POST);
		}
	}
}

#endif
