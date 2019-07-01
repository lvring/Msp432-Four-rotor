#include "RTE_Include.h"
#if RTE_USE_GUI == 1
/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2018.10.09
*** History: 1.0 创建，修改自lvgl：https://littlevgl.com/
*****************************************************************************/
#define GUI_OBJ_DEF_WIDTH  (GUI_DPI)
#define GUI_OBJ_DEF_HEIGHT  (2 * GUI_DPI / 3)
/*************************************************
*** 内部变量
*************************************************/
static obj_t *def_scr = NULL;
static obj_t *act_scr = NULL;
static obj_t *top_layer = NULL;
static obj_t *sys_layer = NULL;
static ll_t scr_ll = {0};                 /*Linked list of screens*/
/*************************************************
*** 内部函数
*************************************************/
static void refresh_childen_position(obj_t * obj, coord_t x_diff, coord_t y_diff);
static void report_style_mod_core(void * style_p, obj_t * obj);
static void refresh_childen_style(obj_t * obj);
static void delete_children(obj_t * obj);
static bool obj_design(obj_t * obj, const  area_t * mask_p, design_mode_t mode);
static res_t obj_signal(obj_t * obj, signal_t sign, void * param);
/*************************************************
*** Args:   NULL
*** Function: 初始化GUI
*************************************************/
void GUI_Init(void)
{
	vdb_init();
	font_init();
#if GUI_USE_ANIMATION
	anim_init();
#endif
	/*Init. the sstyles*/
	style_init();
	/*Init. the screen refresh system*/
	refr_init();
	/*Create the default screen*/
	ll_init(&scr_ll, sizeof(obj_t));
	def_scr = obj_create(NULL, NULL);
	act_scr = def_scr;
	top_layer = obj_create(NULL, NULL);
	obj_set_style(top_layer, &style_transp_fit);
	sys_layer = obj_create(NULL, NULL);
	obj_set_style(sys_layer, &style_transp_fit);
	/*Refresh the screen*/
	obj_invalidate(act_scr);
#if GUI_INDEV_READ_PERIOD != 0
	/*Init the input device handling*/
	indev_init();
#endif
}
/*--------------------
 * 创建和删除相关
 *-------------------*/
