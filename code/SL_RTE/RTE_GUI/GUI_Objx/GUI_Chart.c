/**
 * @file chart.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_CHART != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_CHART_YMIN_DEF	0
#define GUI_CHART_YMAX_DEF	100
#define GUI_CHART_HDIV_DEF	3
#define GUI_CHART_VDIV_DEF	5
#define GUI_CHART_PNUM_DEF	10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool chart_design(obj_t * chart, const area_t * mask, design_mode_t mode);
static res_t chart_signal(obj_t * chart, signal_t sign, void * param);
static void chart_draw_div(obj_t * chart, const area_t * mask);
static void chart_draw_lines(obj_t * chart, const area_t * mask);
static void chart_draw_points(obj_t * chart, const area_t * mask);
static void chart_draw_cols(obj_t * chart, const area_t * mask);

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
 * Create a chart background objects
 * @param par pointer to an object, it will be the parent of the new chart background
 * @param copy pointer to a chart background object, if not NULL then the new object will be copied from it
 * @return pointer to the created chart background
 */
obj_t * chart_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor basic object*/
    obj_t * new_chart = obj_create(par, copy);
    RTE_AssertParam(new_chart);

    /*Allocate the object type specific extended data*/
    chart_ext_t * ext = obj_allocate_ext_attr(new_chart, sizeof(chart_ext_t));
    RTE_AssertParam(ext);
    ll_init(&ext->series_ll, sizeof(chart_series_t));
    ext->series.num = 0;
    ext->ymin = GUI_CHART_YMIN_DEF;
    ext->ymax = GUI_CHART_YMAX_DEF;
    ext->hdiv_cnt = GUI_CHART_HDIV_DEF;
    ext->vdiv_cnt = GUI_CHART_VDIV_DEF;
    ext->point_cnt = GUI_CHART_PNUM_DEF;
    ext->type = GUI_CHART_TYPE_LINE;
    ext->series.opa = GUI_OPA_COVER;
    ext->series.dark = GUI_OPA_50;
    ext->series.width = 2;

    if(ancestor_design_f == NULL) ancestor_design_f = obj_get_design_func(new_chart);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_chart);

    obj_set_signal_func(new_chart, chart_signal);
    obj_set_design_func(new_chart, chart_design);

    /*Init the new chart background object*/
    if(copy == NULL) {
    	obj_set_size(new_chart, GUI_HOR_RES / 3, GUI_VER_RES / 3);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            chart_set_style(new_chart, th->chart);
        } else {
            chart_set_style(new_chart, &style_pretty);
        }

    } else {
    	chart_ext_t * ext_copy = obj_get_ext_attr(copy);
        ext->type = ext_copy->type;
		ext->ymin = ext_copy->ymin;
		ext->ymax = ext_copy->ymax;
		ext->hdiv_cnt = ext_copy->hdiv_cnt;
		ext->vdiv_cnt = ext_copy->vdiv_cnt;
        ext->point_cnt = ext_copy->point_cnt;
        ext->series.opa =  ext_copy->series.opa;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_chart);
    }

    return new_chart;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Allocate and add a data series to the chart
 * @param chart pointer to a chart object
 * @param color color of the data series
 * @return pointer to the allocated data series
 */
chart_series_t * chart_add_series(obj_t * chart, color_t color)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	chart_series_t *ser = ll_ins_head(&ext->series_ll);
	coord_t def = (ext->ymin + ext->ymax) >> 1;	/*half range as default value*/

	if(ser == NULL) return NULL;

    ser->color = color;

	ser->points = RTE_BGet(MEM_RTE,sizeof(coord_t) * ext->point_cnt);

	uint16_t i;
	coord_t * p_tmp = ser->points;
	for(i = 0; i < ext->point_cnt; i++) {
		*p_tmp = def;
		p_tmp++;
	}

	ext->series.num++;

	return ser;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the number of horizontal and vertical division lines
 * @param chart pointer to a graph background object
 * @param hdiv number of horizontal division lines
 * @param vdiv number of vertical division lines
 */
void chart_set_div_line_count(obj_t * chart, uint8_t hdiv, uint8_t vdiv)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	if(ext->hdiv_cnt == hdiv && ext->vdiv_cnt == vdiv) return;

	ext->hdiv_cnt = hdiv;
	ext->vdiv_cnt = vdiv;

	obj_invalidate(chart);
}

/**
 * Set the minimal and maximal y values
 * @param chart pointer to a graph background object
 * @param ymin y minimum value
 * @param ymax y maximum value
 */
void chart_set_range(obj_t * chart, coord_t ymin, coord_t ymax)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	if(ext->ymin == ymin && ext->ymax == ymax) return;

	ext->ymin = ymin;
	ext->ymax = ymax;

	chart_refresh(chart);
}

