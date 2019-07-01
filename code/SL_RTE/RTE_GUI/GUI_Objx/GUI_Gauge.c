/**
 * @file gauge.c
 *
 */


/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_GAUGE != 0


/*********************
 *      DEFINES
 *********************/
#define GUI_GAUGE_DEF_NEEDLE_COLOR       GUI_COLOR_RED
#define GUI_GAUGE_DEF_LABEL_COUNT        6
#define GUI_GAUGE_DEF_LINE_COUNT         21      /*Should be: ((label_cnt - 1) * internal_lines) + 1*/
#define GUI_GAUGE_DEF_ANGLE              220


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool gauge_design(obj_t * gauge, const area_t * mask, design_mode_t mode);
static res_t gauge_signal(obj_t * gauge, signal_t sign, void * param);
static void gauge_draw_scale(obj_t * gauge, const area_t * mask);
static void gauge_draw_needle(obj_t * gauge, const area_t * mask);

/**********************
 *  STATIC VARIABLES
 **********************/
static design_func_t ancestor_design;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a gauge objects
 * @param par pointer to an object, it will be the parent of the new gauge
 * @param copy pointer to a gauge object, if not NULL then the new object will be copied from it
 * @return pointer to the created gauge
 */
obj_t * gauge_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor gauge*/
    obj_t * new_gauge = lmeter_create(par, copy);
    RTE_AssertParam(new_gauge);

    /*Allocate the gauge type specific extended data*/
    gauge_ext_t * ext = obj_allocate_ext_attr(new_gauge, sizeof(gauge_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */
		ext->rotate_angle = 0;
    ext->needle_count = 0;
    ext->values = NULL;
    ext->needle_colors = NULL;
    ext->label_count = GUI_GAUGE_DEF_LABEL_COUNT;
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_gauge);
    if(ancestor_design == NULL) ancestor_design = obj_get_design_func(new_gauge);

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_gauge, gauge_signal);
    obj_set_design_func(new_gauge, gauge_design);

    /*Init the new gauge gauge*/
    if(copy == NULL) {
        gauge_set_scale(new_gauge, GUI_GAUGE_DEF_ANGLE, GUI_GAUGE_DEF_LINE_COUNT, GUI_GAUGE_DEF_LABEL_COUNT);
        gauge_set_needle_count(new_gauge, 1, NULL , NULL);
        gauge_set_critical_value(new_gauge, 80);
        obj_set_size(new_gauge, 2 * GUI_DPI, 2 * GUI_DPI);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            gauge_set_style(new_gauge, th->gauge);
        } else {
            gauge_set_style(new_gauge, &style_pretty_color);
        }
    }
    /*Copy an existing gauge*/
    else {
    	gauge_ext_t * copy_ext = obj_get_ext_attr(copy);
        gauge_set_needle_count(new_gauge, copy_ext->needle_count, copy_ext->needle_colors, copy_ext->needle_lengths);

        uint8_t i;
        for(i = 0; i < ext->needle_count; i++) {
            ext->values[i] = copy_ext->values[i];
        }
        ext->label_count = copy_ext->label_count;
        /*Refresh the style with new signal function*/
        obj_refresh_style(new_gauge);
    }

    return new_gauge;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the number of needles
 * @param gauge pointer to gauge object
 * @param needle_cnt new count of needles
 * @param colors an array of colors for needles (with 'num' elements)
 */
void gauge_set_needle_count(obj_t * gauge, uint8_t needle_cnt, const color_t * colors, const uint16_t * lengths)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);

    if(ext->needle_count != needle_cnt) {
		if(ext->values != NULL) {
			RTE_BRel(MEM_RTE,ext->values);
			ext->values = NULL;
		}

		ext->values = RTE_BGetr(MEM_RTE,ext->values, needle_cnt * sizeof(int16_t));

		int16_t min = gauge_get_min_value(gauge);
		uint8_t n;
		for(n = ext->needle_count; n < needle_cnt; n++) {
			ext->values[n] = min;
		}

		ext->needle_count = needle_cnt;
    }

    ext->needle_colors = colors;
		ext->needle_lengths = lengths;
    obj_invalidate(gauge);
}

/**
 * Set the value of a needle
 * @param gauge pointer to a gauge
 * @param needle_id the id of the needle
 * @param value the new value
 */
void gauge_set_value(obj_t * gauge, uint8_t needle_id, int16_t value)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);

    if(needle_id >= ext->needle_count) return;
    if(ext->values[needle_id] == value) return;


    int16_t min = gauge_get_min_value(gauge);
    int16_t max = gauge_get_max_value(gauge);

    if(value > max) value = max;
    else if(value < min) value = min;

    ext->values[needle_id] = value;


    obj_invalidate(gauge);
}
/**
 * Set the value of a rotate angle
 * @param gauge pointer to a gauge
 * @param value the new angle
 */
