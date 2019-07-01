
/**
 * @file slider.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_SLIDER != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_SLIDER_SIZE_MIN      4           /*hor. pad and ver. pad cannot make the bar or indicator smaller then this [px]*/
#define GUI_SLIDER_NOT_PRESSED   INT16_MIN

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool slider_design(obj_t * slider, const area_t * mask, design_mode_t mode);
static res_t slider_signal(obj_t * slider, signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static design_func_t ancestor_design_f;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a slider objects
 * @param par pointer to an object, it will be the parent of the new slider
 * @param copy pointer to a slider object, if not NULL then the new object will be copied from it
 * @return pointer to the created slider
 */
obj_t * slider_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor slider*/
    obj_t * new_slider = bar_create(par, copy);
    RTE_AssertParam(new_slider);
    
    if(ancestor_design_f == NULL) ancestor_design_f = obj_get_design_func(new_slider);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_slider);

    /*Allocate the slider type specific extended data*/
    slider_ext_t * ext = obj_allocate_ext_attr(new_slider, sizeof(slider_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */
    ext->action = NULL;
    ext->drag_value = GUI_SLIDER_NOT_PRESSED;
    ext->style_knob = &style_pretty;
    ext->knob_in = 0;

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_slider, slider_signal);
    obj_set_design_func(new_slider, slider_design);

    /*Init the new slider slider*/
    if(copy == NULL) {
        obj_set_click(new_slider, true);
        obj_set_protect(new_slider, GUI_PROTECT_PRESS_LOST);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            slider_set_style(new_slider, GUI_SLIDER_STYLE_BG, th->slider.bg);
            slider_set_style(new_slider, GUI_SLIDER_STYLE_INDIC, th->slider.indic);
            slider_set_style(new_slider, GUI_SLIDER_STYLE_KNOB, th->slider.knob);
        } else {
            slider_set_style(new_slider, GUI_SLIDER_STYLE_KNOB, ext->style_knob);
        }
    }
    /*Copy an existing slider*/
    else {
    	slider_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->style_knob = copy_ext->style_knob;
        ext->action = copy_ext->action;
        ext->knob_in = copy_ext->knob_in;
        /*Refresh the style with new signal function*/
        obj_refresh_style(new_slider);
    }
    
    return new_slider;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a function which will be called when a new value is set on the slider
 * @param slider pointer to slider object
 * @param action a callback function
 */
void slider_set_action(obj_t * slider, action_t action)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);
    ext->action = action;
}

/**
 * Set the 'knob in' attribute of a slider
 * @param slider pointer to slider object
 * @param in true: the knob is drawn always in the slider;
 *           false: the knob can be out on the edges
 */
void slider_set_knob_in(obj_t * slider, bool in)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);
    if(ext->knob_in == in) return;

    ext->knob_in = in == false ? 0 : 1;
    obj_invalidate(slider);
}

/**
 * Set a style of a slider
 * @param slider pointer to a slider object
 * @param type which style should be set
 * @param style pointer to a style
 */
void slider_set_style(obj_t *slider, slider_style_t type, style_t *style)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);

    switch (type) {
        case GUI_SLIDER_STYLE_BG:
            bar_set_style(slider, GUI_BAR_STYLE_BG, style);
            break;
        case GUI_SLIDER_STYLE_INDIC:
            bar_set_style(slider, GUI_BAR_STYLE_INDIC, style);
            break;
        case GUI_SLIDER_STYLE_KNOB:
            ext->style_knob = style;
            obj_refresh_ext_size(slider);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a slider
 * @param slider pointer to a slider object
 * @return the value of the slider
 */
int16_t slider_get_value(obj_t * slider)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);

    if(ext->drag_value != GUI_SLIDER_NOT_PRESSED) return ext->drag_value;
    else return bar_get_value(slider);
}

/**
 * Get the slider action function
 * @param slider pointer to slider object
 * @return the callback function
 */
action_t slider_get_action(obj_t * slider)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);
    return ext->action;
}

/**
 * Give the slider is being dragged or not
 * @param slider pointer to a slider object
 * @return true: drag in progress false: not dragged
 */
bool slider_is_dragged(obj_t * slider)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);
    return ext->drag_value == GUI_SLIDER_NOT_PRESSED ? false : true;
}

/**
 * Get the 'knob in' attribute of a slider
 * @param slider pointer to slider object
 * @return true: the knob is drawn always in the slider;
 *         false: the knob can be out on the edges
 */
bool slider_get_knob_in(obj_t * slider)
{
    slider_ext_t * ext = obj_get_ext_attr(slider);
    return ext->knob_in == 0 ? false : true;
}

