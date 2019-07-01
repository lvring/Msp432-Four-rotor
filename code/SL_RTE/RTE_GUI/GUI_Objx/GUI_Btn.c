/**
 * @file btn.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/

#include "RTE_Include.h"
#if GUI_USE_BTN != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static signal_func_t ancestor_signal;

/**********************
 *  STATIC VARIABLES
 **********************/
static res_t btn_signal(obj_t * btn, signal_t sign, void * param);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a button objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a button object, if not NULL then the new object will be copied from it
 * @return pointer to the created button
 */
obj_t * btn_create(obj_t * par, obj_t * copy)
{
    obj_t * new_btn;
    
    new_btn = cont_create(par, copy);
    RTE_AssertParam(new_btn);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_btn);

    /*Allocate the extended data*/
    btn_ext_t * ext = obj_allocate_ext_attr(new_btn, sizeof(btn_ext_t));
    RTE_AssertParam(ext);
    ext->state = GUI_BTN_STATE_REL;

    ext->actions[GUI_BTN_ACTION_PR] = NULL;
    ext->actions[GUI_BTN_ACTION_CLICK] = NULL;
    ext->actions[GUI_BTN_ACTION_LONG_PR] = NULL;
    ext->actions[GUI_BTN_ACTION_LONG_PR_REPEAT] = NULL;

    ext->styles[GUI_BTN_STATE_REL] = &style_btn_rel;
    ext->styles[GUI_BTN_STATE_PR] = &style_btn_pr;
    ext->styles[GUI_BTN_STATE_TGL_REL] = &style_btn_tgl_rel;
    ext->styles[GUI_BTN_STATE_TGL_PR] = &style_btn_tgl_pr;
    ext->styles[GUI_BTN_STATE_INA] = &style_btn_ina;

    ext->long_pr_action_executed = 0;
    ext->toggle = 0;

    obj_set_signal_func(new_btn, btn_signal);
    
    /*If no copy do the basic initialization*/
    if(copy == NULL) {
        /*Set layout if the button is not a screen*/
        if(par != NULL) {
            btn_set_layout(new_btn, GUI_LAYOUT_CENTER);
        }

        obj_set_click(new_btn, true);        /*Be sure the button is clickable*/

	    /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            btn_set_style(new_btn, GUI_BTN_STYLE_REL, th->btn.rel);
            btn_set_style(new_btn, GUI_BTN_STYLE_PR, th->btn.pr);
            btn_set_style(new_btn, GUI_BTN_STYLE_TGL_REL, th->btn.tgl_rel);
            btn_set_style(new_btn, GUI_BTN_STYLE_TGL_PR, th->btn.tgl_pr);
            btn_set_style(new_btn, GUI_BTN_STYLE_INA, th->btn.ina);
        } else {
            obj_set_style(new_btn, ext->styles[GUI_BTN_STATE_REL]);
        }
    }
    /*Copy 'copy'*/
    else {
    	btn_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->state = copy_ext->state;
        ext->toggle = copy_ext->toggle;
    	memcpy(ext->actions, copy_ext->actions, sizeof(ext->actions));
        memcpy(ext->styles, copy_ext->styles, sizeof(ext->styles));

    	/*Refresh the style with new signal function*/
        obj_refresh_style(new_btn);
    }
    
    return new_btn;
}

/*=====================
 * Setter functions 
 *====================*/

/**
 * Enable the toggled states
 * @param btn pointer to a button object
 * @param tgl true: enable toggled states, false: disable
 */
void btn_set_toggle(obj_t * btn, bool tgl)
{
    btn_ext_t * ext = obj_get_ext_attr(btn);
    
    ext->toggle = tgl != false ? 1 : 0;
}

/**
 * Set the state of the button
 * @param btn pointer to a button object
 * @param state the new state of the button (from btn_state_t enum)
 */
void btn_set_state(obj_t * btn, btn_state_t state)
{
    btn_ext_t * ext = obj_get_ext_attr(btn);
    if(ext->state != state) {
        ext->state = state;
        obj_set_style(btn, ext->styles[state]);
    }
}

/**
 * Toggle the state of the button (ON->OFF, OFF->ON)
 * @param btn pointer to a button object
 */
void btn_toggle(obj_t * btn)
{
    btn_ext_t * ext = obj_get_ext_attr(btn);
    switch(ext->state) {
        case GUI_BTN_STATE_REL: btn_set_state(btn, GUI_BTN_STATE_TGL_REL); break;
        case GUI_BTN_STATE_PR: btn_set_state(btn, GUI_BTN_STATE_TGL_PR); break;
        case GUI_BTN_STATE_TGL_REL: btn_set_state(btn, GUI_BTN_STATE_REL); break;
        case GUI_BTN_STATE_TGL_PR: btn_set_state(btn, GUI_BTN_STATE_PR); break;
        default: break;
    }
}