void gauge_set_rotateangle(obj_t * gauge, uint16_t angle)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);
		ext->rotate_angle = angle;
    obj_invalidate(gauge);
}
/**
 * Set the scale settings of a gauge
 * @param gauge pointer to a gauge object
 * @param angle angle of the scale (0..360)
 * @param line_cnt count of scale lines
 * @param label_cnt count of scale labels
 */
void gauge_set_scale(obj_t * gauge, uint16_t angle, uint8_t line_cnt, uint8_t label_cnt)
{
    lmeter_set_scale(gauge, angle, line_cnt);

    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    ext->label_count = label_cnt;
    obj_invalidate(gauge);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a needle
 * @param gauge pointer to gauge object
 * @param needle the id of the needle
 * @return the value of the needle [min,max]
 */
int16_t gauge_get_value(obj_t * gauge,  uint8_t needle)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    int16_t min = gauge_get_min_value(gauge);

    if(needle >= ext->needle_count) return min;

    return ext->values[needle];
}

/**
 * Get the count of needles on a gauge
 * @param gauge pointer to gauge
 * @return count of needles
 */
uint8_t gauge_get_needle_count(obj_t * gauge)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    return ext->needle_count;
}

/**
 * Set the number of labels (and the thicker lines too)
 * @param gauge pointer to a gauge object
 * @return count of labels
 */
