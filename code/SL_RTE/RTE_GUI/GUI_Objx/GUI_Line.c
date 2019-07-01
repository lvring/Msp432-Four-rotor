/**
 * @file line.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

#if GUI_USE_LINE != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool line_design(obj_t * line, const area_t * mask, design_mode_t mode);
static res_t line_signal(obj_t * line, signal_t sign, void * param);

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
 * Create a line objects
 * @param par pointer to an object, it will be the parent of the new line
 * @return pointer to the created line
 */
obj_t * line_create(obj_t * par, obj_t * copy)
{
    /*Create a basic object*/
    obj_t * new_line = obj_create(par, copy);
    RTE_AssertParam(new_line);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_line);

    /*Extend the basic object to line object*/
    line_ext_t * ext = obj_allocate_ext_attr(new_line, sizeof(line_ext_t));
    RTE_AssertParam(ext);
    ext->point_num = 0;
    ext->point_array = NULL;
    ext->auto_size = 1;
    ext->y_inv = 0;

    obj_set_design_func(new_line, line_design);
    obj_set_signal_func(new_line, line_signal);

    /*Init the new line*/
    if(copy == NULL) {
        obj_set_size(new_line, GUI_DPI, GUI_DPI);  /*Auto size is enables, but set default size until no points are added*/
	    obj_set_style(new_line, NULL);           /*Inherit parent's style*/
	    obj_set_click(new_line, false);
    }
    /*Copy an existing object*/
    else {
        line_ext_t * copy_ext = obj_get_ext_attr(copy);
    	line_set_auto_size(new_line,line_get_auto_size(copy));
    	line_set_y_invert(new_line,line_get_y_inv(copy));
    	line_set_auto_size(new_line,line_get_auto_size(copy));
    	line_set_points(new_line, copy_ext->point_array, copy_ext->point_num);
        /*Refresh the style with new signal function*/
        obj_refresh_style(new_line);
    }

    return new_line;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set an array of points. The line object will connect these points.
 * @param line pointer to a line object
 * @param point_a an array of points. Only the address is saved,
 * so the array can NOT be a local variable which will be destroyed
 * @param point_num number of points in 'point_a'
 */
void line_set_points(obj_t * line, const point_t * point_a, uint16_t point_num)
{
	line_ext_t * ext = obj_get_ext_attr(line);
	ext->point_array = point_a;
	ext->point_num = point_num;

	if(point_num > 0 && ext->auto_size != 0) {
		uint16_t i;
		coord_t xmax = GUI_COORD_MIN;
		coord_t ymax = GUI_COORD_MIN;
		for(i = 0; i < point_num; i++) {
			xmax = RTE_MATH_MAX(point_a[i].x, xmax);
			ymax = RTE_MATH_MAX(point_a[i].y, ymax);
		}

		style_t * style = line_get_style(line);
		obj_set_size(line, xmax + style->line.width, ymax + style->line.width);
	}

	obj_invalidate(line);
}

/**
 * Enable (or disable) the auto-size option. The size of the object will fit to its points.
 * (set width to x max and height to y max)
 * @param line pointer to a line object
 * @param autosize_en true: auto size is enabled, false: auto size is disabled
 */
void line_set_auto_size(obj_t * line, bool autosize_en)
{
	line_ext_t * ext = obj_get_ext_attr(line);
	if(ext->auto_size == autosize_en) return;

	ext->auto_size = autosize_en == false ? 0 : 1;

	/*Refresh the object*/
	if(autosize_en) line_set_points(line, ext->point_array, ext->point_num);
}

/**
 * Enable (or disable) the y coordinate inversion.
 * If enabled then y will be subtracted from the height of the object,
 * therefore the y=0 coordinate will be on the bottom.
 * @param line pointer to a line object
 * @param yinv_en true: enable the y inversion, false:disable the y inversion
 */
void line_set_y_invert(obj_t * line, bool yinv_en)
{
	line_ext_t * ext = obj_get_ext_attr(line);
	if(ext->y_inv == yinv_en) return;

	ext->y_inv = yinv_en == false ? 0 : 1;

	obj_invalidate(line);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the auto size attribute
 * @param line pointer to a line object
 * @return true: auto size is enabled, false: disabled
 */
bool line_get_auto_size(obj_t * line)
{
	line_ext_t * ext = obj_get_ext_attr(line);

	return ext->auto_size == 0 ? false : true;
}

/**
 * Get the y inversion attribute
 * @param line pointer to a line object
 * @return true: y inversion is enabled, false: disabled
 */
bool line_get_y_inv(obj_t * line)
{
	line_ext_t * ext = obj_get_ext_attr(line);

	return ext->y_inv == 0 ? false : true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the lines
 * @param line pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool line_design(obj_t * line, const area_t * mask, design_mode_t mode)
{
    /*A line never covers an area*/
    if(mode == GUI_DESIGN_COVER_CHK) return false;
    else if(mode == GUI_DESIGN_DRAW_MAIN) {
		line_ext_t * ext = obj_get_ext_attr(line);

		if(ext->point_num == 0 || ext->point_array == NULL) return false;

		style_t * style = obj_get_style(line);
		area_t area;
		obj_get_coords(line, &area);
		coord_t x_ofs = area.x1;
		coord_t y_ofs = area.y1;
		point_t p1;
		point_t p2;
		coord_t h = obj_get_height(line);
		uint16_t i;

		/*Read all pints and draw the lines*/
		for (i = 0; i < ext->point_num - 1; i++) {

			p1.x = ext->point_array[i].x + x_ofs;
			p2.x = ext->point_array[i + 1].x + x_ofs;

			if(ext->y_inv == 0) {
				p1.y = ext->point_array[i].y + y_ofs;
				p2.y = ext->point_array[i + 1].y + y_ofs;
			} else {
				p1.y = h - ext->point_array[i].y  + y_ofs;
				p2.y = h - ext->point_array[i + 1].y + y_ofs;
			}
			draw_line(&p1, &p2, mask, style);
		}
    }
    return true;
}

/**
 * Signal function of the line
 * @param line pointer to a line object
 * @param sign a signal type from signal_t enum
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t line_signal(obj_t * line, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(line, sign, param);
    if(res != GUI_RES_OK) return res;


    if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "line";
    }
    else if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        style_t * style = line_get_style(line);
        if(line->ext_size < style->line.width) line->ext_size = style->line.width;
    }


    return res;
}
#endif