/**
 * Set a function to call when the button event happens
 * @param btn pointer to a button object
 * @param action type of event form 'action_t' (press, release, long press, long press repeat)
 */
void btn_set_action(obj_t * btn, btn_action_t type, action_t action)
{
    if(type >= GUI_BTN_ACTION_NUM) return;

    btn_ext_t * ext = obj_get_ext_attr(btn);
    ext->actions[type] = action;
}

/**
 * Set a style of a button
 * @param btn pointer to a button object
 * @param type which style should be set
 * @param style pointer to a style
 */
void btn_set_style(obj_t *btn, btn_style_t type, style_t *style)
{
    btn_ext_t *ext = obj_get_ext_attr(btn);

    switch (type) {
        case GUI_BTN_STYLE_REL:
            ext->styles[GUI_BTN_STATE_REL] = style;
            break;
        case GUI_BTN_STYLE_PR:
            ext->styles[GUI_BTN_STATE_PR] = style;
            break;
        case GUI_BTN_STYLE_TGL_REL:
            ext->styles[GUI_BTN_STATE_TGL_REL] = style;
            break;
        case GUI_BTN_STYLE_TGL_PR:
            ext->styles[GUI_BTN_STATE_TGL_PR] = style;
            break;
        case GUI_BTN_STYLE_INA:
            ext->styles[GUI_BTN_STATE_INA] = style;
            break;
    }

    /*Refresh the object with the new style*/
    obj_set_style(btn, ext->styles[ext->state]);
}


/*=====================
 * Getter functions 
 *====================*/

/**
 * Get the current state of the button
 * @param btn pointer to a button object
 * @return the state of the button (from btn_state_t enum)
 */
btn_state_t btn_get_state(obj_t * btn)
{
    btn_ext_t * ext = obj_get_ext_attr(btn);
    return ext->state;
}

/**
 * Get the toggle enable attribute of the button
 * @param btn pointer to a button object
 * @return ture: toggle enabled, false: disabled
 */
bool btn_get_toggle(obj_t * btn)
{
    btn_ext_t * ext = obj_get_ext_attr(btn);
    
    return ext->toggle != 0 ? true : false;
}

/**
 * Get the release action of a button
 * @param btn pointer to a button object
 * @return pointer to the release action function
 */
action_t btn_get_action(obj_t * btn, btn_action_t type)
{
    if(type >= GUI_BTN_ACTION_NUM) return NULL;

    btn_ext_t * ext = obj_get_ext_attr(btn);
    return ext->actions[type];
}