/*************************************************  
* @param parent pointer to a parent object.
*                  If NULL then a screen will be created
* @param copy pointer to a base object, if not NULL then the new object will be copied from it
* @return pointer to the new object
*** Function: 创建一个obj
*************************************************/
obj_t * obj_create(obj_t * parent, obj_t * copy)
{
	obj_t * new_obj = NULL;
	/*Create a screen if the parent is NULL*/
	if(parent == NULL) 
	{
		new_obj = ll_ins_head(&scr_ll);
		new_obj->par = NULL; /*Screens has no a parent*/
		ll_init(&(new_obj->child_ll), sizeof(obj_t));
		/*Set coordinates to full screen size*/
		new_obj->coords.x1 = 0;
		new_obj->coords.y1 = 0;
		new_obj->coords.x2 = GUI_HOR_RES - 1;
		new_obj->coords.y2 = GUI_VER_RES - 1;
		new_obj->ext_size = 0;
		/*Set the default styles*/
		theme_t *th = theme_get_current();
		if(th) 
			new_obj->style_p = th->bg;
		else 
			new_obj->style_p = &style_scr;
		/*Set virtual functions*/
		obj_set_signal_func(new_obj, obj_signal);
		obj_set_design_func(new_obj, obj_design);
		/*Set free data*/
#ifdef GUI_OBJ_FREE_NUM_TYPE
		new_obj->free_num = 0;
#endif
#if GUI_OBJ_FREE_PTR != 0
    new_obj->free_ptr = NULL;
#endif
#if GUI_USE_GROUP
    new_obj->group_p = NULL;
#endif
		/*Set attributes*/
		new_obj->click = 0;
		new_obj->drag = 0;
		new_obj->drag_throw = 0;
		new_obj->drag_parent = 0;
		new_obj->hidden = 0;
		new_obj->top = 0;
    new_obj->protect = GUI_PROTECT_NONE;
		new_obj->ext_attr = NULL;
	}
	/*parent != NULL create normal obj. on a parent*/
	else
	{   
		new_obj = ll_ins_head(&(parent)->child_ll);

		new_obj->par = parent; /*Set the parent*/
		ll_init(&(new_obj->child_ll), sizeof(obj_t));
		
		/*Set coordinates left top corner of parent*/
		new_obj->coords.x1 = parent->coords.x1;
		new_obj->coords.y1 = parent->coords.y1;
		new_obj->coords.x2 = parent->coords.x1 +
															 GUI_OBJ_DEF_WIDTH;
		new_obj->coords.y2 = parent->coords.y1 +
															 GUI_OBJ_DEF_HEIGHT;
		new_obj->ext_size = 0;
		/*Set appearance*/
		new_obj->style_p = &style_plain_color;
		/*Set virtual functions*/
		obj_set_signal_func(new_obj, obj_signal);
		obj_set_design_func(new_obj, obj_design);
    /*Set free data*/
#ifdef GUI_OBJ_FREE_NUM_TYPE
		new_obj->free_num = 0;
#endif
#if GUI_OBJ_FREE_PTR != 0
		new_obj->free_ptr = NULL;
#endif
#if GUI_USE_GROUP
		new_obj->group_p = NULL;
#endif
		/*Set attributes*/
		new_obj->click = 1;
		new_obj->drag = 0;
		new_obj->drag_throw = 0;
		new_obj->drag_parent = 0;
		new_obj->hidden = 0;
		new_obj->top = 0;
		new_obj->protect = GUI_PROTECT_NONE;
		
		new_obj->ext_attr = NULL;
  }
	if(copy != NULL) 
	{
		area_copy(&new_obj->coords, &copy->coords);
		new_obj->ext_size = copy->ext_size;
		/*Set free data*/
#ifdef GUI_OBJ_FREE_NUM_TYPE
		new_obj->free_num = copy->free_num;
#endif
#if GUI_OBJ_FREE_PTR != 0
		new_obj->free_ptr = copy->free_ptr;
#endif
	/*Set attributes*/
		new_obj->click = copy->click;
		new_obj->drag = copy->drag;
		new_obj->drag_throw = copy->drag_throw;
		new_obj->drag_parent = copy->drag_parent;
		new_obj->hidden = copy->hidden;
		new_obj->top = copy->top;
		new_obj->protect = copy->protect;

		new_obj->style_p = copy->style_p;

#if GUI_USE_GROUP
		/*Add to the same group*/
		if(copy->group_p != NULL) 
			group_add_obj(copy->group_p, new_obj);
#endif
		obj_set_pos(new_obj, obj_get_x(copy), obj_get_y(copy));
	}
  /*Send a signal to the parent to notify it about the new child*/
	if(parent != NULL) 
	{
		parent->signal_func(parent, GUI_SIGNAL_CHILD_CHG, new_obj);
		/*Invalidate the area if not screen created*/
		obj_invalidate(new_obj);
	}
  return new_obj;
}
/*************************************************  
 * @param obj pointer to an object to delete
 * @return GUI_RES_INV because the object is deleted
*** Function: 删除一个obj
*************************************************/
res_t obj_del(obj_t * obj)
{
    obj_invalidate(obj);
    
    /*Recursively delete the children*/
    obj_t * i;
    obj_t * i_next;
    i = ll_get_head(&(obj->child_ll));
    while(i != NULL) {
        /*Get the next object before delete this*/
        i_next = ll_get_next(&(obj->child_ll), i);
        
        /*Call the recursive del to the child too*/
        delete_children(i);
        
        /*Set i to the next node*/
        i = i_next;
    }
#if GUI_USE_ANIMATION
    /*Remove the animations from this object*/
    anim_del(obj, NULL);
#endif
    
    /*Delete from the group*/
 #if GUI_USE_GROUP
    if(obj->group_p != NULL) group_remove_obj(obj);
 #endif

    /* Reset all input devices if
     * the currently pressed object is deleted*/
    indev_t * indev = indev_next(NULL);
    while(indev) {
        if(indev->proc.act_obj == obj || indev->proc.last_obj == obj) {
            indev_reset(indev);
        }
        indev = indev_next(indev);
    }

    /*Remove the object from parent's children list*/
    obj_t * par = obj_get_parent(obj);
    if(par == NULL) { /*It is a screen*/
    	ll_rem(&scr_ll, obj);
    } else {
    	ll_rem(&(par->child_ll), obj);
    }

    /* All children deleted.
     * Now clean up the object specific data*/
    obj->signal_func(obj, GUI_SIGNAL_CLEANUP, NULL);

    /*Delete the base objects*/
    if(obj->ext_attr != NULL)  RTE_BRel(MEM_RTE,obj->ext_attr);
    RTE_BRel(MEM_RTE,obj); /*Free the object itself*/

    /*Send a signal to the parent to notify it about the child delete*/
    if(par != NULL) {
    	par->signal_func(par, GUI_SIGNAL_CHILD_CHG, NULL);
    }

    return GUI_RES_INV;
}

/**
 * Delete all children of an object
 * @param obj pointer to an object
 */
void obj_clean(obj_t *obj)
{
    obj_t *child = obj_get_child(obj, NULL);

    while(child) {
        obj_del(child);
        child = obj_get_child(obj, child);
    }
}

/**
 * Mark the object as invalid therefore its current position will be redrawn by 'refr_task'
 * @param obj pointer to an object
 */
void obj_invalidate(obj_t * obj)
{
	if(obj_get_hidden(obj)) return;

    /*Invalidate the object only if it belongs to the 'act_scr'*/
    obj_t * obj_scr = obj_get_screen(obj);
    if(obj_scr == scr_act() ||
       obj_scr == layer_top() ||
       obj_scr == layer_sys())
    {
        /*Truncate recursively to the parents*/
        area_t area_trunc;
        obj_t * par = obj_get_parent(obj);
        bool union_ok = true;
        /*Start with the original coordinates*/
        coord_t ext_size = obj->ext_size;
        area_copy(&area_trunc, &obj->coords);
        area_trunc.x1 -= ext_size;
        area_trunc.y1 -= ext_size;
        area_trunc.x2 += ext_size;
        area_trunc.y2 += ext_size;

        /*Check through all parents*/
        while(par != NULL) {
            union_ok = area_union(&area_trunc, &area_trunc, &par->coords);
            if(union_ok == false) break; 		/*If no common parts with parent break;*/
        	if(obj_get_hidden(par)) return;	/*If the parent is hidden then the child is hidden and won't be drawn*/

            par = obj_get_parent(par);
        }

        if(union_ok != false) inv_area(&area_trunc,true);
    }
}


/*=====================
 * Setter functions 
 *====================*/

/*--------------
 * Screen set 
 *--------------*/