/**
 * Set a new type for a chart
 * @param chart pointer to a chart object
 * @param type new type of the chart (from 'chart_type_t' enum)
 */
void chart_set_type(obj_t * chart, chart_type_t type)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	if(ext->type == type) return;

	ext->type = type;

	chart_refresh(chart);
}

/**
 * Set the number of points on a data line on a chart
 * @param chart pointer r to chart object
 * @param point_cnt new number of points on the data lines
 */
void chart_set_point_count(obj_t * chart, uint16_t point_cnt)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	if(ext->point_cnt == point_cnt) return;

	chart_series_t *ser;
	uint16_t point_cnt_old = ext->point_cnt;
	uint16_t i;
    coord_t def = (ext->ymin + ext->ymax) >> 1;  /*half range as default value*/

	if(point_cnt < 1) point_cnt = 1;

	LL_READ_BACK(ext->series_ll, ser) {
		ser->points = RTE_BGetr(MEM_RTE,ser->points, sizeof(coord_t) * point_cnt);

		/*Initialize the new points*/
		if(point_cnt > point_cnt_old) {
		    for(i = point_cnt_old - 1; i < point_cnt; i++) {
		        ser->points[i] = def;
		    }
		}
	}

	ext->point_cnt = point_cnt;
	chart_refresh(chart);
}

/**
 * Set the opacity of the data series
 * @param chart pointer to a chart object
 * @param opa opacity of the data series
 */
void chart_set_series_opa(obj_t * chart, opa_t opa)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    if(ext->series.opa == opa) return;

    ext->series.opa = opa;
    obj_invalidate(chart);
}

/**
 * Set the line width or point radius of the data series
 * @param chart pointer to a chart object
 * @param width the new width
 */
void chart_set_series_width(obj_t * chart, coord_t width)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    if(ext->series.width == width) return;

    ext->series.width = width;
    obj_invalidate(chart);
}
/**
 * Set the dark effect on the bottom of the points or columns
 * @param chart pointer to a chart object
 * @param dark_eff dark effect level (GUI_OPA_TRANSP to turn off)
 */
void chart_set_series_darking(obj_t * chart, opa_t dark_eff)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    if(ext->series.dark == dark_eff) return;

    ext->series.dark = dark_eff;
    obj_invalidate(chart);
}

/**
 * Initialize all data points with a value
 * @param chart pointer to chart object
 * @param ser pointer to a data series on 'chart'
 * @param y the new value  for all points
 */
void chart_init_points(obj_t * chart, chart_series_t * ser, coord_t y)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    uint16_t i;
    for(i = 0; i < ext->point_cnt; i++) {
        ser->points[i] = y;
    }
    chart_refresh(chart);
}

/**
 * Set the value s of points from an array
 * @param chart pointer to chart object
 * @param ser pointer to a data series on 'chart'
 * @param y_array array of 'coord_t' points (with 'points count' elements )
 */
void chart_set_points(obj_t * chart, chart_series_t * ser, coord_t * y_array)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    memcpy(ser->points, y_array, ext->point_cnt * (sizeof(coord_t)));
    chart_refresh(chart);
}

/**
 * Shift all data right and set the most right data on a data line
 * @param chart pointer to chart object
 * @param ser pointer to a data series on 'chart'
 * @param y the new value of the most right data
 */
void chart_set_next(obj_t * chart, chart_series_t * ser, coord_t y)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);

	uint16_t i;
	for(i = 0; i < ext->point_cnt - 1; i++) {
		ser->points[i] = ser->points[i + 1];
	}

	ser->points[ext->point_cnt - 1] = y;
	chart_refresh(chart);

}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the type of a chart
 * @param chart pointer to chart object
 * @return type of the chart (from 'chart_t' enum)
 */
chart_type_t chart_get_type(obj_t * chart)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    return (chart_type_t)ext->type;
}

/**
 * Get the data point number per data line on chart
 * @param chart pointer to chart object
 * @return point number on each data line
 */
uint16_t chart_get_point_cnt(obj_t * chart)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    return ext->point_cnt;
}

/**
 * Get the opacity of the data series
 * @param chart pointer to chart object
 * @return the opacity of the data series
 */
opa_t chart_get_series_opa(obj_t * chart)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    return ext->series.opa;
}

/**
 * Get the data series width
 * @param chart pointer to chart object
 * @return the width the data series (lines or points)
 */
coord_t chart_get_series_width(obj_t * chart)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    return ext->series.width;
}

/**
 * Get the dark effect level on the bottom of the points or columns
 * @param chart pointer to chart object
 * @return dark effect level (GUI_OPA_TRANSP to turn off)
 */