/**
 * Get a style of a slider
 * @param slider pointer to a slider object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * slider_get_style(obj_t *slider, slider_style_t type)
{
    slider_ext_t *ext = obj_get_ext_attr(slider);

    switch (type) {
        case GUI_SLIDER_STYLE_BG:    return bar_get_style(slider, GUI_BAR_STYLE_BG);
        case GUI_SLIDER_STYLE_INDIC: return bar_get_style(slider, GUI_BAR_STYLE_INDIC);
        case GUI_SLIDER_STYLE_KNOB:  return ext->style_knob;
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * Handle the drawing related tasks of the sliders
 * @param slider pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool slider_design(obj_t * slider, const area_t * mask, design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == GUI_DESIGN_COVER_CHK) {
    	return false;
    }
    /*Draw the object*/
    else if(mode == GUI_DESIGN_DRAW_MAIN) {
        slider_ext_t * ext = obj_get_ext_attr(slider);

        style_t * style_bg = slider_get_style(slider, GUI_SLIDER_STYLE_BG);
        style_t * style_knob = slider_get_style(slider, GUI_SLIDER_STYLE_KNOB);
        style_t * style_indic = slider_get_style(slider, GUI_SLIDER_STYLE_INDIC);

        coord_t slider_w = area_get_width(&slider->coords);
        coord_t slider_h = area_get_height(&slider->coords);

        /*Draw the bar*/
        area_t area_bg;
        area_copy(&area_bg, &slider->coords);

        /*Be sure at least GUI_SLIDER_SIZE_MIN  size will remain*/
        coord_t pad_ver_bg = style_bg->body.padding.ver;
        coord_t pad_hor_bg = style_bg->body.padding.hor;
        if(pad_ver_bg * 2 + GUI_SLIDER_SIZE_MIN > area_get_height(&area_bg)) {
            pad_ver_bg = (area_get_height(&area_bg) - GUI_SLIDER_SIZE_MIN) >> 1;
        }
        if(pad_hor_bg * 2 + GUI_SLIDER_SIZE_MIN > area_get_width(&area_bg)) {
            pad_hor_bg = (area_get_width(&area_bg) - GUI_SLIDER_SIZE_MIN) >> 1;
        }

        if(ext->knob_in) {  /*Enable extra size if the knob is inside */
            if(pad_hor_bg < 0) {
                area_bg.x1 += pad_hor_bg;
                area_bg.x2 -= pad_hor_bg;
            }
            if(pad_ver_bg < 0) {
                area_bg.y1 += pad_hor_bg;
                area_bg.y2 -= pad_hor_bg;
            }
        } else  { /*Let space only in the perpendicular directions*/
            area_bg.x1 += slider_w < slider_h ? pad_hor_bg : 0;   /*Pad only for vertical slider*/
            area_bg.x2 -= slider_w < slider_h ? pad_hor_bg : 0;   /*Pad only for vertical slider*/
            area_bg.y1 += slider_w > slider_h ? pad_ver_bg : 0;   /*Pad only for horizontal slider*/
            area_bg.y2 -= slider_w > slider_h ? pad_ver_bg : 0;   /*Pad only for horizontal slider*/
        }
        draw_rect(&area_bg, mask, style_bg);

        /*Draw the indicator*/
        area_t area_indic;
        area_copy(&area_indic, &area_bg);

        /*Be sure at least ver pad/hor pad width indicator will remain*/
        coord_t pad_ver_indic = style_indic->body.padding.ver;
        coord_t pad_hor_indic = style_indic->body.padding.hor;
        if(pad_ver_indic * 2 + GUI_SLIDER_SIZE_MIN > area_get_height(&area_bg)) {
            pad_ver_indic = (area_get_height(&area_bg) - GUI_SLIDER_SIZE_MIN) >> 1;
        }
        if(pad_hor_indic * 2 + GUI_SLIDER_SIZE_MIN > area_get_width(&area_bg)) {
            pad_hor_indic = (area_get_width(&area_bg) - GUI_SLIDER_SIZE_MIN) >> 1;
        }

        area_indic.x1 += pad_hor_indic;
        area_indic.x2 -= pad_hor_indic;
        area_indic.y1 += pad_ver_indic;
        area_indic.y2 -= pad_ver_indic;


        coord_t cur_value = slider_get_value(slider);
        coord_t min_value = slider_get_min_value(slider);
        coord_t max_value = slider_get_max_value(slider);

        /*If dragged draw to the drag position*/
        if(ext->drag_value != GUI_SLIDER_NOT_PRESSED) cur_value = ext->drag_value;

        if(slider_w >= slider_h) {
            area_indic.x2 = (int32_t) ((int32_t)area_get_width(&area_indic) * (cur_value - min_value)) / (max_value - min_value);
            area_indic.x2 = area_indic.x1 + area_indic.x2 - 1;

        } else {
            area_indic.y1 = (int32_t) ((int32_t)area_get_height(&area_indic) * (cur_value - min_value)) / (max_value - min_value);
            area_indic.y1 = area_indic.y2 - area_indic.y1 + 1;
        }

        if(cur_value != min_value) draw_rect(&area_indic, mask, style_indic);

        /*Draw the knob*/
        area_t knob_area;
        area_copy(&knob_area, &slider->coords);

        if(slider_w >= slider_h) {
            if(ext->knob_in == 0) {
                knob_area.x1 = area_indic.x2 - slider_h / 2;
                knob_area.x2 = knob_area.x1 + slider_h;
            } else {
                knob_area.x1 = (int32_t) ((int32_t)(slider_w - slider_h) * (cur_value - min_value)) / (max_value - min_value);
                knob_area.x1 += slider->coords.x1;
                knob_area.x2 = knob_area.x1 + slider_h;
            }

            knob_area.y1 = slider->coords.y1;
            knob_area.y2 = slider->coords.y2;
        } else {
            if(ext->knob_in == 0) {
                knob_area.y1 = area_indic.y1 - slider_w / 2;
                knob_area.y2 = knob_area.y1 + slider_w;
            } else {
                knob_area.y2 = (int32_t) ((int32_t)(slider_h - slider_w) * (cur_value - min_value)) / (max_value - min_value);
                knob_area.y2 = slider->coords.y2 - knob_area.y2;
                knob_area.y1 = knob_area.y2 - slider_w;
            }
            knob_area.x1 = slider->coords.x1;
            knob_area.x2 = slider->coords.x2;

        }

        draw_rect(&knob_area, mask, style_knob);

    }
    /*Post draw when the children are drawn*/
    else if(mode == GUI_DESIGN_DRAW_POST) {

    }

    return true;
}