/**
 * Load a new screen
 * @param scr pointer to a screen
 */
void scr_load(obj_t * scr)
{
    act_scr = scr;
    
    obj_invalidate(act_scr);
}

/*--------------------
 * Parent/children set 
 *--------------------*/

/**
 * Set a new parent for an object. Its relative position will be the same.
 * @param obj pointer to an object
 * @param parent pointer to the new parent object
 */
void obj_set_parent(obj_t * obj, obj_t * parent)
{
    obj_invalidate(obj);
    
    point_t old_pos;
    old_pos.x = obj_get_x(obj);
    old_pos.y = obj_get_y(obj);
    
    obj_t * old_par = obj->par;

    ll_chg_list(&obj->par->child_ll, &parent->child_ll, obj);
    obj->par = parent;
    obj_set_pos(obj, old_pos.x, old_pos.y);

    /*Notify the original parent because one of its children is lost*/
    old_par->signal_func(old_par, GUI_SIGNAL_CHILD_CHG, NULL);

    /*Notify the new parent about the child*/
    parent->signal_func(parent, GUI_SIGNAL_CHILD_CHG, obj);

    obj_invalidate(obj);
}

/*--------------------
 * Coordinate set
 * ------------------*/

/**
 * Set relative the position of an object (relative to the parent)
 * @param obj pointer to an object
 * @param x new distance from the left side of the parent
 * @param y new distance from the top of the parent
 */
void obj_set_pos(obj_t * obj, coord_t x, coord_t y)
{
    /*Convert x and y to absolute coordinates*/
    obj_t * par = obj->par;
    x = x + par->coords.x1;
    y = y + par->coords.y1;
    
    /*Calculate and set the movement*/
    point_t diff;
    diff.x =  x - obj->coords.x1;
    diff.y =  y - obj->coords.y1;

    /* Do nothing if the position is not changed */
    /* It is very important else recursive positioning can
     * occur without position change*/
    if(diff.x == 0 && diff.y == 0) return;
        
    /*Invalidate the original area*/
    obj_invalidate(obj);

    /*Save the original coordinates*/
    area_t ori;
    obj_get_coords(obj, &ori);

    obj->coords.x1 += diff.x;
    obj->coords.y1 += diff.y;
    obj->coords.x2 += diff.x;
    obj->coords.y2 += diff.y;
    
    refresh_childen_position(obj, diff.x, diff.y);
    
    /*Inform the object about its new coordinates*/
    obj->signal_func(obj, GUI_SIGNAL_CORD_CHG, &ori);
    
    /*Send a signal to the parent too*/
    par->signal_func(par, GUI_SIGNAL_CHILD_CHG, obj);
    
    /*Invalidate the new area*/
    obj_invalidate(obj);
}


/**
 * Set the x coordinate of a object
 * @param obj pointer to an object
 * @param x new distance from the left side from the parent
 */
void obj_set_x(obj_t * obj, coord_t x)
{
    obj_set_pos(obj, x, obj_get_y(obj));
}


/**
 * Set the y coordinate of a object
 * @param obj pointer to an object
 * @param y new distance from the top of the parent
 */
void obj_set_y(obj_t * obj, coord_t y)
{
    obj_set_pos(obj, obj_get_x(obj), y);
}

/**
 * Set the size of an object
 * @param obj pointer to an object
 * @param w new width 
 * @param h new height
 */
void obj_set_size(obj_t * obj, coord_t w, coord_t h)
{

    /* Do nothing if the size is not changed */
    /* It is very important else recursive resizing can
     * occur without size change*/
	if(obj_get_width(obj) == w && obj_get_height(obj) == h) {
		return;
	}

    /*Invalidate the original area*/
    obj_invalidate(obj);
    
    /*Save the original coordinates*/
    area_t ori;
    obj_get_coords(obj, &ori);
    
    //Set the length and height
    obj->coords.x2 = obj->coords.x1 + w - 1;
    obj->coords.y2 = obj->coords.y1 + h - 1;


    /*Send a signal to the object with its new coordinates*/
    obj->signal_func(obj, GUI_SIGNAL_CORD_CHG, &ori);
    
    /*Send a signal to the parent too*/
    obj_t * par = obj_get_parent(obj);
    if(par != NULL) par->signal_func(par, GUI_SIGNAL_CHILD_CHG, obj);
    
    /*Invalidate the new area*/
    obj_invalidate(obj);
}

/**
 * Set the width of an object
 * @param obj pointer to an object
 * @param w new width
 */
void obj_set_width(obj_t * obj, coord_t w)
{
    obj_set_size(obj, w, obj_get_height(obj));
}

/**
 * Set the height of an object
 * @param obj pointer to an object
 * @param h new height
 */
void obj_set_height(obj_t * obj, coord_t h)
{
    obj_set_size(obj, obj_get_width(obj), h);
}

/**
 * Align an object to an other object. 
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'align_t' enum)
 * @param x_mod x coordinate shift after alignment
 * @param y_mod y coordinate shift after alignment
 */