uint8_t gauge_get_label_count(obj_t * gauge)
{
    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    return ext->label_count;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the gauges
 * @param gauge pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool gauge_design(obj_t * gauge, const area_t * mask, design_mode_t mode)
{

    /*Return false if the object is not covers the mask_p area*/
    if(mode == GUI_DESIGN_COVER_CHK) {
    	return false;
    }
    /*Draw the object*/
    else if(mode == GUI_DESIGN_DRAW_MAIN) {

        /* Store the real pointer because of 'group'
         * If the object is in focus 'obj_get_style()' will give a pointer to tmp style
         * and to the real object style. It is important because of style change tricks below*/
        style_t *style_ori_p = gauge->style_p;
        style_t *style = obj_get_style(gauge);
        gauge_ext_t *ext = obj_get_ext_attr(gauge);

        gauge_draw_scale(gauge, mask);

        /*Draw the ancestor line meter with max value to show the rainbow like line colors*/
        uint16_t line_cnt_tmp = ext->lmeter.line_cnt;
        ancestor_design(gauge, mask, mode);           /*To draw lines*/

        /*Temporally modify the line meter to draw thicker and longer lines where labels are*/
        style_t style_tmp;
        style_copy(&style_tmp, style);
        ext->lmeter.line_cnt = ext->label_count;                        /*Only to labels*/
        style_tmp.line.width = style_tmp.line.width * 2;                /*Ticker lines*/
        style_tmp.body.padding.hor = style_tmp.body.padding.hor * 2;    /*Longer lines*/
        gauge->style_p = &style_tmp;

        ancestor_design(gauge, mask, mode);           /*To draw lines*/

        ext->lmeter.line_cnt = line_cnt_tmp;          /*Restore the parameters*/
        gauge->style_p = style_ori_p;                 /*Restore the ORIGINAL style pointer*/

        gauge_draw_needle(gauge, mask);

    }
    /*Post draw when the children are drawn*/
    else if(mode == GUI_DESIGN_DRAW_POST) {
        ancestor_design(gauge, mask, mode);
    }

    return true;
}

/**
 * Signal function of the gauge
 * @param gauge pointer to a gauge object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t gauge_signal(obj_t * gauge, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(gauge, sign, param);
    if(res != GUI_RES_OK) return res;

    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    if(sign == GUI_SIGNAL_CLEANUP) {
        RTE_BRel(MEM_RTE,ext->values);
        ext->values = NULL;
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "gauge";
    }

    return res;
}

/**
 * Draw the scale on a gauge
 * @param gauge pointer to gauge object
 * @param mask mask of drawing
 */
static void gauge_draw_scale(obj_t * gauge, const area_t * mask)
{
    char scale_txt[16];

    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    style_t * style = obj_get_style(gauge);
    coord_t r = obj_get_width(gauge) / 2 - (3 * style->body.padding.hor) - style->body.padding.inner;
    coord_t x_ofs = obj_get_width(gauge) / 2 + gauge->coords.x1;
    coord_t y_ofs = obj_get_height(gauge) / 2 + gauge->coords.y1;
    int16_t scale_angle = lmeter_get_scale_angle(gauge);
    uint16_t label_num = ext->label_count;
		int16_t angle_ofs = 0;
		if(ext->rotate_angle)
			angle_ofs = ext->rotate_angle;
	else if(ext->rotate_angle==0)
			angle_ofs = 90 + (360 - scale_angle) / 2;
    int16_t min = gauge_get_min_value(gauge);
    int16_t max = gauge_get_max_value(gauge);

    uint8_t i;
    for(i = 0; i < label_num; i++) {
				if(scale_angle == 360 && i == (label_num - 1))
					break;
        /*Calculate the position a scale label*/
        int16_t angle = (i * scale_angle) / (label_num - 1) + angle_ofs;

        coord_t y = (int32_t)((int32_t)trigo_sin(angle) * r) / GUI_TRIGO_SIN_MAX;
        y += y_ofs;

        coord_t x = (int32_t)((int32_t)trigo_sin(angle + 90) * r) / GUI_TRIGO_SIN_MAX;
        x += x_ofs;

        int16_t scale_act = (int32_t)((int32_t)(max - min) * i) /  (label_num - 1);
        scale_act += min;
        unum2str(scale_act, scale_txt);

        area_t label_cord;
        point_t label_size;
        txt_get_size(&label_size, scale_txt, style->text.font,
                style->text.letter_space, style->text.line_space, GUI_COORD_MAX, GUI_TXT_FLAG_NONE);

        /*Draw the label*/
        label_cord.x1 = x - label_size.x / 2;
        label_cord.y1 = y - label_size.y / 2;
        label_cord.x2 = label_cord.x1 + label_size.x;
        label_cord.y2 = label_cord.y1 + label_size.y;

        draw_label(&label_cord, mask, style, scale_txt, GUI_TXT_FLAG_NONE, NULL);
    }
}
/**
 * Draw the needles of a gauge
 * @param gauge pointer to gauge object
 * @param mask mask of drawing
 */
static void gauge_draw_needle(obj_t * gauge, const area_t * mask)
{
    style_t style_needle;
    gauge_ext_t * ext = obj_get_ext_attr(gauge);
    style_t * style = gauge_get_style(gauge);

    coord_t r = obj_get_width(gauge) / 2 - style->body.padding.hor;
    coord_t x_ofs = obj_get_width(gauge) / 2 + gauge->coords.x1;
    coord_t y_ofs = obj_get_height(gauge) / 2 + gauge->coords.y1;
    uint16_t angle = lmeter_get_scale_angle(gauge);
		int16_t angle_ofs = 0;
		if(ext->rotate_angle)
			angle_ofs = ext->rotate_angle;
		else if(ext->rotate_angle==0)
			angle_ofs = 90 + (360 - angle) / 2;
    int16_t min = gauge_get_min_value(gauge);
    int16_t max = gauge_get_max_value(gauge);
    point_t p_mid;
    point_t p_end;
    uint8_t i;

    style_copy(&style_needle, style);

    p_mid.x = x_ofs;
    p_mid.y = y_ofs;
    for(i = 0; i < ext->needle_count; i++) {
        /*Calculate the end point of a needle*/
        int16_t needle_angle = (ext->values[i] - min) * angle / (max - min) + angle_ofs;
				if(ext->needle_lengths == NULL)  
				{
					p_end.x = (trigo_sin(needle_angle + 90) * r) / GUI_TRIGO_SIN_MAX + x_ofs;
					p_end.y = (trigo_sin(needle_angle) * r) / GUI_TRIGO_SIN_MAX + y_ofs;
				}
				else 
				{
					p_end.x = (trigo_sin(needle_angle + 90) * (r - ext->needle_lengths[i])) / GUI_TRIGO_SIN_MAX + x_ofs;
					p_end.y = (trigo_sin(needle_angle) * (r - ext->needle_lengths[i])) / GUI_TRIGO_SIN_MAX + y_ofs;
				}
        /*Draw the needle with the corresponding color*/
        if(ext->needle_colors == NULL) style_needle.line.color = GUI_GAUGE_DEF_NEEDLE_COLOR;
        else style_needle.line.color = ext->needle_colors[i];

        draw_line(&p_mid, &p_end, mask, &style_needle);
    }

    /*Draw the needle middle area*/
    style_t style_neddle_mid;
    style_copy(&style_neddle_mid, &style_plain);
    style_neddle_mid.body.main_color = style->body.border.color;
    style_neddle_mid.body.grad_color = style->body.border.color;
    style_neddle_mid.body.radius = GUI_RADIUS_CIRCLE;

    area_t nm_cord;
    nm_cord.x1 = x_ofs - style->body.padding.ver;
    nm_cord.y1 = y_ofs - style->body.padding.ver;
    nm_cord.x2 = x_ofs + style->body.padding.ver;
    nm_cord.y2 = y_ofs + style->body.padding.ver;

    draw_rect(&nm_cord, mask, &style_neddle_mid);
}

#endif
