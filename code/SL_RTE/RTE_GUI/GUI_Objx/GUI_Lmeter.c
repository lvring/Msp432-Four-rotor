/**
 * @file lmeter.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LMETER != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_LMETER_LINE_UPSCALE          5                   /*2^x upscale of line to make rounding*/
#define GUI_LMETER_LINE_UPSCALE_MASK     ((1 << GUI_LMETER_LINE_UPSCALE) - 1)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool lmeter_design(obj_t * lmeter, const area_t * mask, design_mode_t mode);
static res_t lmeter_signal(obj_t * lmeter, signal_t sign, void * param);
static coord_t lmeter_coord_round(int32_t x);

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
 * Create a line meter objects
 * @param par pointer to an object, it will be the parent of the new line meter
 * @param copy pointer to a line meter object, if not NULL then the new object will be copied from it
 * @return pointer to the created line meter
 */
obj_t * lmeter_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor of line meter*/
    obj_t * new_lmeter = obj_create(par, copy);
    RTE_AssertParam(new_lmeter);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_lmeter);
    
    /*Allocate the line meter type specific extended data*/
    lmeter_ext_t * ext = obj_allocate_ext_attr(new_lmeter, sizeof(lmeter_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */
    ext->min_value = 0;
    ext->max_value = 100;
    ext->cur_value = 0;
    ext->line_cnt = 31;    /*Odd scale number looks better*/
    ext->scale_angle = 240; /*(scale_num - 1) * N looks better */

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_lmeter, lmeter_signal);
    obj_set_design_func(new_lmeter, lmeter_design);

    /*Init the new line meter line meter*/
    if(copy == NULL) {
        obj_set_size(new_lmeter, GUI_DPI, GUI_DPI);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            lmeter_set_style(new_lmeter, th->lmeter);
        } else {
            lmeter_set_style(new_lmeter, &style_pretty_color);
        }
    }
    /*Copy an existing line meter*/
    else {
    	lmeter_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->scale_angle = copy_ext->scale_angle;
        ext->line_cnt = copy_ext->line_cnt;
        ext->min_value = copy_ext->min_value;
        ext->max_value = copy_ext->max_value;
        ext->cur_value = copy_ext->cur_value;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_lmeter);
    }
    
    return new_lmeter;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the line meter
 * @param lmeter pointer to a line meter object
 * @param value new value
 */
void lmeter_set_value(obj_t *lmeter, int16_t value)
{
	lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    if(ext->cur_value == value) return;

	ext->cur_value = value > ext->max_value ? ext->max_value : value;
    ext->cur_value = ext->cur_value < ext->min_value ? ext->min_value : ext->cur_value;
    obj_invalidate(lmeter);
}

/**
 * Set minimum and the maximum values of a line meter
 * @param lmeter pointer to he line meter object
 * @param min minimum value
 * @param max maximum value
 */
void lmeter_set_range(obj_t *lmeter, int16_t min, int16_t max)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    if(ext->min_value == min && ext->max_value == max) return;

    ext->max_value = max;
    ext->min_value = min;
    if(ext->cur_value > max) {
        ext->cur_value = max;
        lmeter_set_value(lmeter, ext->cur_value);
    }
    if(ext->cur_value < min) {
        ext->cur_value = min;
        lmeter_set_value(lmeter, ext->cur_value);
    }
    obj_invalidate(lmeter);
}

/**
 * Set the scale settings of a line meter
 * @param lmeter pointer to a line meter object
 * @param angle angle of the scale (0..360)
 * @param line_cnt number of lines
 */
void lmeter_set_scale(obj_t * lmeter, uint16_t angle, uint8_t line_cnt)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    if(ext->scale_angle == angle && ext->line_cnt == line_cnt) return;

    ext->scale_angle = angle;
    ext->line_cnt = line_cnt;

    obj_invalidate(lmeter);
}


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the value of the line meter
 */
int16_t lmeter_get_value(obj_t *lmeter)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    return ext->cur_value;
}

/**
 * Get the minimum value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the minimum value of the line meter
 */
int16_t lmeter_get_min_value(obj_t * lmeter)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    return ext->min_value;
}

/**
 * Get the maximum value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the maximum value of the line meter
 */
int16_t lmeter_get_max_value(obj_t * lmeter)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    return ext->max_value;
}

/**
 * Get the scale number of a line meter
 * @param lmeter pointer to a line meter object
 * @return number of the scale units
 */
uint8_t lmeter_get_line_count(obj_t * lmeter)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    return ext->line_cnt ;
}