void obj_align(obj_t * obj,obj_t * base, align_t align, coord_t x_mod, coord_t y_mod)
{
    coord_t new_x = obj_get_x(obj);
    coord_t new_y = obj_get_y(obj);

    if(base == NULL) {
        base = obj_get_parent(obj);
    }
    
    switch(align)
    {
        case GUI_ALIGN_CENTER:
            new_x = obj_get_width(base) / 2 - obj_get_width(obj) / 2;
            new_y = obj_get_height(base) / 2 - obj_get_height(obj) / 2;
            break;

        case GUI_ALIGN_IN_TOP_LEFT:
            new_x = 0;
            new_y = 0;
        	break;
        case GUI_ALIGN_IN_TOP_MID:
            new_x = obj_get_width(base) / 2 - obj_get_width(obj) / 2;
            new_y = 0;
            break;

        case GUI_ALIGN_IN_TOP_RIGHT:
            new_x = obj_get_width(base) - obj_get_width(obj);
            new_y = 0;
            break;

        case GUI_ALIGN_IN_BOTTOM_LEFT:
            new_x = 0;
            new_y = obj_get_height(base) - obj_get_height(obj);
        	break;
        case GUI_ALIGN_IN_BOTTOM_MID:
            new_x = obj_get_width(base) / 2 - obj_get_width(obj) / 2;
            new_y = obj_get_height(base) - obj_get_height(obj);
            break;

        case GUI_ALIGN_IN_BOTTOM_RIGHT:
            new_x = obj_get_width(base) - obj_get_width(obj);
            new_y = obj_get_height(base) - obj_get_height(obj);
            break;

        case GUI_ALIGN_IN_LEFT_MID:
            new_x = 0;
            new_y = obj_get_height(base) / 2 - obj_get_height(obj) / 2;
        	break;

        case GUI_ALIGN_IN_RIGHT_MID:
            new_x = obj_get_width(base) - obj_get_width(obj);
            new_y = obj_get_height(base) / 2 - obj_get_height(obj) / 2;
        	break;

        case GUI_ALIGN_OUT_TOP_LEFT:
            new_x = 0;
            new_y = -obj_get_height(obj);
        	break;

        case GUI_ALIGN_OUT_TOP_MID:
            new_x = obj_get_width(base) / 2 - obj_get_width(obj) / 2;
            new_y = - obj_get_height(obj);
        	break;

        case GUI_ALIGN_OUT_TOP_RIGHT:
            new_x = obj_get_width(base) - obj_get_width(obj);
            new_y = - obj_get_height(obj);
        	break;

        case GUI_ALIGN_OUT_BOTTOM_LEFT:
            new_x = 0;
            new_y = obj_get_height(base);
        	break;

        case GUI_ALIGN_OUT_BOTTOM_MID:
            new_x = obj_get_width(base) / 2 - obj_get_width(obj) / 2;
            new_y = obj_get_height(base);
        	break;

        case GUI_ALIGN_OUT_BOTTOM_RIGHT:
            new_x = obj_get_width(base) - obj_get_width(obj);
            new_y = obj_get_height(base);
        	break;

        case GUI_ALIGN_OUT_LEFT_TOP:
            new_x = - obj_get_width(obj);
            new_y = 0;
        	break;

        case GUI_ALIGN_OUT_LEFT_MID:
            new_x = - obj_get_width(obj);
            new_y = obj_get_height(base) / 2 - obj_get_height(obj) / 2;
        	break;

        case GUI_ALIGN_OUT_LEFT_BOTTOM:
            new_x = - obj_get_width(obj);
            new_y = obj_get_height(base) - obj_get_height(obj);
        	break;

        case GUI_ALIGN_OUT_RIGHT_TOP:
            new_x = obj_get_width(base);
            new_y = 0;
        	break;

        case GUI_ALIGN_OUT_RIGHT_MID:
            new_x = obj_get_width(base);
            new_y = obj_get_height(base) / 2 - obj_get_height(obj) / 2;
        	break;

        case GUI_ALIGN_OUT_RIGHT_BOTTOM:
            new_x = obj_get_width(base);
            new_y = obj_get_height(base) - obj_get_height(obj);
        	break;
    }

    /*Bring together the coordination system of base and obj*/
    obj_t * par = obj_get_parent(obj);
    coord_t base_abs_x = base->coords.x1;
    coord_t base_abs_y = base->coords.y1;
    coord_t par_abs_x = par->coords.x1;
    coord_t par_abs_y = par->coords.y1;
    new_x += x_mod + base_abs_x;
    new_y += y_mod + base_abs_y;
	new_x -= par_abs_x;
	new_y -= par_abs_y;

	obj_set_pos(obj, new_x, new_y);
}

/*---------------------
 * Appearance set 
 *--------------------*/

/**
 * Set a new style for an object
 * @param obj pointer to an object
 * @param style_p pointer to the new style
 */
void obj_set_style(obj_t * obj, style_t * style)
{
    obj->style_p = style;

    /*Send a signal about style change to every children with NULL style*/
    refresh_childen_style(obj);

    /*Notify the object about the style change too*/
    obj_refresh_style(obj);

}

/**
 * Notify an object about its style is modified
 * @param obj pointer to an object
 */
void obj_refresh_style(obj_t * obj)
{
    obj_invalidate(obj);
    obj->signal_func(obj, GUI_SIGNAL_STYLE_CHG, NULL);
    obj_invalidate(obj);

}

/**
 * Notify all object if a style is modified
 * @param style pointer to a style. Only the objects with this style will be notified
 *               (NULL to notify all objects)
 */
void obj_report_style_mod(style_t * style)
{
    obj_t * i;
    LL_READ(scr_ll, i) {
        report_style_mod_core(style, i);
    }
}

/*-----------------
 * Attribute set
 *----------------*/

/**
 * Hide an object. It won't be visible and clickable.
 * @param obj pointer to an object
 * @param en true: hide the object
 */
