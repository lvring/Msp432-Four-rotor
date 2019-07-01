

/**
 * @file bar.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_BAR != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool bar_design(obj_t * bar, const area_t * mask, design_mode_t mode);
static res_t bar_signal(obj_t * bar, signal_t sign, void * param);

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
 * Create a bar objects
 * @param par pointer to an object, it will be the parent of the new bar
 * @param copy pointer to a bar object, if not NULL then the new object will be copied from it
 * @return pointer to the created bar
 */
obj_t * bar_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor basic object*/
    obj_t * new_bar = obj_create(par, copy);
    RTE_AssertParam(new_bar);

    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_bar);
    if(ancestor_design_f == NULL) ancestor_design_f = obj_get_design_func(new_bar);

    /*Allocate the object type specific extended data*/
    bar_ext_t * ext = obj_allocate_ext_attr(new_bar, sizeof(bar_ext_t));
    RTE_AssertParam(ext);
    ext->min_value = 0;
    ext->max_value = 100;
    ext->cur_value = 0;
    ext->style_indic = &style_pretty_color;

    obj_set_signal_func(new_bar, bar_signal);
    obj_set_design_func(new_bar, bar_design);

    /*Init the new  bar object*/
    if(copy == NULL) {
        obj_set_click(new_bar, false);
    	obj_set_size(new_bar, GUI_DPI * 2, GUI_DPI / 3);
    	bar_set_value(new_bar, ext->cur_value);

    	theme_t *th = theme_get_current();
    	if(th) {
    	    bar_set_style(new_bar, GUI_BAR_STYLE_BG, th->bar.bg);
            bar_set_style(new_bar, GUI_BAR_STYLE_INDIC, th->bar.indic);
    	} else {
            obj_set_style(new_bar, &style_pretty);
    	}
    } else {
    	bar_ext_t * ext_copy = obj_get_ext_attr(copy);
		ext->min_value = ext_copy->min_value;
		ext->max_value = ext_copy->max_value;
		ext->cur_value = ext_copy->cur_value;
        ext->style_indic = ext_copy->style_indic;
        /*Refresh the style with new signal function*/
        obj_refresh_style(new_bar);

        bar_set_value(new_bar, ext->cur_value);
    }
    return new_bar;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the bar
 * @param bar pointer to a bar object
 * @param value new value
 */
void bar_set_value(obj_t * bar, int16_t value)
{
	bar_ext_t * ext = obj_get_ext_attr(bar);
	if(ext->cur_value == value) return;

	ext->cur_value = value > ext->max_value ? ext->max_value : value;
    ext->cur_value = ext->cur_value < ext->min_value ? ext->min_value : ext->cur_value;
	obj_invalidate(bar);
}

#if GUI_USE_ANIMATION
/**
 * Set a new value with animation on the bar
 * @param bar pointer to a bar object
 * @param value new value
 * @param anim_time animation time in milliseconds
 */
void bar_set_value_anim(obj_t * bar, int16_t value, uint16_t anim_time)
{
    bar_ext_t * ext = obj_get_ext_attr(bar);
	if(ext->cur_value == value) return;

    int16_t new_value;
    new_value = value > ext->max_value ? ext->max_value : value;
    new_value = new_value < ext->min_value ? ext->min_value : new_value;

    anim_t a;
    a.var = bar;
    a.start = ext->cur_value;
    a.end = new_value;
    a.fp = (anim_fp_t)bar_set_value;
    a.path = anim_path_linear;
    a.end_cb = NULL;
    a.act_time = 0;
    a.time = anim_time;
    a.playback = 0;
    a.playback_pause = 0;
    a.repeat = 0;
    a.repeat_pause = 0;

    anim_create(&a);
}
#endif


/**
 * Set minimum and the maximum values of a bar
 * @param bar pointer to the bar object
 * @param min minimum value
 * @param max maximum value
 */