/**
 * Get the scale angle of a line meter
 * @param lmeter pointer to a line meter object
 * @return angle of the scale
 */
uint16_t lmeter_get_scale_angle(obj_t * lmeter)
{
    lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
    return ext->scale_angle;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * Handle the drawing related tasks of the line meters
 * @param lmeter pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool lmeter_design(obj_t * lmeter, const area_t * mask, design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == GUI_DESIGN_COVER_CHK) {
    	return false;
    }
    /*Draw the object*/
    else if(mode == GUI_DESIGN_DRAW_MAIN) {
        lmeter_ext_t * ext = obj_get_ext_attr(lmeter);
        style_t * style = obj_get_style(lmeter);
        style_t style_tmp;
        memcpy(&style_tmp, style, sizeof(style_t));


#if GUI_USE_GROUP
        group_t *g = obj_get_group(lmeter);
        if(group_get_focused(g) == lmeter) {
            style_tmp.line.width += 1;
        }
#endif

         coord_t r_out = obj_get_width(lmeter) / 2;
         coord_t r_in = r_out - style->body.padding.hor;
         if(r_in < 1) r_in = 1;

         coord_t x_ofs = obj_get_width(lmeter) / 2 + lmeter->coords.x1;
         coord_t y_ofs = obj_get_height(lmeter) / 2 + lmeter->coords.y1;
         int16_t angle_ofs = 90 + (360 - ext->scale_angle) / 2;
				
         int16_t level = (int32_t)((int32_t)(ext->cur_value - ext->min_value) * ext->line_cnt) / (ext->max_value - ext->min_value);
         uint8_t i;

         style_tmp.line.color = style->body.main_color;

         /*Calculate every coordinate in x32 size to make rounding later*/
         r_out = r_out << GUI_LMETER_LINE_UPSCALE;
         r_in = r_in << GUI_LMETER_LINE_UPSCALE;

         for(i = 0; i < ext->line_cnt; i++) {
             /*Calculate the position a scale label*/
             int16_t angle = (i * ext->scale_angle) / (ext->line_cnt - 1) + angle_ofs;

             coord_t y_out = (int32_t)((int32_t)trigo_sin(angle) * r_out) >> GUI_TRIGO_SHIFT;
             coord_t x_out = (int32_t)((int32_t)trigo_sin(angle + 90) * r_out) >> GUI_TRIGO_SHIFT;
             coord_t y_in = (int32_t)((int32_t)trigo_sin(angle) * r_in) >> GUI_TRIGO_SHIFT;
             coord_t x_in = (int32_t)((int32_t)trigo_sin(angle + 90) * r_in) >> GUI_TRIGO_SHIFT;

             /*Rounding*/
             x_out = lmeter_coord_round(x_out);
             x_in  = lmeter_coord_round(x_in);
             y_out = lmeter_coord_round(y_out);
             y_in  = lmeter_coord_round(y_in);

             point_t p1;
             point_t p2;

             p2.x = x_in + x_ofs;
             p2.y = y_in +  y_ofs;

             p1.x = x_out+ x_ofs;
             p1.y = y_out + y_ofs;

             if(i >= level) style_tmp.line.color = style->line.color;
             else {
                 style_tmp.line.color = color_mix(style->body.grad_color, style->body.main_color, (255 * i) /  ext->line_cnt);
             }

             draw_line(&p1, &p2, mask, &style_tmp);
         }

    }
    /*Post draw when the children are drawn*/
    else if(mode == GUI_DESIGN_DRAW_POST) {

    }

    return true;
}

/**
 * Signal function of the line meter
 * @param lmeter pointer to a line meter object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t lmeter_signal(obj_t * lmeter, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(lmeter, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_CLEANUP) {
        /*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "lmeter";
    }

    return res;
}

/**
 * Round a coordinate which is upscaled  (>=x.5 -> x + 1;   <x.5 -> x)
 * @param x a coordinate which is greater then it should be
 * @return the downscaled and rounded coordinate  (+-1)
 */
static coord_t lmeter_coord_round(int32_t x)
{
#if GUI_LMETER_LINE_UPSCALE > 0
    bool was_negative;
    if(x < 0) {
        was_negative = true;
        x = -x;
    } else {
        was_negative = false;
    }

    x = (x >> GUI_LMETER_LINE_UPSCALE) + ((x & GUI_LMETER_LINE_UPSCALE_MASK) >> (GUI_LMETER_LINE_UPSCALE - 1));

    if(was_negative) x = -x;

    return x;
#else
    return x;
#endif
}

#endif