/**
 * Signal function of the slider
 * @param slider pointer to a slider object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t slider_signal(obj_t * slider, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(slider, sign, param);
    if(res != GUI_RES_OK) return res;

    slider_ext_t * ext = obj_get_ext_attr(slider);
    point_t p;
    coord_t w = obj_get_width(slider);
    coord_t h = obj_get_height(slider);

    if(sign == GUI_SIGNAL_PRESSED) {
        ext->drag_value = slider_get_value(slider);
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        indev_get_point(param, &p);
        int16_t tmp = 0;
        if(w > h) {
            coord_t knob_w = h;
            p.x -= slider->coords.x1 + h / 2;    /*Modify the point to shift with half knob (important on the start and end)*/
            tmp = (int32_t) ((int32_t) p.x * (ext->bar.max_value - ext->bar.min_value + 1)) / (w - knob_w);
            tmp += ext->bar.min_value;
        } else {
            coord_t knob_h = w;
            p.y -= slider->coords.y1 + w / 2;    /*Modify the point to shift with half knob (important on the start and end)*/
            tmp = (int32_t) ((int32_t) p.y * (ext->bar.max_value - ext->bar.min_value + 1)) / (h - knob_h);
            tmp = ext->bar.max_value - tmp;     /*Invert the value: smaller value means higher y*/
        }

        if(tmp < ext->bar.min_value) tmp = ext->bar.min_value;
        else if(tmp > ext->bar.max_value) tmp = ext->bar.max_value;

        if(tmp != ext->drag_value) {
            ext->drag_value = tmp;
            if(ext->action != NULL) ext->action(slider);
            obj_invalidate(slider);
        }
    }
    else if (sign == GUI_SIGNAL_RELEASED || sign == GUI_SIGNAL_PRESS_LOST) {
        slider_set_value(slider, ext->drag_value);
        ext->drag_value = GUI_SLIDER_NOT_PRESSED;
        if(ext->action != NULL) ext->action(slider);
    }
    else if(sign == GUI_SIGNAL_CORD_CHG) {
        /* The knob size depends on slider size.
         * During the drawing method the ext. size is used by the knob so refresh the ext. size.*/
        if(obj_get_width(slider) != area_get_width(param) ||
          obj_get_height(slider) != area_get_height(param)) {
            slider->signal_func(slider, GUI_SIGNAL_REFR_EXT_SIZE, NULL);
        }
    } else if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        style_t *style = slider_get_style(slider, GUI_SLIDER_STYLE_BG);
        style_t *knob_style = slider_get_style(slider, GUI_SLIDER_STYLE_KNOB);
        coord_t shadow_w = knob_style->body.shadow.width;
        if(ext->knob_in == 0) {
            /* The smaller size is the knob diameter*/
            coord_t x = RTE_MATH_MIN(w / 2 + 1 + shadow_w, h / 2 + 1 + shadow_w);
            if(slider->ext_size < x) slider->ext_size = x;
        } else {
            coord_t pad = RTE_MATH_MIN(style->body.padding.hor, style->body.padding.ver);
            if(pad < 0) pad = -pad;
            if(slider->ext_size < pad) slider->ext_size = pad;

            if(slider->ext_size < shadow_w) slider->ext_size = shadow_w;
        }
    } else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT || c == GUI_GROUP_KEY_UP) {
            slider_set_value(slider, slider_get_value(slider) + 1);
            if(ext->action != NULL) ext->action(slider);
        } else if(c == GUI_GROUP_KEY_LEFT || c == GUI_GROUP_KEY_DOWN) {
            slider_set_value(slider, slider_get_value(slider) - 1);
            if(ext->action != NULL) ext->action(slider);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "slider";
    }

    return res;
}
#endif