void obj_set_hidden(obj_t * obj, bool en)
{
	if(!obj->hidden) obj_invalidate(obj);	/*Invalidate when not hidden (hidden objects are ignored) */

    obj->hidden = en == false ? 0 : 1;

	if(!obj->hidden) obj_invalidate(obj);	/*Invalidate when not hidden (hidden objects are ignored) */

    obj_t * par = obj_get_parent(obj);
    par->signal_func(par, GUI_SIGNAL_CHILD_CHG, obj);

}

/**
 * Enable or disable the clicking of an object
 * @param obj pointer to an object
 * @param en true: make the object clickable
 */
void obj_set_click(obj_t * obj, bool en)
{
    obj->click = (en == true ? 1 : 0);
}

/**
 * Enable to bring this object to the foreground if it
 * or any of its children is clicked
 * @param obj pointer to an object
 * @param en true: enable the auto top feature
 */
void obj_set_top(obj_t * obj, bool en)
{
    obj->top = (en == true ? 1 : 0);
}

/**
 * Enable the dragging of an object
 * @param obj pointer to an object
 * @param en true: make the object dragable
 */
void obj_set_drag(obj_t * obj, bool en)
{
    if(en == true) obj_set_click(obj, true);     /*Drag is useless without enabled clicking*/
    obj->drag = (en == true ? 1 : 0);
}

/**
 * Enable the throwing of an object after is is dragged
 * @param obj pointer to an object
 * @param en true: enable the drag throw
 */
void obj_set_drag_throw(obj_t * obj, bool en)
{
    obj->drag_throw = (en == true ? 1 : 0);
}

/**
 * Enable to use parent for drag related operations. 
 * If trying to drag the object the parent will be moved instead 
 * @param obj pointer to an object
 * @param en true: enable the 'drag parent' for the object
 */
void obj_set_drag_parent(obj_t * obj, bool en)
{
    obj->drag_parent = (en == true ? 1 : 0);
}

/**
 * Set a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from obj_prot_t
 */
void obj_set_protect(obj_t * obj, uint8_t prot)
{
    obj->protect |= prot;
}

/**
 * Clear a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from obj_prot_t
 */
void obj_clear_protect(obj_t * obj, uint8_t prot)
{
    prot = (~prot) & 0xFF;
    obj->protect &= prot;
}

/**
 * Set the signal function of an object. 
 * Always call the previous signal function in the new.
 * @param obj pointer to an object
 * @param fp the new signal function
 */
void obj_set_signal_func(obj_t * obj, signal_func_t fp)
{
    obj->signal_func = fp;
}

/**
 * Set a new design function for an object
 * @param obj pointer to an object
 * @param fp the new design function
 */
void obj_set_design_func(obj_t * obj, design_func_t fp)
{
    obj->design_func = fp;
}

/*----------------
 * Other set
 *--------------*/

/**
 * Allocate a new ext. data for an object
 * @param obj pointer to an object
 * @param ext_size the size of the new ext. data
 * @return Normal pointer to the allocated ext
 */
void * obj_allocate_ext_attr(obj_t * obj, uint16_t ext_size)
{
   obj->ext_attr = RTE_BGetr(MEM_RTE,obj->ext_attr, ext_size);
    
   return (void*)obj->ext_attr;
}

/**
 * Send a 'GUI_SIGNAL_REFR_EXT_SIZE' signal to the object
 * @param obj pointer to an object
 */
void obj_refresh_ext_size(obj_t * obj)
{
	obj->ext_size = 0;
	obj->signal_func(obj, GUI_SIGNAL_REFR_EXT_SIZE, NULL);

	obj_invalidate(obj);
}

#ifdef GUI_OBJ_FREE_NUM_TYPE
/**
 * Set an application specific number for an object.
 * It can help to identify objects in the application. 
 * @param obj pointer to an object
 * @param free_num the new free number
 */
void obj_set_free_num(obj_t * obj, GUI_OBJ_FREE_NUM_TYPE free_num)
{
    obj->free_num = free_num;
}
#endif

#if GUI_OBJ_FREE_PTR != 0
/**
 * Set an application specific  pointer for an object.
 * It can help to identify objects in the application.
 * @param obj pointer to an object
 * @param free_p the new free pinter
 */
void obj_set_free_ptr(obj_t * obj, void * free_p)
{
    obj->free_ptr = free_p;
}
#endif

#if GUI_USE_ANIMATION
/**
 * Animate an object
 * @param obj pointer to an object to animate
 * @param type type of animation from 'anim_builtin_t'. 'OR' it with ANIM_IN or ANIM_OUT
 * @param time time of animation in milliseconds
 * @param delay delay before the animation in milliseconds
 * @param cb a function to call when the animation is ready
 */