void bar_set_range(obj_t * bar, int16_t min, int16_t max)
{
	bar_ext_t * ext = obj_get_ext_attr(bar);
	if(ext->min_value == min && ext->max_value == max) return;

	ext->max_value = max;
	ext->min_value = min;
	if(ext->cur_value > max) {
		ext->cur_value = max;
		bar_set_value(bar, ext->cur_value);
	}
    if(ext->cur_value < min) {
        ext->cur_value = min;
        bar_set_value(bar, ext->cur_value);
    }
	obj_invalidate(bar);
}


/**
 * Set a style of a bar
 * @param bar pointer to a bar object
 * @param type which style should be set
 * @param style pointer to a style
 */
void bar_set_style(obj_t *bar, bar_style_t type, style_t *style)
{
    bar_ext_t * ext = obj_get_ext_attr(bar);

    switch (type) {
        case GUI_BAR_STYLE_BG:
            obj_set_style(bar, style);
            break;
        case GUI_BAR_STYLE_INDIC:
            ext->style_indic = style;
            obj_refresh_ext_size(bar);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a bar
 * @param bar pointer to a bar object
 * @return the value of the bar
 */
int16_t bar_get_value(obj_t * bar)
{
	bar_ext_t * ext = obj_get_ext_attr(bar);
	return ext->cur_value;
}

/**
 * Get the minimum value of a bar
 * @param bar pointer to a bar object
 * @return the minimum value of the bar
 */
int16_t bar_get_min_value(obj_t * bar)
{
    bar_ext_t * ext = obj_get_ext_attr(bar);
    return ext->min_value;
}

/**
 * Get the maximum value of a bar
 * @param bar pointer to a bar object
 * @return the maximum value of the bar
 */
int16_t bar_get_max_value(obj_t * bar)
{
    bar_ext_t * ext = obj_get_ext_attr(bar);
    return ext->max_value;
}

/**
 * Get a style of a bar
 * @param bar pointer to a bar object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * bar_get_style(obj_t *bar, bar_style_t type)
{
    bar_ext_t *ext = obj_get_ext_attr(bar);

    switch (type) {
        case GUI_BAR_STYLE_BG:    return obj_get_style(bar);
        case GUI_BAR_STYLE_INDIC: return ext->style_indic;
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the bars
 * @param bar pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool bar_design(obj_t * bar, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
    	/*Return false if the object is not covers the mask area*/
    	return  ancestor_design_f(bar, mask, mode);;
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
        ancestor_design_f(bar, mask, mode);

		bar_ext_t * ext = obj_get_ext_attr(bar);

        style_t *style_indic = bar_get_style(bar, GUI_BAR_STYLE_INDIC);
		area_t indic_area;
		area_copy(&indic_area, &bar->coords);
		indic_area.x1 += style_indic->body.padding.hor;
		indic_area.x2 -= style_indic->body.padding.hor;
		indic_area.y1 += style_indic->body.padding.ver;
		indic_area.y2 -= style_indic->body.padding.ver;

		coord_t w = area_get_width(&indic_area);
        coord_t h = area_get_height(&indic_area);

		if(w >= h) {
		    indic_area.x2 = (int32_t) ((int32_t)w * (ext->cur_value - ext->min_value)) / (ext->max_value - ext->min_value);
            indic_area.x2 = indic_area.x1 + indic_area.x2 - 1;
		} else {
		    indic_area.y1 = (int32_t) ((int32_t)h * (ext->cur_value - ext->min_value)) / (ext->max_value - ext->min_value);
            indic_area.y1 = indic_area.y2 - indic_area.y1 + 1;
		}

		/*Draw the indicator*/
        draw_rect(&indic_area, mask, style_indic);
    }
    return true;
}

/**
 * Signal function of the bar
 * @param bar pointer to a bar object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t bar_signal(obj_t * bar, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(bar, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        style_t * style_indic = bar_get_style(bar, GUI_BAR_STYLE_INDIC);
        if(style_indic->body.shadow.width > bar->ext_size) bar->ext_size = style_indic->body.shadow.width;
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "bar";
    }

    return res;
}


#endif