opa_t chart_get_series_darking(obj_t * chart)
{
    chart_ext_t * ext = obj_get_ext_attr(chart);
    return ext->series.dark;
}

/*=====================
 * Other functions
 *====================*/

/**
 * Refresh a chart if its data line has changed
 * @param chart pointer to chart object
 */
void chart_refresh(obj_t * chart)
{
    obj_invalidate(chart);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the chart backgrounds
 * @param chart pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool chart_design(obj_t * chart, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
    	/*Return false if the object is not covers the mask_p area*/
    	return ancestor_design_f(chart, mask, mode);
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
		/*Draw the background*/
        draw_rect(&chart->coords, mask, obj_get_style(chart));

		chart_ext_t * ext = obj_get_ext_attr(chart);

		chart_draw_div(chart, mask);

		if(ext->type & GUI_CHART_TYPE_LINE) chart_draw_lines(chart, mask);
		if(ext->type & GUI_CHART_TYPE_COLUMN) chart_draw_cols(chart, mask);
		if(ext->type & GUI_CHART_TYPE_POINT) chart_draw_points(chart, mask);
    }
    return true;
}

/**
 * Signal function of the chart background
 * @param chart pointer to a chart background object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 */
static res_t chart_signal(obj_t * chart, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(chart, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_CLEANUP) {
        coord_t ** datal;
        chart_ext_t * ext = obj_get_ext_attr(chart);
        LL_READ(ext->series_ll, datal) {
            RTE_BRel(MEM_RTE,*datal);
        }
        ll_clear(&ext->series_ll);
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "chart";
    }

    return res;
}

/**
 * Draw the division lines on chart background
 * @param chart pointer to chart object
 * @param mask mask, inherited from the design function
 */
static void chart_draw_div(obj_t * chart, const area_t * mask)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);
	style_t * style = obj_get_style(chart);

	uint8_t div_i;
    uint8_t div_i_end;
    uint8_t div_i_start;
	point_t p1;
	point_t p2;
	coord_t w = obj_get_width(chart);
	coord_t h = obj_get_height(chart);
	coord_t x_ofs = chart->coords.x1;
	coord_t y_ofs = chart->coords.y1;

	if(ext->hdiv_cnt != 0) {
        /*Draw slide lines if no border*/
        if(style->body.border.width != 0) {
            div_i_start = 1;
            div_i_end = ext->hdiv_cnt;
        } else {
            div_i_start = 0;
            div_i_end = ext->hdiv_cnt + 1;
        }

        p1.x = 0 + x_ofs;
        p2.x = w + x_ofs;
        for(div_i = div_i_start; div_i <= div_i_end; div_i++) {
            p1.y = (int32_t)((int32_t)h * div_i) / (ext->hdiv_cnt + 1);
            p1.y +=  y_ofs;
            if(div_i == div_i_start) p1.y += (style->line.width >> 1) + 1;  /*The first line might not be visible*/
            if(div_i == div_i_end) p1.y -= (style->line.width >> 1) + 1;  /*The last line might not be visible*/

            p2.y = p1.y;
            draw_line(&p1, &p2, mask, style);
        }
	}

	if(ext->vdiv_cnt != 0) {
        /*Draw slide lines if no border*/
        if(style->body.border.width != 0) {
            div_i_start = 1;
            div_i_end = ext->vdiv_cnt;
        } else {
            div_i_start = 0;
            div_i_end = ext->vdiv_cnt + 1;
        }

        p1.y = 0 + y_ofs;
        p2.y = h + y_ofs;
        for(div_i = div_i_start; div_i <= div_i_end; div_i ++) {
            p1.x = (int32_t)((int32_t)w * div_i) / (ext->vdiv_cnt + 1);
            p1.x +=  x_ofs;
            if(div_i == div_i_start) p1.x += (style->line.width >> 1) + 1;  /*The first line might not be visible*/
            if(div_i == div_i_end) p1.x -= (style->line.width >> 1) + 1;  /*The last line might not be visible*/
            p2.x = p1.x;
            draw_line(&p1, &p2, mask, style);
        }
	}
}

/**
 * Draw the data lines as lines on a chart
 * @param obj pointer to chart object
 */