void obj_animate(obj_t * obj, anim_builtin_t type, uint16_t time, uint16_t delay, void (*cb) (obj_t *))
{
	obj_t * par = obj_get_parent(obj);

	/*Get the direction*/
	bool out = (type & GUI_ANIM_DIR_MASK) == GUI_ANIM_IN ? false : true;
	type = (anim_builtin_t)(type & (~GUI_ANIM_DIR_MASK));

	anim_t a;
	a.var = obj;
	a.time = time;
	a.act_time = (int32_t)-delay;
	a.end_cb = (void(*)(void*))cb;
	a.path = anim_path_linear;
	a.playback_pause = 0;
	a.repeat_pause = 0;
	a.playback = 0;
	a.repeat = 0;

	/*Init to ANIM_IN*/
	switch(type) {
		case GUI_ANIM_FLOAT_LEFT:
			a.fp = (void(*)(void *, int32_t))obj_set_x;
			a.start = -obj_get_width(obj);
			a.end = obj_get_x(obj);
			break;
		case GUI_ANIM_FLOAT_RIGHT:
			a.fp = (void(*)(void *, int32_t))obj_set_x;
			a.start = obj_get_width(par);
			a.end = obj_get_x(obj);
			break;
		case GUI_ANIM_FLOAT_TOP:
			a.fp = (void(*)(void * , int32_t))obj_set_y;
			a.start = -obj_get_height(obj);
			a.end = obj_get_y(obj);
			break;
		case GUI_ANIM_FLOAT_BOTTOM:
			a.fp = (void(*)(void * , int32_t))obj_set_y;
			a.start = obj_get_height(par);
			a.end = obj_get_y(obj);
			break;
		case GUI_ANIM_GROW_H:
			a.fp = (void(*)(void * , int32_t))obj_set_width;
			a.start = 0;
			a.end = obj_get_width(obj);
			break;
		case GUI_ANIM_GROW_V:
			a.fp = (void(*)(void * , int32_t))obj_set_height;
			a.start = 0;
			a.end = obj_get_height(obj);
			break;
        case GUI_ANIM_NONE:
            a.fp = NULL;
            a.start = 0;
            a.end = 0;
            break;
		default:
			break;
	}

	/*Swap start and end in case of ANIM OUT*/
	if(out != false) {
		int32_t tmp = a.start;
		a.start = a.end;
		a.end = tmp;
	}

	anim_create(&a);
}

#endif

/*=======================
 * Getter functions
 *======================*/

/*------------------
 * Screen get
 *-----------------*/

/**
 * Return with a pointer to the active screen
 * @return pointer to the active screen object (loaded by 'scr_load()')
 */
obj_t * scr_act(void)
{
    return act_scr;
}

/**
 * Return with the top layer. (Same on every screen and it is above the normal screen layer)
 * @return pointer to the top layer object  (transparent screen sized obj)
 */
obj_t * layer_top(void)
{
    return top_layer;
}

/**
 * Return with the system layer. (Same on every screen and it is above the all other layers)
 * It is used for example by the cursor
 * @return pointer to the system layer object (transparent screen sized obj)
 */
obj_t * layer_sys(void)
{
    return sys_layer;
}

/**
 * Return with the screen of an object
 * @param obj pointer to an object
 * @return pointer to a screen
 */
obj_t * obj_get_screen(obj_t * obj)
{
    obj_t * par = obj;
    obj_t * act_p;
    
    do {
        act_p = par;
        par = obj_get_parent(act_p);
    }
    while(par != NULL);
    
    return act_p;
}

/*---------------------
 * Parent/children get
 *--------------------*/

/**
 * Returns with the parent of an object
 * @param obj pointer to an object
 * @return pointer to the parent of  'obj' 
 */
obj_t * obj_get_parent(obj_t * obj)
{
    return obj->par;
}

/**
 * Iterate through the children of an object (start from the "youngest")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
obj_t * obj_get_child(obj_t * obj, obj_t * child)
{
	if(child == NULL) {
		return ll_get_head(&obj->child_ll);
	} else {
		return ll_get_next(&obj->child_ll, child);
	}

//	return NULL;
}

/**
 * Iterate through the children of an object (start from the "oldest")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
obj_t * obj_get_child_back(obj_t * obj, obj_t * child)
{
    if(child == NULL) {
        return ll_get_tail(&obj->child_ll);
    } else {
        return ll_get_prev(&obj->child_ll, child);
    }

//    return NULL;
}

/**
 * Count the children of an object (only children directly on 'obj')
 * @param obj pointer to an object
 * @return children number of 'obj'
 */
uint16_t obj_count_children(obj_t * obj)
{
	obj_t * i;
	uint16_t cnt = 0;

	LL_READ(obj->child_ll, i) cnt++;

	return cnt;
}

/*---------------------
 * Coordinate get
 *--------------------*/

/**
 * Copy the coordinates of an object to an area
 * @param obj pointer to an object
 * @param cords_p pointer to an area to store the coordinates 
 */
void obj_get_coords(obj_t * obj, area_t * cords_p)
{
    area_copy(cords_p, &obj->coords);
}


/**
 * Get the x coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the left side of its parent 
 */
coord_t obj_get_x(obj_t * obj)
{
    coord_t rel_x;
    obj_t * parent = obj_get_parent(obj);
    rel_x = obj->coords.x1 - parent->coords.x1;
    
    return rel_x;
}

/**
 * Get the y coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the top of its parent 
 */
coord_t obj_get_y(obj_t * obj)
{
    coord_t rel_y;
    obj_t * parent = obj_get_parent(obj);
    rel_y = obj->coords.y1 - parent->coords.y1;
    
    return rel_y;
}

/**
 * Get the width of an object
 * @param obj pointer to an object
 * @return the width
 */
coord_t obj_get_width(obj_t * obj)
{
    return area_get_width(&obj->coords);
}

/**
 * Get the height of an object
 * @param obj pointer to an object
 * @return the height
 */
coord_t obj_get_height(obj_t * obj)
{
    return area_get_height(&obj->coords);
}

/**
 * Get the extended size attribute of an object
 * @param obj pointer to an object
 * @return the extended size attribute
 */
coord_t obj_get_ext_size(obj_t * obj)
{
    return obj->ext_size;
}

