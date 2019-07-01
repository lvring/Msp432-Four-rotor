#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2018.10.09
*** History: 1.0 创建，修改自lvgl：https://littlevgl.com/
*****************************************************************************/
#if RTE_USE_GUI == 1
#if GUI_INDEV_READ_PERIOD != 0
/*************************************************
*** 内部函数
*************************************************/
static void indev_proc_task(void * param);
static void indev_pointer_proc(indev_t * i, indev_data_t * data);
static void indev_keypad_proc(indev_t * i, indev_data_t * data);
static void indev_button_proc(indev_t * i, indev_data_t * data);
static void indev_proc_press(indev_proc_t * proc);
static void indev_proc_release(indev_proc_t * proc);
static void indev_proc_reset_query_handler(indev_t * indev);
static obj_t * indev_search_obj(const indev_proc_t * indev, obj_t * obj);
static void indev_drag(indev_proc_t * state);
static void indev_drag_throw(indev_proc_t * state);
#endif
/*************************************************
*** 内部变量
*************************************************/
static indev_t *indev_act = NULL;
/*************************************************
*** Args:   NULL
*** Function: 初始化输入设备
*************************************************/
void indev_init(void)
{
#if GUI_INDEV_READ_PERIOD != 0
  RTE_RoundRobin_CreateTimer("InputTimer",GUI_INDEV_READ_PERIOD,1,1,1,indev_proc_task, (void *)0);
#endif
	indev_reset(NULL);   /*Reset all input devices*/
}
/*************************************************
*** @return pointer to the currently processed input device or NULL if no input device processing right now
*** Function: 获取当前正在处理的输入设备
*************************************************/
indev_t * indev_get_act(void)
{
	return indev_act;
}

/**
 * Reset one or all input devices
 * @param indev pointer to an input device to reset or NULL to reset all of them
 */
void indev_reset(indev_t * indev)
{
    if(indev) indev->proc.reset_query = 1;
    else {
        indev_t * i = indev_next(NULL);
        while(i) {
            i->proc.reset_query = 1;
            i = indev_next(i);
        }
    }
}

/**
 * Reset the long press state of an input device
 * @param indev pointer to an input device
 */
void indev_reset_lpr(indev_t * indev)
{
    indev->proc.long_pr_sent = 0;
    indev->proc.longpr_rep_timestamp = RTE_RoundRobin_GetTick();
    indev->proc.pr_timestamp = RTE_RoundRobin_GetTick();
}

/**
 * Enable input devices device by type
 * @param type Input device type
 * @param enable true: enable this type; false: disable this type
 */
void indev_enable(hal_indev_type_t type, bool enable)
{
    indev_t *i = indev_next(NULL);

    while (i) {
        if (i->driver.type == type) i->proc.disabled = enable == false ? 1 : 0;
        i = indev_next(i);
    }
}

/**
 * Set a cursor for a pointer input device (for GUI_INPUT_TYPE_POINTER and GUI_INPUT_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param cur_obj pointer to an object to be used as cursor
 */
void indev_set_cursor(indev_t *indev, obj_t *cur_obj)
{
    if(indev->driver.type != GUI_INDEV_TYPE_POINTER && indev->driver.type != GUI_INDEV_TYPE_BUTTON) return;

    indev->cursor = cur_obj;
    obj_set_parent(indev->cursor, layer_sys());
    obj_set_pos(indev->cursor, indev->proc.act_point.x,  indev->proc.act_point.y);
}

#if GUI_USE_GROUP
/**
 * Set a destination group for a keypad input device (for GUI_INDEV_TYPE_KEYPAD)
 * @param indev pointer to an input device
 * @param group point to a group
 */
void indev_set_group(indev_t *indev, group_t *group)
{
    if(indev->driver.type == GUI_INDEV_TYPE_KEYPAD) indev->group = group;
}
#endif

/**
 * Set the an array of points for GUI_INDEV_TYPE_BUTTON.
 * These points will be assigned to the buttons to press a specific point on the screen
 * @param indev pointer to an input device
 * @param group point to a group
 */
