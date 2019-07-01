/**
 * @file sw.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_SW != 0

/*Testing of dependencies*/
#if GUI_USE_SLIDER == 0
#error "sw: slider is required. Enable it in conf.h (GUI_USE_SLIDER  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t sw_signal(obj_t * sw, signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a switch objects
 * @param par pointer to an object, it will be the parent of the new switch
 * @param copy pointer to a switch object, if not NULL then the new object will be copied from it
 * @return pointer to the created switch
 */
obj_t * sw_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor of switch*/
    obj_t * new_sw = slider_create(par, copy);
    RTE_AssertParam(new_sw);

    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_sw);
    
    /*Allocate the switch type specific extended data*/
    sw_ext_t * ext = obj_allocate_ext_attr(new_sw, sizeof(sw_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */
    ext->changed = 0;
    ext->style_knob_off = ext->slider.style_knob;
    ext->style_knob_on = ext->slider.style_knob;

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_sw, sw_signal);

    /*Init the new switch switch*/
    if(copy == NULL) {
        slider_set_range(new_sw, 0, 1);
        obj_set_size(new_sw, 2 * GUI_DPI / 3, GUI_DPI / 3);
        slider_set_knob_in(new_sw, true);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            sw_set_style(new_sw, GUI_SW_STYLE_BG, th->sw.bg);
            sw_set_style(new_sw, GUI_SW_STYLE_INDIC, th->sw.indic);
            sw_set_style(new_sw, GUI_SW_STYLE_KNOB_OFF, th->sw.knob_off);
            sw_set_style(new_sw, GUI_SW_STYLE_KNOB_ON, th->sw.knob_on);
        } else {
            /*Let the slider' style*/
        }

    }
    /*Copy an existing switch*/
    else {
        sw_ext_t *copy_ext = obj_get_ext_attr(copy);
        ext->style_knob_off = copy_ext->style_knob_off;
        ext->style_knob_on = copy_ext->style_knob_on;

        if(sw_get_state(new_sw)) slider_set_style(new_sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_on);
        else slider_set_style(new_sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_off);
        /*Refresh the style with new signal function*/
        obj_refresh_style(new_sw);
    }
    
    return new_sw;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Turn ON the switch
 * @param sw pointer to a switch object
 */
void sw_on(obj_t *sw)
{
	if(sw_get_state(sw)) return;		/*Do nothing is already turned on*/

    sw_ext_t *ext = obj_get_ext_attr(sw);
    slider_set_value(sw, 1);
    slider_set_style(sw, GUI_SLIDER_STYLE_KNOB,ext->style_knob_on);
}

/**
 * Turn OFF the switch
 * @param sw pointer to a switch object
 */
void sw_off(obj_t *sw)
{
	if(!sw_get_state(sw)) return;	/*Do nothing is already turned off*/

    sw_ext_t *ext = obj_get_ext_attr(sw);
    slider_set_value(sw, 0);
    slider_set_style(sw, GUI_SLIDER_STYLE_KNOB,ext->style_knob_off);
}

/**
 * Set a style of a switch
 * @param sw pointer to a switch object
 * @param type which style should be set
 * @param style pointer to a style
 */
void sw_set_style(obj_t *sw, sw_style_t type, style_t *style)
{
    sw_ext_t * ext = obj_get_ext_attr(sw);

    switch (type) {
        case GUI_SLIDER_STYLE_BG:
            slider_set_style(sw, GUI_SLIDER_STYLE_BG, style);
            break;
        case GUI_SLIDER_STYLE_INDIC:
            bar_set_style(sw, (bar_style_t)GUI_SLIDER_STYLE_INDIC, style);
            break;
        case GUI_SW_STYLE_KNOB_OFF:
            ext->style_knob_off = style;
            if(sw_get_state(sw) == 0) slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, style);
            break;
        case GUI_SW_STYLE_KNOB_ON:
            ext->style_knob_on = style;
            if(sw_get_state(sw) != 0) slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, style);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get a style of a switch
 * @param sw pointer to a  switch object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * sw_get_style(obj_t *sw, sw_style_t type)
{
    sw_ext_t *ext = obj_get_ext_attr(sw);

    switch (type) {
        case GUI_SW_STYLE_BG:    return slider_get_style(sw, GUI_SLIDER_STYLE_BG);
        case GUI_SW_STYLE_INDIC: return slider_get_style(sw, GUI_SLIDER_STYLE_INDIC);
        case GUI_SW_STYLE_KNOB_OFF:  return ext->style_knob_off;
        case GUI_SW_STYLE_KNOB_ON:  return ext->style_knob_on;
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the switch
 * @param sw pointer to a switch object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t sw_signal(obj_t * sw, signal_t sign, void * param)
{
    sw_ext_t * ext = obj_get_ext_attr(sw);

    /*Save the current (old) value before slider signal modifies it*/
    int16_t old_val;
    if(sign == GUI_SIGNAL_PRESSING) old_val = ext->slider.drag_value;
    else old_val = slider_get_value(sw);

    /*Do not let the slider to call the callback. The Switch will do it if required*/
    action_t slider_action = ext->slider.action;
    ext->slider.action = NULL;

    res_t res;
    /* Include the ancient signal function */
    res = ancestor_signal(sw, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_CLEANUP) {
        /*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        int16_t act_val = ext->slider.drag_value;
        if(act_val != old_val) ext->changed = 1;
    }
    else if(sign == GUI_SIGNAL_PRESS_LOST) {
        ext->changed = 0;
        if(sw_get_state(sw)) slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_on);
        else slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_off);
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        if(ext->changed == 0) {
            int16_t v = slider_get_value(sw);
            if(v == 0) slider_set_value(sw, 1);
            else slider_set_value(sw, 0);
        }

        if(sw_get_state(sw)) slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_on);
        else slider_set_style(sw, GUI_SLIDER_STYLE_KNOB, ext->style_knob_off);

        if(slider_action != NULL) slider_action(sw);

        ext->changed = 0;
    }
    else if(sign == GUI_SIGNAL_CONTROLL) {

        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_ENTER || c == GUI_GROUP_KEY_ENTER_LONG) {
            if(sw_get_state(sw)) sw_off(sw);
            else sw_on(sw);

            if(slider_action) slider_action(sw);
        }
        else if(c == GUI_GROUP_KEY_UP || c== GUI_GROUP_KEY_RIGHT) {
            sw_on(sw);
            if(slider_action) slider_action(sw);
        }
        else if(c == GUI_GROUP_KEY_DOWN || c== GUI_GROUP_KEY_LEFT) {
            sw_off(sw);
            if(slider_action) slider_action(sw);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "sw";
    }

    /*Restore the callback*/
    ext->slider.action = slider_action;

    return res;
}

#endif