/*-----------------
 * Appearance get
 *---------------*/

/**
 * Get the style pointer of an object (if NULL get style of the parent)
 * @param obj pointer to an object
 * @return pointer to a style
 */
style_t * obj_get_style(obj_t * obj)
{
    style_t * style_act = obj->style_p;
    if(style_act == NULL) {
        obj_t * par = obj->par;

        while(par) {
            if(par->style_p) {
                if(par->style_p->glass == 0) {
#if GUI_USE_GROUP == 0
                    style_act = par->style_p;
#else
                    /*Is a parent is focused then use then focused style*/
                    group_t *g = obj_get_group(par);
                    if(group_get_focused(g) == par) {
                        style_act = group_mod_style(g, par->style_p);
                    } else {
                        style_act = par->style_p;
                    }
#endif
                    break;
                }
            }
            par = par->par;
        }
    }
#if GUI_USE_GROUP
    if(obj->group_p) {
        if(group_get_focused(obj->group_p) == obj) {
            style_act = group_mod_style(obj->group_p, style_act);
        }
    }
#endif

    if(style_act == NULL) style_act = &style_plain;

    return style_act;
}

/*-----------------
 * Attribute get
 *----------------*/

/**
 * Get the hidden attribute of an object
 * @param obj pointer to an object
 * @return true: the object is hidden
 */
bool obj_get_hidden(obj_t * obj)
{
    return obj->hidden == 0 ? false : true;
}

/**
 * Get the click enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is clickable
 */
bool obj_get_click(obj_t * obj)
{
    return obj->click == 0 ? false : true;
}

/**
 * Get the top enable attribute of an object
 * @param obj pointer to an object
 * @return true: the auto top feture is enabled
 */
bool obj_get_top(obj_t * obj)
{
    return obj->top == 0 ? false : true;
}

/**
 * Get the drag enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is dragable
 */
bool obj_get_drag(obj_t * obj)
{
    return obj->drag == 0 ? false : true;   
}

/**
 * Get the drag thow enable attribute of an object
 * @param obj pointer to an object
 * @return true: drag throw is enabled
 */
bool obj_get_drag_throw(obj_t * obj)
{
    return obj->drag_throw == 0 ? false : true;
}

/**
 * Get the drag parent attribute of an object
 * @param obj pointer to an object
 * @return true: drag parent is enabled
 */
bool obj_get_drag_parent(obj_t * obj)
{
    return obj->drag_parent == 0 ? false : true;
}

/**
 * Get the protect field of an object
 * @param obj pointer to an object
 * @return protect field ('OR'ed values of obj_prot_t)
 */
uint8_t obj_get_protect(obj_t * obj)
{
    return obj->protect ;
}

/**
 * Check at least one bit of a given protect bitfield is set
 * @param obj pointer to an object
 * @param prot protect bits to test ('OR'ed values of obj_prot_t)
 * @return false: none of the given bits are set, true: at least one bit is set
 */
bool obj_is_protected(obj_t * obj, uint8_t prot)
{
    return (obj->protect & prot) == 0 ? false : true ;
}

/**
 * Get the signal function of an object
 * @param obj pointer to an object
 * @return the signal function
 */
signal_func_t   obj_get_signal_func(obj_t * obj)
{
    return obj->signal_func;
}

/**
 * Get the design function of an object
 * @param obj pointer to an object
 * @return the design function
 */
design_func_t obj_get_design_func(obj_t * obj)
{
    return obj->design_func;
}

/*------------------
 * Other get
 *-----------------*/

/**
 * Get the ext pointer
 * @param obj pointer to an object
 * @return the ext pointer but not the dynamic version
 *         Use it as ext->data1, and NOT da(ext)->data1
 */
void * obj_get_ext_attr(obj_t * obj)
{
   return obj->ext_attr;
}

/**
 * Get object's and its ancestors type. Put their name in `type_buf` starting with the current type.
 * E.g. buf.type[0]="btn", buf.type[1]="cont", buf.type[2]="obj"
 * @param obj pointer to an object which type should be get
 * @param buf pointer to an `obj_type_t` buffer to store the types
 */
void obj_get_type(obj_t * obj, obj_type_t * buf)
{
    obj_type_t tmp;

    memset(buf, 0, sizeof(obj_type_t));
    memset(&tmp, 0, sizeof(obj_type_t));

    obj->signal_func(obj, GUI_SIGNAL_GET_TYPE, &tmp);

    uint8_t cnt;
    for(cnt = 0; cnt < GUI_MAX_ANCESTOR_NUM; cnt++) {
        if(tmp.type[cnt] == NULL) break;
    }


    /*Swap the order. The real type comes first*/
    uint8_t i;
    for(i = 0; i < cnt; i++) {
        buf->type[i] = tmp.type[cnt - 1 - i];
    }
}

#ifdef GUI_OBJ_FREE_NUM_TYPE
/**
 * Get the free number
 * @param obj pointer to an object
 * @return the free number
 */
GUI_OBJ_FREE_NUM_TYPE obj_get_free_num(obj_t * obj)
{
    return obj->free_num;
}
#endif

#if GUI_OBJ_FREE_PTR != 0
/**
 * Get the free pointer
 * @param obj pointer to an object
 * @return the free pointer
 */
void * obj_get_free_ptr(obj_t * obj)
{
    return obj->free_ptr;
}
#endif


#if GUI_USE_GROUP
/**
 * Get the group of the object
 * @param obj pointer to an object
 * @return the pointer to group of the object
 */