static void chart_draw_lines(obj_t * chart, const area_t * mask)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);

	uint16_t i;
	point_t p1;
	point_t p2;
	coord_t w = obj_get_width(chart);
	coord_t h = obj_get_height(chart);
    coord_t x_ofs = chart->coords.x1;
    coord_t y_ofs = chart->coords.y1;
	int32_t y_tmp;
	chart_series_t *ser;
	style_t lines;
	style_copy(&lines, &style_plain);
	lines.line.opa = ext->series.opa;
    lines.line.width = ext->series.width;

	/*Go through all data lines*/
	LL_READ_BACK(ext->series_ll, ser) {
		lines.line.color = ser->color;

		p1.x = 0 + x_ofs;
		p2.x = 0 + x_ofs;
		y_tmp = (int32_t)((int32_t) ser->points[0] - ext->ymin) * h;
		y_tmp = y_tmp / (ext->ymax - ext->ymin);
		p2.y = h - y_tmp + y_ofs;

		for(i = 1; i < ext->point_cnt; i ++) {
			p1.x = p2.x;
			p1.y = p2.y;

			p2.x = ((w * i) / (ext->point_cnt - 1)) + x_ofs;

			y_tmp = (int32_t)((int32_t) ser->points[i] - ext->ymin) * h;
			y_tmp = y_tmp / (ext->ymax - ext->ymin);
			p2.y = h - y_tmp + y_ofs;

			draw_line(&p1, &p2, mask, &lines);
		}
	}
}

/**
 * Draw the data lines as points on a chart
 * @param chart pointer to chart object
 * @param mask mask, inherited from the design function
 */
static void chart_draw_points(obj_t * chart, const area_t * mask)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);

	uint16_t i;
	area_t cir_a;
	coord_t w = obj_get_width(chart);
	coord_t h = obj_get_height(chart);
    coord_t x_ofs = chart->coords.x1;
    coord_t y_ofs = chart->coords.y1;
	int32_t y_tmp;
    chart_series_t * ser;
    uint8_t series_cnt = 0;
    style_t style_point;
    style_copy(&style_point, &style_plain);

	style_point.body.border.width = 0;
	style_point.body.empty = 0;
	style_point.body.radius = GUI_RADIUS_CIRCLE;
    style_point.body.opa = ext->series.opa;
    style_point.body.radius = ext->series.width;

	/*Go through all data lines*/
	LL_READ_BACK(ext->series_ll, ser) {
		style_point.body.main_color = ser->color;
		style_point.body.grad_color = color_mix(GUI_COLOR_BLACK, ser->color, ext->series.dark);

		for(i = 0; i < ext->point_cnt; i ++) {
			cir_a.x1 = ((w * i) / (ext->point_cnt - 1)) + x_ofs;
			cir_a.x2 = cir_a.x1 + style_point.body.radius;
			cir_a.x1 -= style_point.body.radius;

			y_tmp = (int32_t)((int32_t) ser->points[i] - ext->ymin) * h;
			y_tmp = y_tmp / (ext->ymax - ext->ymin);
			cir_a.y1 = h - y_tmp + y_ofs;
			cir_a.y2 = cir_a.y1 + style_point.body.radius;
			cir_a.y1 -= style_point.body.radius;

			draw_rect(&cir_a, mask, &style_point);
		}
		series_cnt++;
	}
}

/**
 * Draw the data lines as columns on a chart
 * @param chart pointer to chart object
 * @param mask mask, inherited from the design function
 */
static void chart_draw_cols(obj_t * chart, const area_t * mask)
{
	chart_ext_t * ext = obj_get_ext_attr(chart);

	uint16_t i;
	area_t col_a;
	area_t col_mask;
	bool mask_ret;
	coord_t w = obj_get_width(chart);
	coord_t h = obj_get_height(chart);
	int32_t y_tmp;
    chart_series_t *ser;
    style_t rects;
	coord_t col_w = w / ((ext->series.num + 1) * ext->point_cnt); /* Suppose + 1 series as separator*/
	coord_t x_ofs = col_w / 2; /*Shift with a half col.*/

	style_copy(&rects, &style_plain);
	rects.body.border.width = 0;
	rects.body.empty = 0;
	rects.body.radius = 0;
	rects.body.opa = ext->series.opa;

	col_a.y2 = chart->coords.y2;

	coord_t x_act;

	/*Go through all points*/
    for(i = 0; i < ext->point_cnt; i ++) {
        x_act = (int32_t)((int32_t) w * i) / ext->point_cnt;
        x_act += chart->coords.x1 + x_ofs;

        /*Draw the current point of all data line*/
        LL_READ_BACK(ext->series_ll, ser) {
            rects.body.main_color = ser->color;
            rects.body.grad_color = color_mix(GUI_COLOR_BLACK, ser->color, ext->series.dark);
            col_a.x1 = x_act;
            col_a.x2 = col_a.x1 + col_w;
            x_act += col_w;

            y_tmp = (int32_t)((int32_t) ser->points[i] - ext->ymin) * h;
            y_tmp = y_tmp / (ext->ymax - ext->ymin);
            col_a.y1 = h - y_tmp + chart->coords.y1;

            mask_ret = area_union(&col_mask, mask, &col_a);
            if(mask_ret != false) {
                draw_rect(&chart->coords, &col_mask, &rects);
            }
        }
	}
}
#endif