void indev_set_button_points(indev_t *indev, point_t *points)
{
    if(indev->driver.type == GUI_INDEV_TYPE_BUTTON) indev->btn_points = points;
}

/**
 * Get the last point of an input device (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param point pointer to a point to store the result
 */
void indev_get_point(indev_t * indev, point_t * point)
{
    if(indev->driver.type != GUI_INDEV_TYPE_POINTER && indev->driver.type != GUI_INDEV_TYPE_BUTTON) {
        point->x = 0;
        point->y = 0;
    } else {
        point->x = indev->proc.act_point.x;
        point->y = indev->proc.act_point.y;
    }
}

/**
 * Check if there is dragging with an input device or not (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @return true: drag is in progress
 */
bool indev_is_dragging(indev_t * indev)
{
    if(indev == NULL) return false;
    if(indev->driver.type != GUI_INDEV_TYPE_POINTER && indev->driver.type != GUI_INDEV_TYPE_BUTTON) return false;
    return indev->proc.drag_in_prog == 0 ? false : true;
}

/**
 * Get the vector of dragging of an input device (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param point pointer to a point to store the vector
 */
void indev_get_vect(indev_t * indev, point_t * point)
{
    if(indev->driver.type != GUI_INDEV_TYPE_POINTER && indev->driver.type != GUI_INDEV_TYPE_BUTTON) {
        point->x = 0;
        point->y = 0;
    } else {
        point->x = indev->proc.vect.x;
        point->y = indev->proc.vect.y;
    }
}

/**
 * Get elapsed time since last press
 * @param indev pointer to an input device (NULL to get the overall smallest inactivity)
 * @return Elapsed ticks (milliseconds) since last press
 */
uint32_t indev_get_inactive_time(indev_t * indev)
{
    uint32_t t;

    if(indev) return t = RTE_RoundRobin_TickElaps(indev->last_activity_time);

    indev_t *i;
    t = UINT16_MAX;
    i = indev_next(NULL);
    while(i) {
        t = RTE_MATH_MIN(t, RTE_RoundRobin_TickElaps(i->last_activity_time));
        i = indev_next(i);
    }

    return t;
}

/**
 * Do nothing until the next release
 * @param indev pointer to an input device
 */