void * obj_get_group(obj_t * obj)
{
    return obj->group_p;
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the base objects.
 * @param obj pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 * @param return true/false, depends on 'mode'        
 */
static bool obj_design(obj_t * obj, const  area_t * mask_p, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {

        /* Because of the radius it is not sure the area is covered
         * Check the areas where there is no radius*/
        style_t * style = obj_get_style(obj);
        if(style->body.empty != 0) return false;

        uint16_t r = style->body.radius;

        if(r == GUI_RADIUS_CIRCLE) return false;

        area_t area_tmp;

        /*Check horizontally without radius*/
        obj_get_coords(obj, &area_tmp);
        area_tmp.x1 += r;
        area_tmp.x2 -= r;
        if(area_is_in(mask_p, &area_tmp) == false) return false;

        /*Check vertically without radius*/
        obj_get_coords(obj, &area_tmp);
        area_tmp.y1 += r;
        area_tmp.y2 -= r;
        if(area_is_in(mask_p, &area_tmp) == false) return false;

    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
		style_t * style = obj_get_style(obj);
		draw_rect(&obj->coords, mask_p, style);
    }

    return true;
}

/**
 * Signal function of the basic object
 * @param obj pointer to an object
 * @param sign signal type
 * @param param parameter for the signal (depends on signal type)
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t obj_signal(obj_t * obj, signal_t sign, void * param)
{
    (void)param;

    res_t res = GUI_RES_OK;

    style_t * style = obj_get_style(obj);
    if(sign == GUI_SIGNAL_CHILD_CHG) {
        /*Return 'invalid' if the child change signal is not enabled*/
        if(obj_is_protected(obj, GUI_PROTECT_CHILD_CHG) != false) res = GUI_RES_INV;
    }
    else if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        if(style->body.shadow.width > obj->ext_size) obj->ext_size = style->body.shadow.width;
    }
    else if(sign ==  GUI_SIGNAL_STYLE_CHG) {
        obj_refresh_ext_size(obj);
    }
    else if(sign ==  GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        buf->type[0] = "obj";
    }

    return res;
}

/**
 * Reposition the children of an object. (Called recursively)
 * @param obj pointer to an object which children will be repositioned
 * @param x_diff x coordinate shift
 * @param y_diff y coordinate shift
 */
static void refresh_childen_position(obj_t * obj, coord_t x_diff, coord_t y_diff)
{
    obj_t * i;   
    LL_READ(obj->child_ll, i) {
        i->coords.x1 += x_diff;
        i->coords.y1 += y_diff;
        i->coords.x2 += x_diff;
        i->coords.y2 += y_diff;
        
        refresh_childen_position(i, x_diff, y_diff);
    }
}

/**
 * Refresh the style of all children of an object. (Called recursively)
 * @param style_p refresh objects only with this style. (ignore is if NULL)
 * @param obj pointer to an object
 */
static void report_style_mod_core(void * style_p, obj_t * obj)
{
    obj_t * i;
    LL_READ(obj->child_ll, i) {
        if(i->style_p == style_p) {
            refresh_childen_style(i);
            obj_refresh_style(i);
        }
        
        report_style_mod_core(style_p, i);
    }
}


/**
 * Recursively refresh the style of the children. Go deeper until a not NULL style is found
 * because the NULL styles are inherited from the parent
 * @param obj pointer to an object
 */
static void refresh_childen_style(obj_t * obj)
{
    obj_t * child = obj_get_child(obj, NULL);
    while(child != NULL) {
        if(child->style_p == NULL) {
            refresh_childen_style(child);     /*Check children too*/
            obj_refresh_style(child);       /*Notify the child about the style change*/
        } else if(child->style_p->glass) {
            /*Children with 'glass' parent might be effected if their style == NULL*/
            refresh_childen_style(child);
        }
        child = obj_get_child(child, NULL);
    }
}

/**
 * Called by 'obj_del' to delete the children objects
 * @param obj pointer to an object (all of its children will be deleted)
 */
static void delete_children(obj_t * obj)
{
   obj_t * i;
   obj_t * i_next;
   i = ll_get_head(&(obj->child_ll));
   while(i != NULL) {
       /*Get the next object before delete this*/
       i_next = ll_get_next(&(obj->child_ll), i);

       /*Call the recursive del to the child too*/
       delete_children(i);

       /*Set i to the next node*/
       i = i_next;
   }

   /*Remove the animations from this object*/
#if GUI_USE_ANIMATION
   anim_del(obj, NULL);
#endif

   /*Delete from the group*/
#if GUI_USE_GROUP
   if(obj->group_p != NULL) group_remove_obj(obj);
#endif

   /* Reset the input devices if
    * the currently pressed object is deleted*/
   indev_t * indev = indev_next(NULL);
   while(indev) {
       if(indev->proc.act_obj == obj || indev->proc.last_obj == obj) {
           indev_reset(indev);
       }
       indev = indev_next(indev);
   }

   /*Remove the object from parent's children list*/
   obj_t * par = obj_get_parent(obj);
   ll_rem(&(par->child_ll), obj);

   /* Clean up the object specific data*/
   obj->signal_func(obj, GUI_SIGNAL_CLEANUP, NULL);

   /*Delete the base objects*/
   if(obj->ext_attr != NULL)  RTE_BRel(MEM_RTE,obj->ext_attr);
   RTE_BRel(MEM_RTE,obj); /*Free the object itself*/

}
#endif