/**
 * Get a style of a button
 * @param btn pointer to a button object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * btn_get_style(obj_t *btn, btn_style_t type)
{
    btn_ext_t *ext = obj_get_ext_attr(btn);

    switch (type) {
        case GUI_BTN_STYLE_REL:     return ext->styles[GUI_BTN_STATE_REL];
        case GUI_BTN_STYLE_PR:      return ext->styles[GUI_BTN_STATE_PR];
        case GUI_BTN_STYLE_TGL_REL: return ext->styles[GUI_BTN_STATE_TGL_REL];
        case GUI_BTN_STYLE_TGL_PR:  return ext->styles[GUI_BTN_STATE_TGL_PR];
        case GUI_BTN_STYLE_INA:     return ext->styles[GUI_BTN_STATE_INA];
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the button
 * @param btn pointer to a button object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t btn_signal(obj_t * btn, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(btn, sign, param);
    if(res != GUI_RES_OK) return res;

    btn_ext_t * ext = obj_get_ext_attr(btn);
    btn_state_t state = btn_get_state(btn);
    bool tgl = btn_get_toggle(btn);

    if(sign == GUI_SIGNAL_PRESSED) {
        /*Refresh the state*/
        if(ext->state == GUI_BTN_STATE_REL) {
            btn_set_state(btn, GUI_BTN_STATE_PR);
        } else if(ext->state == GUI_BTN_STATE_TGL_REL) {
            btn_set_state(btn, GUI_BTN_STATE_TGL_PR);
        }

        ext->long_pr_action_executed = 0;
        /*Call the press action, 'param' is the caller indev_proc*/
        if(ext->actions[GUI_BTN_ACTION_PR] && state != GUI_BTN_STATE_INA) {
            res = ext->actions[GUI_BTN_ACTION_PR](btn);
        }
    }
    else if(sign ==  GUI_SIGNAL_PRESS_LOST) {
        /*Refresh the state*/
        if(ext->state == GUI_BTN_STATE_PR) btn_set_state(btn, GUI_BTN_STATE_REL);
        else if(ext->state == GUI_BTN_STATE_TGL_PR) btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        /*When the button begins to drag revert pressed states to released*/
        if(indev_is_dragging(param) != false) {
            if(ext->state == GUI_BTN_STATE_PR) btn_set_state(btn, GUI_BTN_STATE_REL);
            else if(ext->state == GUI_BTN_STATE_TGL_PR) btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
        }
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        /*If not dragged and it was not long press action then
         *change state and run the action*/
        if(indev_is_dragging(param) == false && ext->long_pr_action_executed == 0) {
            if(ext->state == GUI_BTN_STATE_PR && tgl == false) {
                btn_set_state(btn, GUI_BTN_STATE_REL);
            } else if(ext->state == GUI_BTN_STATE_TGL_PR && tgl == false) {
                btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
            } else if(ext->state == GUI_BTN_STATE_PR && tgl == true) {
                btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
            } else if(ext->state == GUI_BTN_STATE_TGL_PR && tgl == true) {
                btn_set_state(btn, GUI_BTN_STATE_REL);
            }

            if(ext->actions[GUI_BTN_ACTION_CLICK] && state != GUI_BTN_STATE_INA) {
                res = ext->actions[GUI_BTN_ACTION_CLICK](btn);
            }
        } else { /*If dragged change back the state*/
            if(ext->state == GUI_BTN_STATE_PR) {
                btn_set_state(btn, GUI_BTN_STATE_REL);
            } else if(ext->state == GUI_BTN_STATE_TGL_PR) {
                btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
            }
        }
    }
    else if(sign == GUI_SIGNAL_LONG_PRESS) {
            if(ext->actions[GUI_BTN_ACTION_LONG_PR] && state != GUI_BTN_STATE_INA) {
                ext->long_pr_action_executed = 1;
                res = ext->actions[GUI_BTN_ACTION_LONG_PR](btn);
            }
    }
    else if(sign == GUI_SIGNAL_LONG_PRESS_REP) {
        if(ext->actions[GUI_BTN_ACTION_LONG_PR_REPEAT] && state != GUI_BTN_STATE_INA) {
            res = ext->actions[GUI_BTN_ACTION_LONG_PR_REPEAT](btn);
        }
    }
    else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT || c == GUI_GROUP_KEY_UP) {
            if(btn_get_toggle(btn) != false) btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
            if(ext->actions[GUI_BTN_ACTION_CLICK] && btn_get_state(btn) != GUI_BTN_STATE_INA) {
                res = ext->actions[GUI_BTN_ACTION_CLICK](btn);
            }
        } else if(c == GUI_GROUP_KEY_LEFT || c == GUI_GROUP_KEY_DOWN) {
            if(btn_get_toggle(btn) != false) btn_set_state(btn, GUI_BTN_STATE_REL);
            if(ext->actions[GUI_BTN_ACTION_CLICK] && btn_get_state(btn) != GUI_BTN_STATE_INA) {
                res = ext->actions[GUI_BTN_ACTION_CLICK](btn);
            }
        } else if(c == GUI_GROUP_KEY_ENTER) {
            if(!ext->long_pr_action_executed) {
                if(btn_get_toggle(btn)) {
                    if(state == GUI_BTN_STATE_REL) btn_set_state(btn, GUI_BTN_STATE_TGL_REL);
                    else if(state == GUI_BTN_STATE_PR) btn_set_state(btn, GUI_BTN_STATE_TGL_PR);
                    else if(state == GUI_BTN_STATE_TGL_REL) btn_set_state(btn, GUI_BTN_STATE_REL);
                    else if(state == GUI_BTN_STATE_TGL_PR) btn_set_state(btn, GUI_BTN_STATE_PR);
                }
                if(ext->actions[GUI_BTN_ACTION_CLICK] && state != GUI_BTN_STATE_INA) {
                    res = ext->actions[GUI_BTN_ACTION_CLICK](btn);
                }
            }
            ext->long_pr_action_executed  = 0;
        }
        else if(c == GUI_GROUP_KEY_ENTER_LONG) {
            if(ext->actions[GUI_BTN_ACTION_LONG_PR] && state != GUI_BTN_STATE_INA) {
                res = ext->actions[GUI_BTN_ACTION_LONG_PR](btn);
                ext->long_pr_action_executed = 1;
            }
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "btn";
    }

    return res;
}


#endif