void indev_wait_release(indev_t * indev)
{
    indev->proc.wait_unil_release = 1;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if GUI_INDEV_READ_PERIOD != 0
/**
 * Called periodically to handle the input devices
 * @param param unused
 */
static void indev_proc_task(void * param)
{
    (void)param;

    indev_data_t data;
    indev_t * i;
    i = indev_next(NULL);

    /*Read and process all indevs*/
    while(i) {
        indev_act = i;

        /*Handle reset query before processing the point*/
        indev_proc_reset_query_handler(i);

        if(i->proc.disabled == 0) {
            bool more_to_read;
            do {
                /*Read the data*/
                more_to_read = indev_read(i, &data);
                i->proc.state = data.state;

                if(i->proc.state == GUI_INDEV_STATE_PR) {
                    i->last_activity_time = RTE_RoundRobin_GetTick();
                }

                if(i->driver.type == GUI_INDEV_TYPE_POINTER) {
                    indev_pointer_proc(i,&data);
                }
                else if(i->driver.type == GUI_INDEV_TYPE_KEYPAD) {
                    indev_keypad_proc(i, &data);
                }
                else if(i->driver.type == GUI_INDEV_TYPE_BUTTON) {
                    indev_button_proc(i, &data);
                }
                /*Handle reset query if it happened in during processing*/
                indev_proc_reset_query_handler(i);
            } while(more_to_read);
        }
        i = indev_next(i);    /*Go to the next indev*/
    }

    indev_act = NULL;   /*End of indev processing, so no act indev*/
}


/**
 * Process a new point from GUI_INDEV_TYPE_POINTER input device
 * @param i pointer to an input device
 * @param data pointer to the data read from the input device
 */
static void indev_pointer_proc(indev_t * i, indev_data_t * data)
{
    /*Move the cursor if set and moved*/
    if(i->cursor != NULL &&
       (i->proc.last_point.x != data->point.x ||
        i->proc.last_point.y != data->point.y))
    {
        obj_set_pos(i->cursor, data->point.x, data->point.y);
    }

    i->proc.act_point.x = data->point.x;
    i->proc.act_point.y = data->point.y;

    if(i->proc.state == GUI_INDEV_STATE_PR){
        indev_proc_press(&i->proc);
    } else {
        indev_proc_release(&i->proc);
    }

    i->proc.last_point.x = i->proc.act_point.x;
    i->proc.last_point.y = i->proc.act_point.y;
}

/**
 * Process a new point from GUI_INDEV_TYPE_KEYPAD input device
 * @param i pointer to an input device
 * @param data pointer to the data read from the input device
 */
static void indev_keypad_proc(indev_t * i, indev_data_t * data)
{
#if GUI_USE_GROUP
    if(i->group == NULL) return;

    /*Key press happened*/
    if(data->state == GUI_INDEV_STATE_PR &&
       i->proc.last_state == GUI_INDEV_STATE_REL)
    {
        i->proc.pr_timestamp = RTE_RoundRobin_GetTick();
    }
    /*Pressing*/
    else if(data->state == GUI_INDEV_STATE_PR && i->proc.last_state == GUI_INDEV_STATE_PR) {
        if(data->key == GUI_GROUP_KEY_ENTER &&
           i->proc.long_pr_sent == 0 &&
           RTE_RoundRobin_TickElaps(i->proc.pr_timestamp) > GUI_INDEV_LONG_PRESS_TIME)
        {

            group_send_data(i->group, GUI_GROUP_KEY_ENTER_LONG);
            i->proc.long_pr_sent = 1;

        }
    }
    /*Release happened*/
    else if(data->state == GUI_INDEV_STATE_REL && i->proc.last_state == GUI_INDEV_STATE_PR)
    {
        /*The user might clear the key it was released. Always release the pressed key*/
        data->key = i->proc.last_key;

        if(data->key == GUI_GROUP_KEY_NEXT) {
                   group_focus_next(i->group);
        }
        else if(data->key == GUI_GROUP_KEY_PREV) {
            group_focus_prev(i->group);
        }
        else {
            group_send_data(i->group, data->key);
        }

        i->proc.pr_timestamp = 0;
        i->proc.long_pr_sent = 0;
    }

    i->proc.last_state = data->state;
    i->proc.last_key = data->key;
#endif
}


/**
 * Process new points from a input device. indev->state.pressed has to be set
 * @param indev pointer to an input device state
 * @param x x coordinate of the next point
 * @param y y coordinate of the next point
 */
static void indev_button_proc(indev_t * i, indev_data_t * data)
{
    i->proc.act_point.x = i->btn_points[data->btn].x;
    i->proc.act_point.y = i->btn_points[data->btn].y;

    /*Still the same point is pressed*/
    if(i->proc.last_point.x == i->proc.act_point.x &&
       i->proc.last_point.y == i->proc.act_point.y &&
       data->state == GUI_INDEV_STATE_PR) {
        indev_proc_press(&i->proc);
    } else {
        /*If a new point comes always make a release*/
        indev_proc_release(&i->proc);
    }

    i->proc.last_point.x = i->proc.act_point.x;
    i->proc.last_point.y = i->proc.act_point.y;
}

/**
 * Process the pressed state of GUI_INDEV_TYPE_POINER input devices
 * @param indev pointer to an input device 'proc'
 */
static void indev_proc_press(indev_proc_t * proc)
{
    obj_t * pr_obj = proc->act_obj;
    
    if(proc->wait_unil_release != 0) return;

    /*If there is no last object then search*/
    if(proc->act_obj == NULL) {
        pr_obj = indev_search_obj(proc, layer_top());
        if(pr_obj == NULL) pr_obj = indev_search_obj(proc, scr_act());
    }
    /*If there is last object but it is not dragged and not protected also search*/
    else if(proc->drag_in_prog == 0 &&
            obj_is_protected(proc->act_obj, GUI_PROTECT_PRESS_LOST) == false) {/*Now act_obj != NULL*/
        pr_obj = indev_search_obj(proc, layer_top());
        if(pr_obj == NULL) pr_obj = indev_search_obj(proc, scr_act());
    }
    /*If a dragable or a protected object was the last then keep it*/
    else {
        
    }
    
    /*If a new object was found reset some variables and send a pressed signal*/
    if(pr_obj != proc->act_obj) {

        proc->last_point.x = proc->act_point.x;
        proc->last_point.y = proc->act_point.y;
        
        /*If a new object found the previous was lost, so send a signal*/
        if(proc->act_obj != NULL) {
            proc->act_obj->signal_func(proc->act_obj, GUI_SIGNAL_PRESS_LOST, indev_act);
            if(proc->reset_query != 0) return;
        }
        
        if(pr_obj != NULL) {
            /* Save the time when the obj pressed. 
             * It is necessary to count the long press time.*/
            proc->pr_timestamp = RTE_RoundRobin_GetTick();
            proc->long_pr_sent = 0;
            proc->drag_range_out = 0;
            proc->drag_in_prog = 0;
            proc->drag_sum.x = 0;
            proc->drag_sum.y = 0;

            /*Search for 'top' attribute*/
            obj_t * i = pr_obj;
            obj_t * last_top = NULL;
            while(i != NULL){
				if(i->top != 0) last_top = i;
				i = obj_get_parent(i);
            }

            if(last_top != NULL) {
            	/*Move the last_top object to the foreground*/
            	obj_t * par = obj_get_parent(last_top);
            	/*After list change it will be the new head*/
                ll_chg_list(&par->child_ll, &par->child_ll, last_top);
                obj_invalidate(last_top);
            }

            /*Send a signal about the press*/
            pr_obj->signal_func(pr_obj, GUI_SIGNAL_PRESSED, indev_act);
            if(proc->reset_query != 0) return;
        }
    }
    
    proc->act_obj = pr_obj;            /*Save the pressed object*/
    proc->last_obj = proc->act_obj;   /*Refresh the last_obj*/

    /*Calculate the vector*/
    proc->vect.x = proc->act_point.x - proc->last_point.x;
    proc->vect.y = proc->act_point.y - proc->last_point.y;

    /*If there is active object and it can be dragged run the drag*/
    if(proc->act_obj != NULL) {
        proc->act_obj->signal_func(proc->act_obj, GUI_SIGNAL_PRESSING, indev_act);
        if(proc->reset_query != 0) return;

        indev_drag(proc);
        if(proc->reset_query != 0) return;

        /*If there is no drag then check for long press time*/
        if(proc->drag_in_prog == 0 && proc->long_pr_sent == 0) {
            /*Send a signal about the long press if enough time elapsed*/
            if(RTE_RoundRobin_TickElaps(proc->pr_timestamp) > GUI_INDEV_LONG_PRESS_TIME) {
                pr_obj->signal_func(pr_obj, GUI_SIGNAL_LONG_PRESS, indev_act);
                if(proc->reset_query != 0) return;

                /*Mark the signal sending to do not send it again*/
                proc->long_pr_sent = 1;

                /*Save the long press time stamp for the long press repeat handler*/
                proc->longpr_rep_timestamp = RTE_RoundRobin_GetTick();
            }
        }
        /*Send long press repeated signal*/
        if(proc->drag_in_prog == 0 && proc->long_pr_sent == 1) {
            /*Send a signal about the long press repeate if enough time elapsed*/
            if(RTE_RoundRobin_TickElaps(proc->longpr_rep_timestamp) > GUI_INDEV_LONG_PRESS_REP_TIME) {
                pr_obj->signal_func(pr_obj, GUI_SIGNAL_LONG_PRESS_REP, indev_act);
                if(proc->reset_query != 0) return;
                proc->longpr_rep_timestamp = RTE_RoundRobin_GetTick();

            }
        }
    }
}

/**
 * Process the released state of GUI_INDEV_TYPE_POINER input devices
 * @param proc pointer to an input device 'proc'
 */
static void indev_proc_release(indev_proc_t * proc)
{
    if(proc->wait_unil_release != 0) {
        proc->act_obj = NULL;
        proc->last_obj = NULL;
        proc->pr_timestamp = 0;
        proc->longpr_rep_timestamp = 0;
        proc->wait_unil_release = 0;
    }

    /*Forgot the act obj and send a released signal */
    if(proc->act_obj != NULL) {
        proc->act_obj->signal_func(proc->act_obj, GUI_SIGNAL_RELEASED, indev_act);
        if(proc->reset_query != 0) return;
        proc->act_obj = NULL;
        proc->pr_timestamp = 0;
        proc->longpr_rep_timestamp = 0;
    }
    
    /*The reset can be set in the signal function. 
     * In case of reset query ignore the remaining parts.*/
    if(proc->last_obj != NULL && proc->reset_query == 0) {
        indev_drag_throw(proc);
        if(proc->reset_query != 0) return;
    }
}

/**
 * Process a new point from GUI_INDEV_TYPE_BUTTON input device
 * @param i pointer to an input device
 * @param data pointer to the data read from the input device
 * Reset input device if a reset query has been sent to it
 * @param indev pointer to an input device
 */
static void indev_proc_reset_query_handler(indev_t * indev)
{
    if(indev->proc.reset_query) {
        indev->proc.act_obj = NULL;
        indev->proc.last_obj = NULL;
        indev->proc.drag_range_out = 0;
        indev->proc.drag_in_prog = 0;
        indev->proc.long_pr_sent = 0;
        indev->proc.pr_timestamp = 0;
        indev->proc.longpr_rep_timestamp = 0;
        indev->proc.drag_sum.x = 0;
        indev->proc.drag_sum.y = 0;
        indev->proc.reset_query = 0;
    }
}
/**
 * Search the most top, clickable object on the last point of an input device
 * @param indev pointer to  an input device
 * @param obj pointer to a start object, typically the screen
 * @return pointer to the found object or NULL if there was no suitable object 
 */
static obj_t * indev_search_obj(const indev_proc_t * indev, obj_t * obj)
{
    obj_t * found_p = NULL;
    
    /*If the point is on this object*/
    /*Check its children too*/
    if(area_is_point_on(&obj->coords, &indev->act_point)) {
        obj_t * i;
    
        LL_READ(obj->child_ll, i) {
            found_p = indev_search_obj(indev, i);
            
            /*If a child was found then break*/
            if(found_p != NULL) {
                break;
            }
        }
        
        /*If then the children was not ok, and this obj is clickable
         * and it or its parent is not hidden then save this object*/
        if(found_p == NULL && obj_get_click(obj) != false) {
        	obj_t * hidden_i = obj;
        	while(hidden_i != NULL) {
        		if(obj_get_hidden(hidden_i) == true) break;
        		hidden_i = obj_get_parent(hidden_i);
        	}
        	/*No parent found with hidden == true*/
        	if(hidden_i == NULL) found_p = obj;
        }
        
    }
    
    return found_p;    
}

/**
 * Handle the dragging of indev_proc_p->act_obj
 * @param indev pointer to a input device state
 */
static void indev_drag(indev_proc_t * state)
{
    obj_t * drag_obj = state->act_obj;
    
    /*If drag parent is active check recursively the drag_parent attribute*/
	while(obj_get_drag_parent(drag_obj) != false &&
		  drag_obj != NULL) {
		drag_obj = obj_get_parent(drag_obj);
	}

	if(drag_obj == NULL) return;
    
    if(obj_get_drag(drag_obj) == false) return;

    /*If still there is no drag then count the movement*/
    if(state->drag_range_out == 0) {
        state->drag_sum.x += state->vect.x;
        state->drag_sum.y += state->vect.y;
        
        /*If a move is greater then GUI_DRAG_LIMIT then begin the drag*/
        if(RTE_MATH_ABS(state->drag_sum.x) >= GUI_INDEV_DRAG_LIMIT ||
           RTE_MATH_ABS(state->drag_sum.y) >= GUI_INDEV_DRAG_LIMIT)
           {
                state->drag_range_out = 1;
           }
    }
    
    /*If the drag limit is stepped over then handle the dragging*/
    if(state->drag_range_out != 0) {
        /*Set new position if the vector is not zero*/
        if(state->vect.x != 0 ||
           state->vect.y != 0) {
            /*Get the coordinates of the object and modify them*/
            coord_t act_x = obj_get_x(drag_obj);
            coord_t act_y = obj_get_y(drag_obj);
            uint16_t inv_buf_size = refr_get_buf_size(); /*Get the number of currently invalidated areas*/

            coord_t prev_x = drag_obj->coords.x1;
            coord_t prev_y = drag_obj->coords.y1;

            obj_set_pos(drag_obj, act_x + state->vect.x, act_y + state->vect.y);

            /*Set the drag in progress flag if the object is really moved*/

            if(drag_obj->coords.x1 != prev_x || drag_obj->coords.y1 != prev_y) {
                if(state->drag_range_out != 0) { /*Send the drag begin signal on first move*/
                    drag_obj->signal_func(drag_obj,  GUI_SIGNAL_DRAG_BEGIN, indev_act);
                    if(state->reset_query != 0) return;
                }
                state->drag_in_prog = 1;
            }
            /*If the object didn't moved then clear the invalidated areas*/
            else {
                uint16_t new_inv_buf_size = refr_get_buf_size();
                refr_pop_from_buf(new_inv_buf_size - inv_buf_size);
            }
        }
    }
}

/**
 * Handle throwing by drag if the drag is ended
 * @param indev pointer to an input device state
 */
static void indev_drag_throw(indev_proc_t * state)
{
	if(state->drag_in_prog == 0) return;

    /*Set new position if the vector is not zero*/
    obj_t * drag_obj = state->last_obj;
    
    /*If drag parent is active check recursively the drag_parent attribute*/
	while(obj_get_drag_parent(drag_obj) != false &&
		  drag_obj != NULL) {
		drag_obj = obj_get_parent(drag_obj);
	}

	if(drag_obj == NULL) return;
    
    /*Return if the drag throw is not enabled*/
    if(obj_get_drag_throw(drag_obj) == false ){
    	state->drag_in_prog = 0;
        drag_obj->signal_func(drag_obj, GUI_SIGNAL_DRAG_END, indev_act);
        return;
    }
    
    /*Reduce the vectors*/
    state->vect.x = state->vect.x * (100 -GUI_INDEV_DRAG_THROW) / 100;
    state->vect.y = state->vect.y * (100 -GUI_INDEV_DRAG_THROW) / 100;
    
    if(state->vect.x != 0 ||
       state->vect.y != 0)
    {
        /*Get the coordinates  and modify them*/
        coord_t act_x = obj_get_x(drag_obj) + state->vect.x;
        coord_t act_y = obj_get_y(drag_obj) + state->vect.y;
        obj_set_pos(drag_obj, act_x, act_y);

        /*If non of the coordinates are changed then do not continue throwing*/
        if((obj_get_x(drag_obj) != act_x || state->vect.x == 0) &&
           (obj_get_y(drag_obj) != act_y || state->vect.y == 0)) {
            state->drag_in_prog = 0;
            state->vect.x = 0;
            state->vect.y = 0;
            drag_obj->signal_func(drag_obj, GUI_SIGNAL_DRAG_END, indev_act);

        }
    }
    /*If the vectors become 0 -> drag_in_prog = 0 and send a drag end signal*/
    else {
        state->drag_in_prog = 0;
        drag_obj->signal_func(drag_obj, GUI_SIGNAL_DRAG_END, indev_act);
    }
}
#endif
#endif
