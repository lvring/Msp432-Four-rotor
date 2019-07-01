#include "RTE_Include.h"
#if RTE_USE_GUI == 1
/*********************
 *      DEFINES
 *********************/
#define CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD  5   /*Circle segment greater then this value will be anti-aliased by a non-linear (cos) opacity mapping*/

#define LINE_WIDTH_CORR_BASE 64
#define LINE_WIDTH_CORR_SHIFT 6

#define LABEL_RECOLOR_PAR_LENGTH    6

#define SHADOW_OPA_EXTRA_PRECISION      0       /*Calculate with 2^x bigger shadow opacity values to avoid rounding errors*/
#define SHADOW_BOTTOM_AA_EXTRA_RADIUS   3       /*Add extra radius with GUI_SHADOW_BOTTOM to cover anti-aliased corners*/
/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    CMD_STATE_WAIT,
    CMD_STATE_PAR,
    CMD_STATE_IN,
}cmd_state_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void draw_rect_main_mid(const area_t * coords, const area_t * mask, const style_t * style);
static void draw_rect_main_corner(const area_t * coords, const area_t * mask, const style_t * style);
static void draw_rect_border_straight(const area_t * coords, const area_t * mask, const style_t * style);
static void draw_rect_border_corner(const area_t * coords, const area_t * mask, const style_t * style);
#if GUI_USE_SHADOW && GUI_VDB_SIZE
static void draw_shadow(const area_t * coords, const area_t * mask, const style_t * style);
static void draw_shadow_full(const area_t * coords, const area_t * mask, const  style_t * style);
static void draw_shadow_bottom(const area_t * coords, const area_t * mask, const style_t * style);
static void draw_shadow_full_straight(const area_t * coords, const area_t * mask, const style_t * style, const opa_t * map);
#endif
static uint16_t draw_cont_radius_corr(uint16_t r, coord_t w, coord_t h);
#if GUI_ANTIALIAS != 0
static opa_t antialias_get_opa(coord_t seg, coord_t px_id, opa_t line_opa);
static opa_t antialias_get_opa_circ(coord_t seg, coord_t px_id, opa_t opa);
#endif


static uint8_t hex_char_to_num(char hex);

#if GUI_USE_TRIANGLE != 0
static void point_swap(point_t * p1, point_t * p2);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static void (*px_fp)(coord_t x, coord_t y, const area_t * mask, color_t color, opa_t opa) = vpx;
static void (*fill_fp)(const area_t * coords, const area_t * mask, color_t color, opa_t opa) =  vfill;
static void (*letter_fp)(const point_t * pos_p, const area_t * mask, const font_t * font_p, uint32_t letter, color_t color, opa_t opa) = vletter;
#if GUI_USE_IMG
static void (*map_fp)(const area_t * cords_p, const area_t * mask_p,
        const uint8_t * map_p, opa_t opa, bool chroma_key, bool alpha_byte,
        color_t recolor, opa_t recolor_opa) = vmap;
#endif /*GUI_USE_IMG*/
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Draw a rectangle 
 * @param coords the coordinates of the rectangle
 * @param mask the rectangle will be drawn only in this mask
 * @param style pointer to a style
 */
void draw_rect(const area_t * coords, const area_t * mask, const style_t * style)
{
    if(area_get_height(coords) < 1 || area_get_width(coords) < 1) return;

#if GUI_USE_SHADOW && GUI_VDB_SIZE
    if(style->body.shadow.width != 0) {
        draw_shadow(coords, mask, style);
    }
#endif
    if(style->body.empty == 0){
        draw_rect_main_mid(coords, mask, style);

        if(style->body.radius != 0) {
            draw_rect_main_corner(coords, mask, style);
        }
    } 
    
    if(style->body.border.width != 0 && style->body.border.part != GUI_BORDER_NONE) {
        draw_rect_border_straight(coords, mask, style);

        if(style->body.radius != 0) {
            draw_rect_border_corner(coords, mask, style);
        }
    }
}

#if GUI_USE_TRIANGLE != 0
/**
 *
 * @param points pointer to an array with 3 points
 * @param mask the triangle will be drawn only in this mask
 * @param color color of the triangle
 */
void draw_triangle(const point_t * points, const area_t * mask, color_t color)
{
    point_t tri[3];

    memcpy(tri, points, sizeof(tri));

    /*Sort the vertices according to their y coordinate (0: y max, 1: y mid, 2:y min)*/
    if(tri[1].y < tri[0].y) point_swap(&tri[1], &tri[0]);
    if(tri[2].y < tri[1].y) point_swap(&tri[2], &tri[1]);
    if(tri[1].y < tri[0].y) point_swap(&tri[1], &tri[0]);

    /*Return is the triangle is degenerated*/
    if(tri[0].x == tri[1].x && tri[0].y == tri[1].y) return;
    if(tri[1].x == tri[2].x && tri[1].y == tri[2].y) return;
    if(tri[0].x == tri[2].x && tri[0].y == tri[2].y) return;

    if(tri[0].x == tri[1].x && tri[1].x == tri[2].x) return;
    if(tri[0].y == tri[1].y && tri[1].y == tri[2].y) return;

    /*Draw the triangle*/
    point_t edge1;
    coord_t dx1 = RTE_MATH_ABS(tri[0].x - tri[1].x);
    coord_t sx1 = tri[0].x < tri[1].x ? 1 : -1;
    coord_t dy1 = RTE_MATH_ABS(tri[0].y - tri[1].y);
    coord_t sy1 = tri[0].y < tri[1].y ? 1 : -1;
    coord_t err1 = (dx1 > dy1 ? dx1 : -dy1) / 2;
    coord_t err_tmp1;

    point_t edge2;
    coord_t dx2 = RTE_MATH_ABS(tri[0].x - tri[2].x);
    coord_t sx2 = tri[0].x < tri[2].x ? 1 : -1;
    coord_t dy2 = RTE_MATH_ABS(tri[0].y - tri[2].y);
    coord_t sy2 = tri[0].y < tri[2].y ? 1 : -1;
    coord_t err2 = (dx1 > dy2 ? dx2 : -dy2) / 2;
    coord_t err_tmp2;

    coord_t y1_tmp;
    coord_t y2_tmp;

    edge1.x = tri[0].x;
    edge1.y = tri[0].y;
    edge2.x = tri[0].x;
    edge2.y = tri[0].y;
    area_t act_area;
    area_t draw_area;

    while(1) {
        act_area.x1 = edge1.x;
        act_area.x2 = edge2.x ;
        act_area.y1 = edge1.y;
        act_area.y2 = edge2.y ;


        draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
        draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
        draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
        draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
        draw_area.x2--; /*Do not draw most right pixel because it will be drawn by the adjacent triangle*/
        fill_fp(&draw_area, mask, color, GUI_OPA_50);

        /*Calc. the next point of edge1*/
        y1_tmp = edge1.y;
        do {
            if (edge1.x == tri[1].x && edge1.y == tri[1].y) {

                dx1 = RTE_MATH_ABS(tri[1].x - tri[2].x);
                sx1 = tri[1].x < tri[2].x ? 1 : -1;
                dy1 = RTE_MATH_ABS(tri[1].y - tri[2].y);
                sy1 = tri[1].y < tri[2].y ? 1 : -1;
                err1 = (dx1 > dy1 ? dx1 : -dy1) / 2;
            }
            else if (edge1.x == tri[2].x && edge1.y == tri[2].y) return;
            err_tmp1 = err1;
            if (err_tmp1 >-dx1) {
                err1 -= dy1;
                edge1.x += sx1;
            }
            if (err_tmp1 < dy1) {
                err1 += dx1;
                edge1.y += sy1;
            }
        } while(edge1.y == y1_tmp);

        /*Calc. the next point of edge2*/
        y2_tmp = edge2.y;
        do {
            if (edge2.x == tri[2].x && edge2.y == tri[2].y) return;
            err_tmp2 = err2;
            if (err_tmp2 > -dx2) {
                err2 -= dy2;
                edge2.x += sx2;
            }
            if (err_tmp2 < dy2) {
                err2 += dx2;
                edge2.y += sy2;
            }
        } while(edge2.y == y2_tmp);
    }
}
#endif

/**
 * Write a text
 * @param coords coordinates of the label
 * @param mask the label will be drawn only in this area
 * @param style pointer to a style
 * @param txt 0 terminated text to write
 * @param flag settings for the text from 'txt_flag_t' enum
 * @param offset text offset in x and y direction (NULL if unused)
 *
 */
void draw_label(const area_t * coords,const area_t * mask, const style_t * style,
                    const char * txt, txt_flag_t flag, point_t * offset)
{

    const font_t * font = style->text.font;
    coord_t w;
    if((flag & GUI_TXT_FLAG_EXPAND) == 0) {
        w = area_get_width(coords);
    } else {
        point_t p;
        txt_get_size(&p, txt, style->text.font, style->text.letter_space, style->text.line_space, GUI_COORD_MAX, flag);
        w = p.x;
    }

    /*Init variables for the first line*/
    coord_t line_length = 0;
    uint32_t line_start = 0;
    uint32_t line_end = txt_get_next_line(txt, font, style->text.letter_space, w, flag);

    point_t pos;
    pos.x = coords->x1;
    pos.y = coords->y1;

    /*Align the line to middle if enabled*/
    if(flag & GUI_TXT_FLAG_CENTER) {
        line_length = txt_get_width(&txt[line_start], line_end - line_start,
                                    font, style->text.letter_space, flag);
        pos.x += (w - line_length) / 2;
    }

    cmd_state_t cmd_state = CMD_STATE_WAIT;
    uint32_t i;
    uint16_t par_start = 0;
    color_t recolor;
    coord_t letter_w;

    coord_t x_ofs = 0;
    coord_t y_ofs = 0;
    if(offset != NULL) {
        x_ofs = offset->x;
        y_ofs = offset->y;
        pos.y += y_ofs;
    }

    /*Real draw need a background color for higher bpp letter*/
#if GUI_VDB_SIZE == 0
    rletter_set_background(style->body.main_color);
#endif

    /*Write out all lines*/
    while(txt[line_start] != '\0') {
        if(offset != NULL) {
            pos.x += x_ofs;
        }
        /*Write all letter of a line*/
        cmd_state = CMD_STATE_WAIT;
        i = line_start;
        uint32_t letter;
        while(i < line_end) {
            letter = txt_utf8_next(txt, &i);
            /*Handle the re-color command*/
            if((flag & GUI_TXT_FLAG_RECOLOR) != 0) {
                if(letter == (uint32_t)GUI_TXT_COLOR_CMD[0]) {
                    if(cmd_state == CMD_STATE_WAIT) { /*Start char*/
                        par_start = i;// + txt_utf8_size(txt[i]);
                        cmd_state = CMD_STATE_PAR;
                        continue;
                    } else if(cmd_state == CMD_STATE_PAR) { /*Other start char in parameter escaped cmd. char */
                        cmd_state = CMD_STATE_WAIT;
                    }else if(cmd_state == CMD_STATE_IN) { /*Command end */
                        cmd_state = CMD_STATE_WAIT;
                        continue;
                    }
                }

                /*Skip the color parameter and wait the space after it*/
                if(cmd_state == CMD_STATE_PAR) {
                    if(letter == ' ') {
                        /*Get the parameter*/
                        if(i - par_start == LABEL_RECOLOR_PAR_LENGTH + 1) {
                            char buf[LABEL_RECOLOR_PAR_LENGTH + 1];
                            memcpy(buf, &txt[par_start], LABEL_RECOLOR_PAR_LENGTH);
                            buf[LABEL_RECOLOR_PAR_LENGTH] = '\0';
                            int r,g,b;
                            r = (hex_char_to_num(buf[0]) << 4) + hex_char_to_num(buf[1]);
                            g = (hex_char_to_num(buf[2]) << 4) + hex_char_to_num(buf[3]);
                            b = (hex_char_to_num(buf[4]) << 4) + hex_char_to_num(buf[5]);
                            recolor = GUI_COLOR_MAKE(r, g, b);
                        } else {
                            recolor.full = style->text.color.full;
                        }
                        cmd_state = CMD_STATE_IN; /*After the parameter the text is in the command*/
                    }
                    continue;
                }
            }

            color_t color = style->text.color;

            if(cmd_state == CMD_STATE_IN) color = recolor;

            letter_fp(&pos, mask, font, letter, color, style->text.opa);
            letter_w = font_get_width(font, letter);

            pos.x += letter_w + style->text.letter_space;
        }
        /*Go to next line*/
        line_start = line_end;
        line_end += txt_get_next_line(&txt[line_start], font, style->text.letter_space, w, flag);

        pos.x = coords->x1;
        /*Align to middle*/
        if(flag & GUI_TXT_FLAG_CENTER) {
            line_length = txt_get_width(&txt[line_start], line_end - line_start,
                                     font, style->text.letter_space, flag);
            pos.x += (w - line_length) / 2;
        }
        /*Go the next line position*/
        pos.y += font_get_height(font);
        pos.y += style->text.line_space;
    }
}

#if GUI_USE_IMG
/**
 * Draw an image
 * @param coords the coordinates of the image
 * @param mask the image will be drawn only in this area
 * @param map_p pointer to a color_t array which contains the pixels of the image
 * @param opa opacity of the image (0..255)
 */
void draw_img(const area_t * coords, const area_t * mask,
             const style_t * style, const void * src)
{
    if(src == NULL) {
        draw_rect(coords, mask, &style_plain);
        draw_label(coords, mask, &style_plain, "No\ndata", GUI_TXT_FLAG_NONE, NULL);
        return;
    }
		const img_t * img_var = src;
		area_t mask_com;    /*Common area of mask and coords*/
		bool union_ok;
		union_ok = area_union(&mask_com, mask, coords);
		if(union_ok == false) {
				return;         /*Out of mask*/
		}

		map_fp(coords, mask, img_var->pixel_map, style->image.opa, img_var->header.chroma_keyed, img_var->header.alpha_byte, style->image.color, style->image.intense);
}
#endif


/**
 * Draw a line
 * @param p1 first point of the line
 * @param p2 second point of the line
 * @param maskthe line will be drawn only on this area
 * @param lines_p pointer to a line style
 */
void draw_line(const point_t * point1, const point_t * point2, const area_t * mask,
                  const style_t * style)
{

    if(style->line.width == 0) return;
    if(point1->x == point2->x && point1->y == point2->y) return;

    point_t p1;
    point_t p2;

    /*Be sure always x1 < x2*/
    if(point1->x < point2->x) {
        p1.x = point1->x;
        p1.y = point1->y;
        p2.x = point2->x;
        p2.y = point2->y;
    } else {
        p1.x = point2->x;
        p1.y = point2->y;
        p2.x = point1->x;
        p2.y = point1->y;
    }

    coord_t dx = RTE_MATH_ABS(p2.x - p1.x);
    coord_t sx = p1.x < p2.x ? 1 : -1;
    coord_t dy = RTE_MATH_ABS(p2.y - p1.y);
    coord_t sy = p1.y < p2.y ? 1 : -1;
    coord_t err = (dx > dy ? dx : -dy) / 2;
    coord_t e2;
    bool hor = dx > dy ? true : false;	/*Rather horizontal or vertical*/
    coord_t last_x = p1.x;
    coord_t last_y = p1.y;
    point_t act_point;
    act_point.x = p1.x;
    act_point.y = p1.y;


    coord_t width;
    uint16_t wcor;
    uint16_t width_half = 0;
    uint16_t width_1 = 0;
    static const uint8_t width_corr_array[] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 66, 66, 66, 66, 66,
            67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 71, 71, 72, 72, 72, 73, 73, 74,
            74, 75, 75, 76, 77, 77, 78, 78, 79, 79, 80, 81, 81, 82, 82, 83, 84, 84, 85,
            86, 86, 87, 88, 88, 89, 90, 91,
    };

    if(hor == false) {
        wcor = (dx * LINE_WIDTH_CORR_BASE) / dy;
    } else  {
        wcor = (dy * LINE_WIDTH_CORR_BASE) / dx;
    }


    width = style->line.width - 1;

#if GUI_ANTIALIAS != 0
    bool aa_invert = false;
    aa_invert = p1.y < p2.y ? false : true;      /*Direction of opacity increase on the edges*/
    if(p1.x != p2.x && p1.y != p2.y) width--;    /*Because of anti aliasing (no anti aliasing on hor. and ver. lines)*/
#endif

    /*Make the correction on lie width*/
    if(width > 0) {
        width = (width * width_corr_array[wcor]);
        width = width >> LINE_WIDTH_CORR_SHIFT;
        width_half = width >> 1;
        width_1 = width & 0x1 ? 1 : 0;
    }

    /*Special case draw a horizontal line*/
    if(p1.y == p2.y ) {
        area_t act_area;
        act_area.x1 = p1.x;
        act_area.x2 = p2.x;
        act_area.y1 = p1.y - width_half - width_1;
        act_area.y2 = p2.y + width_half ;

        area_t draw_area;
        draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
        draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
        draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
        draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
        fill_fp(&draw_area, mask, style->line.color, style->line.opa);

        return;
    }

    /*Special case draw a vertical line*/
    if(p1.x == p2.x ) {
        area_t act_area;
        act_area.x1 = p1.x - width_half;
        act_area.x2 = p2.x + width_half + width_1;
        act_area.y1 = p1.y;
        act_area.y2 = p2.y;

        area_t draw_area;
        draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
        draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
        draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
        draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
        fill_fp(&draw_area, mask, style->line.color, style->line.opa);
        return;
    }


    while(1){
        if(hor == true && last_y != act_point.y) {
            area_t act_area;
            area_t draw_area;
            act_area.x1 = last_x;
            act_area.x2 = act_point.x - sx;
            act_area.y1 = last_y - width_half ;
            act_area.y2 = act_point.y - sy  + width_half + width_1;
            last_y = act_point.y;
            last_x = act_point.x;
            draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
            draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
            draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
            draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
            if(width >= 0) {
                fill_fp(&draw_area, mask, style->line.color, style->line.opa);
            }

#if GUI_ANTIALIAS != 0
            coord_t seg_w = area_get_width(&draw_area);  /*Segment width*/
            point_t aa_p1;
            point_t aa_p2;

            aa_p1.x = draw_area.x1;
            aa_p1.y = draw_area.y1 - 1;

            aa_p2.x = draw_area.x1;
            aa_p2.y = draw_area.y1 + width + 1;

            coord_t i;
            for(i = 0; i  < seg_w; i++) {
                opa_t aa_opa = antialias_get_opa(seg_w, i, style->line.opa);

                px_fp(aa_p1.x + i, aa_p1.y, mask, style->line.color, aa_invert ? aa_opa : style->line.opa - aa_opa);
                px_fp(aa_p2.x + i, aa_p2.y, mask, style->line.color, aa_invert ? style->line.opa - aa_opa :  aa_opa);
            }
#endif
        }
        if (hor == false && last_x != act_point.x) {
            area_t act_area;
            area_t draw_area;
            act_area.x1 = last_x - width_half;
            act_area.x2 = act_point.x  - sx + width_half + width_1;
            act_area.y1 = last_y ;
            act_area.y2 = act_point.y - sy;
            last_y = act_point.y;
            last_x = act_point.x;

            draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
            draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
            draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
            draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
            if(width >= 0) {
                fill_fp(&draw_area, mask, style->line.color, style->line.opa);
            }

#if GUI_ANTIALIAS != 0
            coord_t seg_h = area_get_height(&draw_area);  /*Segment height*/
            point_t aa_p1;
            point_t aa_p2;

            aa_p1.x = draw_area.x1 - 1;
            aa_p1.y = draw_area.y1;

            aa_p2.x = draw_area.x1 + width + 1;
            aa_p2.y = draw_area.y1;

            coord_t i;
            for(i = 0; i  < seg_h; i++) {
                opa_t aa_opa = antialias_get_opa(seg_h, i, style->line.opa);

                px_fp(aa_p1.x, aa_p1.y + i, mask, style->line.color, aa_invert ? aa_opa : style->line.opa - aa_opa);
                px_fp(aa_p2.x, aa_p2.y + i, mask, style->line.color, aa_invert ? style->line.opa - aa_opa :  aa_opa);
            }
#endif
        }

        /*Calc. the next point of the line*/
        if (act_point.x == p2.x && act_point.y == p2.y) break;
        e2 = err;
        if (e2 >-dx) {
            err -= dy;
            act_point.x += sx;
        }
        if (e2 < dy) {
            err += dx;
            act_point.y += sy;
        }
    }

    /*Draw the last part of the line*/
    if(hor == true) {
        area_t act_area;
        area_t draw_area;
        act_area.x1 = last_x;
        act_area.x2 = act_point.x;
        act_area.y1 = last_y - width_half ;
        act_area.y2 = act_point.y + width_half + width_1;

        draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
        draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
        draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
        draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
        if(width >= 0) {
            fill_fp(&draw_area, mask, style->line.color, style->line.opa);
        }

#if GUI_ANTIALIAS != 0
        coord_t seg_w = area_get_width(&draw_area);  /*Segment width*/
        point_t aa_p1;
        point_t aa_p2;

        aa_p1.x = draw_area.x1;
        aa_p1.y = draw_area.y1 - 1;

        aa_p2.x = draw_area.x1;
        aa_p2.y = draw_area.y1 + width + 1;

        coord_t i;
        for(i = 0; i  < seg_w; i++) {
            opa_t aa_opa = antialias_get_opa(seg_w, i, style->line.opa);

            px_fp(aa_p1.x + i, aa_p1.y, mask, style->line.color, aa_invert ? aa_opa : style->line.opa - aa_opa);
            px_fp(aa_p2.x + i, aa_p2.y, mask, style->line.color, aa_invert ? style->line.opa - aa_opa :  aa_opa);
        }
#endif

    }
    if (hor == false) {
        area_t act_area;
        area_t draw_area;
        act_area.x1 = last_x - width_half;
        act_area.x2 = act_point.x + width_half + width_1;
        act_area.y1 = last_y;
        act_area.y2 = act_point.y;

        draw_area.x1 = RTE_MATH_MIN(act_area.x1, act_area.x2);
        draw_area.x2 = RTE_MATH_MAX(act_area.x1, act_area.x2);
        draw_area.y1 = RTE_MATH_MIN(act_area.y1, act_area.y2);
        draw_area.y2 = RTE_MATH_MAX(act_area.y1, act_area.y2);
        if(width >= 0) {
            fill_fp(&draw_area, mask, style->line.color, style->line.opa);
        }

#if GUI_ANTIALIAS != 0
        coord_t seg_h = area_get_height(&draw_area);  /*Segment height*/
        point_t aa_p1;
        point_t aa_p2;

        aa_p1.x = draw_area.x1 - 1;
        aa_p1.y = draw_area.y1;

        aa_p2.x = draw_area.x1 + width + 1;
        aa_p2.y = draw_area.y1;

        coord_t i;
        for(i = 0; i  < seg_h; i++) {
            opa_t aa_opa = antialias_get_opa(seg_h, i, style->line.opa);

            px_fp(aa_p1.x, aa_p1.y + i, mask, style->line.color, aa_invert ? aa_opa : style->line.opa - aa_opa);
            px_fp(aa_p2.x, aa_p2.y + i, mask, style->line.color, aa_invert ? style->line.opa - aa_opa :  aa_opa);
        }
#endif
    }
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Draw the middle part (rectangular) of a rectangle
 * @param coords the coordinates of the original rectangle
 * @param mask the rectangle will be drawn only  on this area
 * @param rects_p pointer to a rectangle style
 */
static void draw_rect_main_mid(const area_t * coords, const area_t * mask, const style_t * style)
{
    uint16_t radius = style->body.radius;

    color_t mcolor = style->body.main_color;
    color_t gcolor = style->body.grad_color;
    uint8_t mix;
    opa_t opa = style->body.opa;
    coord_t height = area_get_height(coords);
    coord_t width = area_get_width(coords);

    radius = draw_cont_radius_corr(radius, width, height);

    /*If the radius is too big then there is no body*/
    if(radius > height / 2)  return;

	area_t work_area;
	work_area.x1 = coords->x1;
	work_area.x2 = coords->x2;

    if(mcolor.full == gcolor.full) {
    	work_area.y1 = coords->y1 + radius;
    	work_area.y2 = coords->y2 - radius;

        if(style->body.radius != 0) {
#if GUI_ANTIALIAS
            work_area.y1 += 2;
            work_area.y2 -= 2;
#else
            work_area.y1 += 1;
            work_area.y2 -= 1;
#endif
        }

		fill_fp(&work_area, mask, mcolor, opa);
    } else {
		coord_t row;
		coord_t row_start = coords->y1 + radius;
		coord_t row_end = coords->y2 - radius;
		color_t act_color;

        if(style->body.radius != 0) {
#if GUI_ANTIALIAS
		    row_start += 2;
            row_end -= 2;
#else
            row_start += 1;
            row_end -= 1;
#endif
		}
        if(row_start < 0) row_start = 0;

		for(row = row_start; row <= row_end; row ++)
		{
			work_area.y1 = row;
			work_area.y2 = row;
			mix = (uint32_t)((uint32_t)(coords->y2 - work_area.y1) * 255) / height;
			act_color = color_mix(mcolor, gcolor, mix);

			fill_fp(&work_area, mask, act_color, opa);
		}
    }
}
/**
 * Draw the top and bottom parts (corners) of a rectangle
 * @param coords the coordinates of the original rectangle
 * @param mask the rectangle will be drawn only  on this area
 * @param rects_p pointer to a rectangle style
 */
static void draw_rect_main_corner(const area_t * coords, const area_t * mask, const style_t * style)
{
    uint16_t radius = style->body.radius;

    color_t mcolor = style->body.main_color;
    color_t gcolor = style->body.grad_color;
    color_t act_color;
    opa_t opa = style->body.opa;
    uint8_t mix;
    coord_t height = area_get_height(coords);
    coord_t width = area_get_width(coords);

    radius = draw_cont_radius_corr(radius, width, height);

    point_t lt_origo;   /*Left  Top    origo*/
    point_t lb_origo;   /*Left  Bottom origo*/
    point_t rt_origo;   /*Right Top    origo*/
    point_t rb_origo;   /*Left  Bottom origo*/

    lt_origo.x = coords->x1 + radius + GUI_ANTIALIAS;
    lt_origo.y = coords->y1 + radius + GUI_ANTIALIAS;

    lb_origo.x = coords->x1 + radius + GUI_ANTIALIAS;
    lb_origo.y = coords->y2 - radius - GUI_ANTIALIAS;

    rt_origo.x = coords->x2 - radius - GUI_ANTIALIAS;
    rt_origo.y = coords->y1 + radius + GUI_ANTIALIAS;

    rb_origo.x = coords->x2 - radius - GUI_ANTIALIAS;
    rb_origo.y = coords->y2 - radius - GUI_ANTIALIAS;

    area_t edge_top_area;
    area_t mid_top_area;
    area_t mid_bot_area;
    area_t edge_bot_area;

    point_t cir;
    coord_t cir_tmp;
    circ_init(&cir, &cir_tmp, radius);

    /*Init the areas*/
    area_set(&mid_bot_area,  lb_origo.x + GUI_CIRC_OCT4_X(cir),
                             lb_origo.y + GUI_CIRC_OCT4_Y(cir),
                             rb_origo.x + GUI_CIRC_OCT1_X(cir),
                             rb_origo.y + GUI_CIRC_OCT1_Y(cir));

    area_set(&edge_bot_area, lb_origo.x + GUI_CIRC_OCT3_X(cir),
                             lb_origo.y + GUI_CIRC_OCT3_Y(cir),
                             rb_origo.x + GUI_CIRC_OCT2_X(cir),
                             rb_origo.y + GUI_CIRC_OCT2_Y(cir));

    area_set(&mid_top_area,  lt_origo.x + GUI_CIRC_OCT5_X(cir),
                             lt_origo.y + GUI_CIRC_OCT5_Y(cir),
                             rt_origo.x + GUI_CIRC_OCT8_X(cir),
                             rt_origo.y + GUI_CIRC_OCT8_Y(cir));

    area_set(&edge_top_area, lt_origo.x + GUI_CIRC_OCT6_X(cir),
                             lt_origo.y + GUI_CIRC_OCT6_Y(cir),
                             rt_origo.x + GUI_CIRC_OCT7_X(cir),
                             rt_origo.y + GUI_CIRC_OCT7_Y(cir));
#if GUI_ANTIALIAS
    /*Store some internal states for anti-aliasing*/
    coord_t out_y_seg_start = 0;
    coord_t out_y_seg_end = 0;
    coord_t out_x_last = radius;

    color_t aa_color_hor_top;
    color_t aa_color_hor_bottom;
    color_t aa_color_ver;
#endif

    while(circ_cont(&cir)) {
#if GUI_ANTIALIAS != 0
        /*New step in y on the outter circle*/
        if(out_x_last != cir.x) {
            out_y_seg_end = cir.y;
            coord_t seg_size = out_y_seg_end - out_y_seg_start;
            point_t aa_p;

            aa_p.x = out_x_last;
            aa_p.y = out_y_seg_start;

            mix = (uint32_t)((uint32_t)(radius - out_x_last) * 255) / height;
            aa_color_hor_top = color_mix(gcolor, mcolor, mix);
            aa_color_hor_bottom = color_mix(mcolor, gcolor, mix);

            coord_t i;
            for(i = 0; i  < seg_size; i++) {
                opa_t aa_opa;
                if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) {    /*Use non-linear opa mapping on the first segment*/
                    aa_opa = antialias_get_opa_circ(seg_size, i, style->body.opa);
                } else {
                    aa_opa = opa - antialias_get_opa(seg_size, i, style->body.opa);
                }

                px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) + 1, mask, aa_color_hor_bottom, aa_opa);
                px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) + 1, mask, aa_color_hor_bottom, aa_opa);
                px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) - 1, mask, aa_color_hor_top, aa_opa);
                px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) - 1, mask, aa_color_hor_top, aa_opa);

                mix = (uint32_t)((uint32_t)(radius - out_y_seg_start + i) * 255) / height;
                aa_color_ver = color_mix(mcolor, gcolor, mix);
                px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, aa_color_ver, aa_opa);
                px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, aa_color_ver, aa_opa);

                aa_color_ver = color_mix(gcolor, mcolor, mix);
                px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, aa_color_ver, aa_opa);
                px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, aa_color_ver, aa_opa);
            }

            out_x_last = cir.x;
            out_y_seg_start = out_y_seg_end;
        }
#endif
        uint8_t edge_top_refr = 0;
        uint8_t mid_top_refr = 0;
        uint8_t mid_bot_refr = 0;
        uint8_t edge_bot_refr = 0;

        /* If a new row coming draw the previous
         * The y coordinate can remain the same so wait for a new*/
        if(mid_bot_area.y1 != GUI_CIRC_OCT4_Y(cir) + lb_origo.y ) mid_bot_refr = 1;

        if(edge_bot_area.y1 != GUI_CIRC_OCT2_Y(cir) + lb_origo.y) edge_bot_refr = 1;

        if(mid_top_area.y1 != GUI_CIRC_OCT8_Y(cir) + lt_origo.y) mid_top_refr = 1;

        if(edge_top_area.y1 != GUI_CIRC_OCT7_Y(cir) + lt_origo.y) edge_top_refr = 1;

        /*Draw the areas which are not disabled*/
        if(edge_top_refr != 0){
            if(mcolor.full == gcolor.full) act_color = mcolor;
            else {
                mix = (uint32_t)((uint32_t)(coords->y2 - edge_top_area.y1)  * 255) / height;
                act_color = color_mix(mcolor, gcolor, mix);
            }
            fill_fp(&edge_top_area, mask, act_color, opa);
        }

        if(mid_top_refr != 0) {
            if(mcolor.full == gcolor.full) act_color = mcolor;
            else {
                mix = (uint32_t)((uint32_t)(coords->y2 - mid_top_area.y1) * 255) / height;
                act_color = color_mix(mcolor, gcolor, mix);
            }
            fill_fp(&mid_top_area, mask, act_color, opa);
        }

        if(mid_bot_refr != 0) {
            if(mcolor.full == gcolor.full) act_color = mcolor;
            else {
                mix = (uint32_t)((uint32_t)(coords->y2 - mid_bot_area.y1) * 255) / height;
                act_color = color_mix(mcolor, gcolor, mix);
            }
            fill_fp(&mid_bot_area, mask, act_color, opa);
        }

        if(edge_bot_refr != 0) {

            if(mcolor.full == gcolor.full) act_color = mcolor;
            else {
                mix = (uint32_t)((uint32_t)(coords->y2 - edge_bot_area.y1) * 255) / height;
                act_color = color_mix(mcolor, gcolor, mix);
            }
            fill_fp(&edge_bot_area, mask, act_color, opa);
        }

         /*Save the current coordinates*/
        area_set(&mid_bot_area,  lb_origo.x + GUI_CIRC_OCT4_X(cir),
                                 lb_origo.y + GUI_CIRC_OCT4_Y(cir),
                                 rb_origo.x + GUI_CIRC_OCT1_X(cir),
                                 rb_origo.y + GUI_CIRC_OCT1_Y(cir));

        area_set(&edge_bot_area, lb_origo.x + GUI_CIRC_OCT3_X(cir),
                                 lb_origo.y + GUI_CIRC_OCT3_Y(cir),
                                 rb_origo.x + GUI_CIRC_OCT2_X(cir),
                                 rb_origo.y + GUI_CIRC_OCT2_Y(cir));

        area_set(&mid_top_area,  lt_origo.x + GUI_CIRC_OCT5_X(cir),
                                 lt_origo.y + GUI_CIRC_OCT5_Y(cir),
                                 rt_origo.x + GUI_CIRC_OCT8_X(cir),
                                 rt_origo.y + GUI_CIRC_OCT8_Y(cir));

        area_set(&edge_top_area, lt_origo.x + GUI_CIRC_OCT6_X(cir),
                                 lt_origo.y + GUI_CIRC_OCT6_Y(cir),
                                 rt_origo.x + GUI_CIRC_OCT7_X(cir),
                                 rt_origo.y + GUI_CIRC_OCT7_Y(cir));

        circ_next(&cir, &cir_tmp);
    }

    if(mcolor.full == gcolor.full) act_color = mcolor;
    else {
        mix = (uint32_t)((uint32_t)(coords->y2 - edge_top_area.y1)  * 255) / height;
        act_color = color_mix(mcolor, gcolor, mix);
    }
	fill_fp(&edge_top_area, mask, act_color, opa);

	if(edge_top_area.y1 != mid_top_area.y1) {

        if(mcolor.full == gcolor.full) act_color = mcolor;
        else {
            mix = (uint32_t)((uint32_t)(coords->y2 - mid_top_area.y1) * 255) / height;
            act_color = color_mix(mcolor, gcolor, mix);
	    }
		fill_fp(&mid_top_area, mask, act_color, opa);
	}

    if(mcolor.full == gcolor.full) act_color = mcolor;
    else {
        mix = (uint32_t)((uint32_t)(coords->y2 - mid_bot_area.y1) * 255) / height;
        act_color = color_mix(mcolor, gcolor, mix);
    }
	fill_fp(&mid_bot_area, mask, act_color, opa);

	if(edge_bot_area.y1 != mid_bot_area.y1) {

        if(mcolor.full == gcolor.full) act_color = mcolor;
        else {
            mix = (uint32_t)((uint32_t)(coords->y2 - edge_bot_area.y1) * 255) / height;
            act_color = color_mix(mcolor, gcolor, mix);
        }
		fill_fp(&edge_bot_area, mask, act_color, opa);
	}


#if GUI_ANTIALIAS
	/*The first and the last line is not drawn*/
	edge_top_area.x1 = coords->x1 + radius + 2;
    edge_top_area.x2 = coords->x2 - radius - 2;
    edge_top_area.y1 = coords->y1;
    edge_top_area.y2 = coords->y1;
    fill_fp(&edge_top_area, mask, style->body.main_color, opa);

    edge_top_area.y1 = coords->y2;
    edge_top_area.y2 = coords->y2;
    fill_fp(&edge_top_area, mask, style->body.grad_color, opa);

    /*Last parts of the anti-alias*/
    out_y_seg_end = cir.y;
    coord_t seg_size = out_y_seg_end - out_y_seg_start;
    point_t aa_p;

    aa_p.x = out_x_last;
    aa_p.y = out_y_seg_start;

    mix = (uint32_t)((uint32_t)(radius - out_x_last) * 255) / height;
    aa_color_hor_bottom = color_mix(gcolor, mcolor, mix);
    aa_color_hor_top = color_mix(mcolor, gcolor, mix);

    coord_t i;
    for(i = 0; i  < seg_size; i++) {
        opa_t aa_opa = opa - antialias_get_opa(seg_size, i, opa);
        px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) + 1, mask, aa_color_hor_top, aa_opa);
        px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) + 1, mask, aa_color_hor_top, aa_opa);
        px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) - 1, mask, aa_color_hor_bottom, aa_opa);
        px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) - 1, mask, aa_color_hor_bottom, aa_opa);

        mix = (uint32_t)((uint32_t)(radius - out_y_seg_start + i) * 255) / height;
        aa_color_ver = color_mix(mcolor, gcolor, mix);
        px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, aa_color_ver, aa_opa);
        px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, aa_color_ver, aa_opa);

        aa_color_ver = color_mix(gcolor, mcolor, mix);
        px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, aa_color_ver, aa_opa);
        px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, aa_color_ver, aa_opa);
    }

    /*In some cases the last pixel is not drawn*/
    if(RTE_MATH_ABS(aa_p.x - aa_p.y) == seg_size) {
        aa_p.x = out_x_last;
        aa_p.y = out_x_last;

        mix = (uint32_t)((uint32_t)(out_x_last) * 255) / height;
        aa_color_hor_top = color_mix(gcolor, mcolor, mix);
        aa_color_hor_bottom = color_mix(mcolor, gcolor, mix);

        opa_t aa_opa = style->body.opa >> 1;
        px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p), rb_origo.y + GUI_CIRC_OCT2_Y(aa_p), mask, aa_color_hor_bottom, aa_opa);
        px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p), lb_origo.y + GUI_CIRC_OCT4_Y(aa_p), mask, aa_color_hor_bottom, aa_opa);
        px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p), lt_origo.y + GUI_CIRC_OCT6_Y(aa_p), mask, aa_color_hor_top, aa_opa);
        px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p), rt_origo.y + GUI_CIRC_OCT8_Y(aa_p), mask, aa_color_hor_top, aa_opa);
    }

#endif


}

/**
 * Draw the straight parts of a rectangle border
 * @param coords the coordinates of the original rectangle
 * @param mask_ the rectangle will be drawn only  on this area
 * @param rstyle pointer to a rectangle style
 */
static void draw_rect_border_straight(const area_t * coords, const area_t * mask, const style_t * style)
{
    uint16_t radius = style->body.radius;

    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);
    uint16_t bwidth = style->body.border.width;
    opa_t opa = style->body.border.opa;
    border_part_t part = style->body.border.part;
    color_t color = style->body.border.color;
    area_t work_area;
    coord_t length_corr = 0;
    coord_t corner_size = 0;

    /*the 0 px border width drawn as 1 px, so decrement the b_width*/
    bwidth--;

    radius = draw_cont_radius_corr(radius, width, height);

    if(radius < bwidth) {
        length_corr = bwidth - radius - GUI_ANTIALIAS;
        corner_size = bwidth;
    } else {
        corner_size = radius + GUI_ANTIALIAS;
    }

    /* Modify the corner_size if corner is drawn */
    corner_size ++;

    /*Depending one which part's are drawn modify the area lengths */
    if(part & GUI_BORDER_TOP) work_area.y1 = coords->y1 + corner_size;
    else  work_area.y1 = coords->y1 + radius;

    if(part & GUI_BORDER_BOTTOM) work_area.y2 = coords->y2 - corner_size;
    else work_area.y2 = coords->y2 - radius;

    /*Left border*/
    if(part & GUI_BORDER_LEFT) {
        work_area.x1 = coords->x1;
        work_area.x2 = work_area.x1 + bwidth;
        fill_fp(&work_area, mask, color, opa);
    }

    /*Right border*/
    if(part & GUI_BORDER_RIGHT) {
        work_area.x2 = coords->x2;
        work_area.x1 = work_area.x2 - bwidth;
        fill_fp(&work_area, mask, color, opa);
    }

    work_area.x1 = coords->x1 + corner_size - length_corr;
    work_area.x2 = coords->x2 - corner_size + length_corr;

    /*Upper border*/
    if(part & GUI_BORDER_TOP) {
        work_area.y1 = coords->y1;
        work_area.y2 = coords->y1 + bwidth;
        fill_fp(&work_area, mask, color, opa);
    }

    /*Lower border*/
    if(part & GUI_BORDER_BOTTOM) {
        work_area.y2 = coords->y2;
        work_area.y1 = work_area.y2 - bwidth;
        fill_fp(&work_area, mask, color, opa);
    }

    /*Draw the a remaining rectangles if the radius is smaller then bwidth */
    if(length_corr != 0) {
        /*Left top correction*/
        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
            work_area.x1 = coords->x1;
            work_area.x2 = coords->x1 + radius + GUI_ANTIALIAS;
            work_area.y1 = coords->y1 + radius + 1 + GUI_ANTIALIAS;
            work_area.y2 = coords->y1 + bwidth;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Right top correction*/
        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
            work_area.x1 = coords->x2 - radius - GUI_ANTIALIAS;
            work_area.x2 = coords->x2;
            work_area.y1 = coords->y1 + radius + 1 + GUI_ANTIALIAS;
            work_area.y2 = coords->y1 + bwidth;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Left bottom correction*/
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
            work_area.x1 = coords->x1;
            work_area.x2 = coords->x1 + radius + GUI_ANTIALIAS;
            work_area.y1 = coords->y2 - bwidth;
            work_area.y2 = coords->y2 - radius - 1 - GUI_ANTIALIAS;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Right bottom correction*/
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
            work_area.x1 = coords->x2 - radius - GUI_ANTIALIAS;
            work_area.x2 = coords->x2;
            work_area.y1 = coords->y2 - bwidth;
            work_area.y2 = coords->y2 - radius - 1 - GUI_ANTIALIAS;
            fill_fp(&work_area, mask, color, opa);
        }
    }

    /*If radius == 0 one px on the corners are not drawn by main drawer*/
    if(style->body.radius == 0) {
        /*Left top corner*/
        if(part & (GUI_BORDER_TOP | GUI_BORDER_LEFT)) {
            work_area.x1 = coords->x1;
            work_area.x2 = coords->x1 + GUI_ANTIALIAS;
            work_area.y1 = coords->y1;
            work_area.y2 = coords->y1 + GUI_ANTIALIAS;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Right top corner*/
        if(part & (GUI_BORDER_TOP | GUI_BORDER_RIGHT)) {
            work_area.x1 = coords->x2 - GUI_ANTIALIAS;
            work_area.x2 = coords->x2;
            work_area.y1 = coords->y1;
            work_area.y2 = coords->y1 + GUI_ANTIALIAS;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Left bottom corner*/
        if(part & (GUI_BORDER_BOTTOM | GUI_BORDER_LEFT)) {
            work_area.x1 = coords->x1;
            work_area.x2 = coords->x1 + GUI_ANTIALIAS;
            work_area.y1 = coords->y2 - GUI_ANTIALIAS;
            work_area.y2 = coords->y2;
            fill_fp(&work_area, mask, color, opa);
        }

        /*Right bottom corner*/
        if(part & (GUI_BORDER_BOTTOM | GUI_BORDER_RIGHT)) {
            work_area.x1 = coords->x2 - GUI_ANTIALIAS;
            work_area.x2 = coords->x2;
            work_area.y1 = coords->y2 - GUI_ANTIALIAS;
            work_area.y2 = coords->y2;
            fill_fp(&work_area, mask, color, opa);
        }
    }
}


/**
 * Draw the corners of a rectangle border
 * @param coords the coordinates of the original rectangle
 * @param mask the rectangle will be drawn only  on this area
 * @param rects_p pointer to a rectangle style
 * @param opa opacity of the rectangle (0..255)
 */
static void draw_rect_border_corner(const area_t * coords, const area_t * mask, const  style_t * style)
{
    uint16_t radius = style->body.radius ;
    uint16_t bwidth = style->body.border.width;
    color_t color = style->body.border.color;
    opa_t opa = style->body.border.opa;
    border_part_t part = style->body.border.part;

    /*0 px border width drawn as 1 px, so decrement the bwidth*/
    bwidth--;

#if GUI_ANTIALIAS
    bwidth--;    /*Because of anti-aliasing the border seems one pixel ticker*/
#endif

    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);

    radius = draw_cont_radius_corr(radius, width, height);

    point_t lt_origo;   /*Left  Top    origo*/
    point_t lb_origo;   /*Left  Bottom origo*/
    point_t rt_origo;   /*Right Top    origo*/
    point_t rb_origo;   /*Left  Bottom origo*/

    lt_origo.x = coords->x1 + radius + GUI_ANTIALIAS;
    lt_origo.y = coords->y1 + radius + GUI_ANTIALIAS;

    lb_origo.x = coords->x1 + radius + GUI_ANTIALIAS;
    lb_origo.y = coords->y2 - radius - GUI_ANTIALIAS;

    rt_origo.x = coords->x2 - radius - GUI_ANTIALIAS;
    rt_origo.y = coords->y1 + radius + GUI_ANTIALIAS;

    rb_origo.x = coords->x2 - radius - GUI_ANTIALIAS;
    rb_origo.y = coords->y2 - radius - GUI_ANTIALIAS;

    point_t cir_out;
    coord_t tmp_out;
    circ_init(&cir_out, &tmp_out, radius);

    point_t cir_in;
    coord_t tmp_in;
    coord_t radius_in = radius - bwidth;

    if(radius_in < 0){
        radius_in = 0;
    }

    circ_init(&cir_in, &tmp_in, radius_in);

    area_t circ_area;
    coord_t act_w1;
    coord_t act_w2;

#if GUI_ANTIALIAS
    /*Store some internal states for anti-aliasing*/
    coord_t out_y_seg_start = 0;
    coord_t out_y_seg_end = 0;
    coord_t out_x_last = radius;


    coord_t in_y_seg_start = 0;
    coord_t in_y_seg_end = 0;
    coord_t in_x_last = radius - bwidth;
#endif

    while( cir_out.y <= cir_out.x) {

        /*Calculate the actual width to avoid overwriting pixels*/
        if(cir_in.y < cir_in.x) {
            act_w1 = cir_out.x - cir_in.x;
            act_w2 = act_w1;
        } else {
            act_w1 = cir_out.x - cir_out.y;
            act_w2 = act_w1 - 1;
        }

#if GUI_ANTIALIAS != 0
        /*New step in y on the outter circle*/
        if(out_x_last != cir_out.x) {
            out_y_seg_end = cir_out.y;
            coord_t seg_size = out_y_seg_end - out_y_seg_start;
            point_t aa_p;

            aa_p.x = out_x_last;
            aa_p.y = out_y_seg_start;

            coord_t i;
            for(i = 0; i  < seg_size; i++) {
                opa_t aa_opa;

                if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) {    /*Use non-linear opa mapping on the first segment*/
                    aa_opa = antialias_get_opa_circ(seg_size, i, style->body.border.opa);
                } else {
                    aa_opa = style->body.border.opa - antialias_get_opa(seg_size, i, style->body.border.opa);
                }

                if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
                    px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
                    px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
                }

                if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
                    px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
                    px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
                }


                if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
                    px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
                    px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
                }

                if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
                    px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
                    px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
                }
            }

            out_x_last = cir_out.x;
            out_y_seg_start = out_y_seg_end;
        }

        /*New step in y on the inner circle*/
       if(in_x_last != cir_in.x) {
           in_y_seg_end = cir_out.y;
           coord_t seg_size = in_y_seg_end - in_y_seg_start;
           point_t aa_p;

           aa_p.x = in_x_last;
           aa_p.y = in_y_seg_start;

           coord_t i;
           for(i = 0; i  < seg_size; i++) {
               opa_t aa_opa;

               if(seg_size > CIRCLE_AA_NON_LINEAR_OPA_THRESHOLD) {    /*Use non-linear opa mapping on the first segment*/
                   aa_opa = style->body.border.opa - antialias_get_opa_circ(seg_size, i, style->body.border.opa);
               } else {
                   aa_opa = antialias_get_opa(seg_size, i, style->body.border.opa);
               }

               if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
                   px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) - 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
               }

               if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
                   px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
               }

               if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
                   px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) + 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
               }

               if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
                  px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
               }

               /*Be sure the pixels on the middle are not drawn twice*/
               if(GUI_CIRC_OCT1_X(aa_p) - 1 != GUI_CIRC_OCT2_X(aa_p) + i) {
                   if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
                       px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
                   }

                   if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
                       px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) + 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
                   }

                   if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
                       px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
                   }

                   if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
                       px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) - 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
                   }
               }

           }

           in_x_last = cir_in.x;
           in_y_seg_start = in_y_seg_end;

       }

#endif


        /*Draw the octets to the right bottom corner*/
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
            circ_area.x1 = rb_origo.x + GUI_CIRC_OCT1_X(cir_out) - act_w2;
            circ_area.x2 = rb_origo.x + GUI_CIRC_OCT1_X(cir_out);
            circ_area.y1 = rb_origo.y + GUI_CIRC_OCT1_Y(cir_out);
            circ_area.y2 = rb_origo.y + GUI_CIRC_OCT1_Y(cir_out);
            fill_fp(&circ_area, mask, color, opa);

            circ_area.x1 = rb_origo.x + GUI_CIRC_OCT2_X(cir_out);
            circ_area.x2 = rb_origo.x + GUI_CIRC_OCT2_X(cir_out);
            circ_area.y1 = rb_origo.y + GUI_CIRC_OCT2_Y(cir_out)- act_w1;
            circ_area.y2 = rb_origo.y + GUI_CIRC_OCT2_Y(cir_out);
            fill_fp(&circ_area, mask, color, opa);
        }

        /*Draw the octets to the left bottom corner*/
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
            circ_area.x1 = lb_origo.x + GUI_CIRC_OCT3_X(cir_out);
            circ_area.x2 = lb_origo.x + GUI_CIRC_OCT3_X(cir_out);
            circ_area.y1 = lb_origo.y + GUI_CIRC_OCT3_Y(cir_out) - act_w2;
            circ_area.y2 = lb_origo.y + GUI_CIRC_OCT3_Y(cir_out);
            fill_fp(&circ_area, mask, color, opa);

            circ_area.x1 = lb_origo.x + GUI_CIRC_OCT4_X(cir_out);
            circ_area.x2 = lb_origo.x + GUI_CIRC_OCT4_X(cir_out) + act_w1;
            circ_area.y1 = lb_origo.y + GUI_CIRC_OCT4_Y(cir_out);
            circ_area.y2 = lb_origo.y + GUI_CIRC_OCT4_Y(cir_out);
            fill_fp(&circ_area, mask, color, opa);
        }

        /*Draw the octets to the left top corner*/
        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
            if(lb_origo.y + GUI_CIRC_OCT4_Y(cir_out) > lt_origo.y + GUI_CIRC_OCT5_Y(cir_out)) {
                /*Don't draw if the lines are common in the middle*/
                circ_area.x1 = lt_origo.x + GUI_CIRC_OCT5_X(cir_out);
                circ_area.x2 = lt_origo.x + GUI_CIRC_OCT5_X(cir_out) + act_w2;
                circ_area.y1 = lt_origo.y + GUI_CIRC_OCT5_Y(cir_out);
                circ_area.y2 = lt_origo.y + GUI_CIRC_OCT5_Y(cir_out);
                fill_fp(&circ_area, mask, color, opa);
            }

            circ_area.x1 = lt_origo.x + GUI_CIRC_OCT6_X(cir_out);
            circ_area.x2 = lt_origo.x + GUI_CIRC_OCT6_X(cir_out);
            circ_area.y1 = lt_origo.y + GUI_CIRC_OCT6_Y(cir_out);
            circ_area.y2 = lt_origo.y + GUI_CIRC_OCT6_Y(cir_out) + act_w1;
            fill_fp(&circ_area, mask, color, opa);
        }

        /*Draw the octets to the right top corner*/
        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
            circ_area.x1 = rt_origo.x + GUI_CIRC_OCT7_X(cir_out);
            circ_area.x2 = rt_origo.x + GUI_CIRC_OCT7_X(cir_out);
            circ_area.y1 = rt_origo.y + GUI_CIRC_OCT7_Y(cir_out);
            circ_area.y2 = rt_origo.y + GUI_CIRC_OCT7_Y(cir_out) + act_w2;
            fill_fp(&circ_area, mask, color, opa);

            /*Don't draw if the lines are common in the middle*/
            if(rb_origo.y + GUI_CIRC_OCT1_Y(cir_out) > rt_origo.y + GUI_CIRC_OCT8_Y(cir_out)) {
                circ_area.x1 = rt_origo.x + GUI_CIRC_OCT8_X(cir_out) - act_w1;
                circ_area.x2 = rt_origo.x + GUI_CIRC_OCT8_X(cir_out);
                circ_area.y1 = rt_origo.y + GUI_CIRC_OCT8_Y(cir_out);
                circ_area.y2 = rt_origo.y + GUI_CIRC_OCT8_Y(cir_out);
                fill_fp(&circ_area, mask, color, opa);
            }
        }
        circ_next(&cir_out, &tmp_out);

        /*The internal circle will be ready faster
         * so check it! */
        if(cir_in.y < cir_in.x) {
            circ_next(&cir_in, &tmp_in);
        }
    }


#if GUI_ANTIALIAS != 0

    /*Last parts of the outer anti-alias*/
    out_y_seg_end = cir_out.y;
    coord_t seg_size = out_y_seg_end - out_y_seg_start;
    point_t aa_p;

    aa_p.x = out_x_last;
    aa_p.y = out_y_seg_start;

    coord_t i;
    for(i = 0; i  < seg_size; i++) {
        opa_t aa_opa = style->body.border.opa - antialias_get_opa(seg_size, i, style->body.border.opa);
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) + 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
            px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
            px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
            px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) - 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
            px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) - 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
            px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
            px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) + 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
        }
    }

    /*In some cases the last pixel in the outer middle is not drawn*/
    if(RTE_MATH_ABS(aa_p.x - aa_p.y) == seg_size) {
        aa_p.x = out_x_last;
        aa_p.y = out_x_last;

        opa_t aa_opa = style->body.border.opa >> 1;

        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p), rb_origo.y + GUI_CIRC_OCT2_Y(aa_p), mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
            px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p), lb_origo.y + GUI_CIRC_OCT4_Y(aa_p), mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
            px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p), lt_origo.y + GUI_CIRC_OCT6_Y(aa_p), mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p), rt_origo.y + GUI_CIRC_OCT8_Y(aa_p), mask, style->body.border.color, aa_opa);
        }
    }

    /*Last parts of the inner anti-alias*/
    in_y_seg_end = cir_in.y;
    aa_p.x = in_x_last;
    aa_p.y = in_y_seg_start;
    seg_size = in_y_seg_end - in_y_seg_start;

    for(i = 0; i  < seg_size; i++) {
        opa_t aa_opa =  antialias_get_opa(seg_size, i, style->body.border.opa);
        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rb_origo.x + GUI_CIRC_OCT1_X(aa_p) - 1, rb_origo.y + GUI_CIRC_OCT1_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
            px_fp(lb_origo.x + GUI_CIRC_OCT3_X(aa_p) - i, lb_origo.y + GUI_CIRC_OCT3_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
            px_fp(lt_origo.x + GUI_CIRC_OCT5_X(aa_p) + 1, lt_origo.y + GUI_CIRC_OCT5_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
        }

        if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
            px_fp(rt_origo.x + GUI_CIRC_OCT7_X(aa_p) + i, rt_origo.y + GUI_CIRC_OCT7_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
        }

        if(GUI_CIRC_OCT1_X(aa_p) - 1 != GUI_CIRC_OCT2_X(aa_p) + i) {
            if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_RIGHT)) {
                px_fp(rb_origo.x + GUI_CIRC_OCT2_X(aa_p) + i, rb_origo.y + GUI_CIRC_OCT2_Y(aa_p) - 1, mask, style->body.border.color, aa_opa);
            }

            if((part & GUI_BORDER_BOTTOM) && (part & GUI_BORDER_LEFT)) {
                px_fp(lb_origo.x + GUI_CIRC_OCT4_X(aa_p) + 1, lb_origo.y + GUI_CIRC_OCT4_Y(aa_p) + i, mask, style->body.border.color, aa_opa);
            }

            if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_LEFT)) {
                px_fp(lt_origo.x + GUI_CIRC_OCT6_X(aa_p) - i, lt_origo.y + GUI_CIRC_OCT6_Y(aa_p) + 1, mask, style->body.border.color, aa_opa);
            }

            if((part & GUI_BORDER_TOP) && (part & GUI_BORDER_RIGHT)) {
                px_fp(rt_origo.x + GUI_CIRC_OCT8_X(aa_p) - 1, rt_origo.y + GUI_CIRC_OCT8_Y(aa_p) - i, mask, style->body.border.color, aa_opa);
            }
        }
    }

#endif

}

#if GUI_USE_SHADOW && GUI_VDB_SIZE

/**
 * Draw a shadow
 * @param rect pointer to rectangle object
 * @param mask pointer to a mask area (from the design functions)
 */
static void draw_shadow(const area_t * coords, const area_t * mask, const  style_t * style)
{
    /* If mask is in the middle of cords do not draw shadow*/
    coord_t radius = style->body.radius;
    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);
    radius = draw_cont_radius_corr(radius, width, height);
    area_t area_tmp;

    /*Check horizontally without radius*/
    area_copy(&area_tmp, coords);
    area_tmp.x1 += radius;
    area_tmp.x2 -= radius;
    if(area_is_in(mask, &area_tmp) != false) return;

    /*Check vertically without radius*/
    area_copy(&area_tmp, coords);
    area_tmp.y1 += radius;
    area_tmp.y2 -= radius;
    if(area_is_in(mask, &area_tmp) != false) return;

    if(style->body.shadow.type == GUI_SHADOW_FULL) {
        draw_shadow_full(coords, mask, style);
    } else if(style->body.shadow.type == GUI_SHADOW_BOTTOM) {
        draw_shadow_bottom(coords, mask, style);
    }
}

static void draw_shadow_full(const area_t * coords, const area_t * mask, const style_t * style)
{

    coord_t radius = style->body.radius;
    coord_t swidth = style->body.shadow.width;

    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);

    radius = draw_cont_radius_corr(radius, width, height);

    if(radius != 0) radius -= GUI_ANTIALIAS;
    swidth += GUI_ANTIALIAS;

#if GUI_COMPILER_VLA_SUPPORTED
    coord_t curve_x[radius + swidth + 1];     /*Stores the 'x' coordinates of a quarter circle.*/
#else
# if GUI_HOR_RES > GUI_VER_RES
    coord_t curve_x[GUI_HOR_RES];
# else
    coord_t curve_x[GUI_VER_RES];
# endif
#endif
    memset(curve_x, 0, sizeof(curve_x));
    point_t circ;
    coord_t circ_tmp;
    circ_init(&circ, &circ_tmp, radius);
    while(circ_cont(&circ)) {
        curve_x[GUI_CIRC_OCT1_Y(circ)] = GUI_CIRC_OCT1_X(circ);
        curve_x[GUI_CIRC_OCT2_Y(circ)] = GUI_CIRC_OCT2_X(circ);
        circ_next(&circ, &circ_tmp);
    }
    int16_t line;

    int16_t filter_width = 2 * swidth + 1;
#if GUI_COMPILER_VLA_SUPPORTED
    uint32_t *line_1d_blur = (uint32_t *)RTE_BGet(MEM_RTE,filter_width*sizeof(uint32_t));
#else
# if GUI_HOR_RES > GUI_VER_RES
    uint32_t line_1d_blur[GUI_HOR_RES];
# else
    uint32_t line_1d_blur[GUI_VER_RES];
# endif
#endif
    /*1D Blur horizontally*/
    for(line = 0; line < filter_width; line++) {
        line_1d_blur[line] = (uint32_t)((uint32_t)(filter_width - line) * (style->body.opa * 2)  << SHADOW_OPA_EXTRA_PRECISION) / (filter_width * filter_width);
    }

    uint16_t col;
#if GUI_COMPILER_VLA_SUPPORTED
    opa_t line_2d_blur[radius + swidth];
#else
# if GUI_HOR_RES > GUI_VER_RES
    opa_t line_2d_blur[GUI_HOR_RES];
# else
    opa_t line_2d_blur[GUI_VER_RES];
# endif
#endif

    point_t point_rt;
    point_t point_rb;
    point_t point_lt;
    point_t point_lb;
    point_t ofs_rb;
    point_t ofs_rt;
    point_t ofs_lb;
    point_t ofs_lt;
    ofs_rb.x = coords->x2 - radius - GUI_ANTIALIAS;
    ofs_rb.y = coords->y2 - radius - GUI_ANTIALIAS;

    ofs_rt.x = coords->x2 - radius - GUI_ANTIALIAS;
    ofs_rt.y = coords->y1 + radius + GUI_ANTIALIAS;

    ofs_lb.x = coords->x1 + radius + GUI_ANTIALIAS;
    ofs_lb.y = coords->y2 - radius - GUI_ANTIALIAS;

    ofs_lt.x = coords->x1 + radius + GUI_ANTIALIAS;
    ofs_lt.y = coords->y1 + radius + GUI_ANTIALIAS;
    bool line_ready;
    for(line = 1; line <= radius + swidth; line++) {        /*Check all rows and make the 1D blur to 2D*/
        line_ready = false;
        for(col = 1; col < radius + swidth; col++) {        /*Check all pixels in a 1D blur line (from the origo to last shadow pixel (radius + swidth))*/

            /*Sum the opacities from the lines above and below this 'row'*/
            int16_t line_rel;
            uint32_t px_opa_sum = 0;
            for(line_rel = -swidth; line_rel <= swidth; line_rel ++) {
                /*Get the relative x position of the 'line_rel' to 'line'*/
                int16_t col_rel;
                if(line + line_rel < 0) {                       /*Below the radius, here is the blur of the edge */
                    col_rel = radius - curve_x[line] - col;
                } else if(line + line_rel > radius) {           /*Above the radius, here won't be more 1D blur*/
                    break;
                } else {                                        /*Blur from the curve*/
                    col_rel = curve_x[line + line_rel] - curve_x[line] - col;
                }

                /*Add the value of the 1D blur on 'col_rel' position*/
                if(col_rel < -swidth) {                         /*Outside of the blurred area. */
                    if(line_rel == -swidth) line_ready = true;  /*If no data even on the very first line then it wont't be anything else in this line*/
                    break;                                      /*Break anyway because only smaller 'col_rel' values will come */
                }
                else if (col_rel > swidth) px_opa_sum += line_1d_blur[0];       /*Inside the not blurred area*/
                else px_opa_sum += line_1d_blur[swidth - col_rel];              /*On the 1D blur (+ swidth to align to the center)*/
            }

            line_2d_blur[col] = px_opa_sum >> SHADOW_OPA_EXTRA_PRECISION;
            if(line_ready) {
                col++;      /*To make this line to the last one ( drawing will go to '< col')*/
                break;
            }

        }

        /*Flush the line*/
        point_rt.x = curve_x[line] + ofs_rt.x + 1;
        point_rt.y = ofs_rt.y - line;

        point_rb.x = curve_x[line] + ofs_rb.x + 1;
        point_rb.y = ofs_rb.y + line;

        point_lt.x = ofs_lt.x -  curve_x[line] - 1;
        point_lt.y = ofs_lt.y - line;

        point_lb.x = ofs_lb.x - curve_x[line] - 1;
        point_lb.y = ofs_lb.y + line;

        uint16_t d;
        for(d = 1; d < col; d++) {

            if(point_rt.x != point_lt.x) {
                px_fp(point_lt.x,point_lt.y , mask, style->body.shadow.color, line_2d_blur[d]);
            }

            if(point_rb.x != point_lb.x && point_lt.y != point_lb.y) {
                px_fp(point_lb.x,point_lb.y , mask, style->body.shadow.color, line_2d_blur[d]);
            }

            if(point_lt.y != point_lb.y) {
                px_fp(point_rb.x,point_rb.y , mask, style->body.shadow.color, line_2d_blur[d]);
            }

            px_fp(point_rt.x,point_rt.y , mask, style->body.shadow.color, line_2d_blur[d]);


            point_rb.x++;
            point_lb.x--;

            point_rt.x++;
            point_lt.x--;
        }

        /* Put the first line to the edges too.
         * It is not correct because blur should be done below the corner too
         * but is is simple, fast and gives a good enough result*/
        if(line == 1) draw_shadow_full_straight(coords, mask, style, line_2d_blur);
    }
		RTE_BRel(MEM_RTE,line_1d_blur);
}


static void draw_shadow_bottom(const area_t * coords, const area_t * mask, const style_t * style)
{
    coord_t radius = style->body.radius;
    coord_t swidth = style->body.shadow.width;
    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);

    radius = draw_cont_radius_corr(radius, width, height);
    radius += GUI_ANTIALIAS * SHADOW_BOTTOM_AA_EXTRA_RADIUS;
    swidth += GUI_ANTIALIAS;
#if GUI_COMPILER_VLA_SUPPORTED
    coord_t curve_x[radius + 1];             /*Stores the 'x' coordinates of a quarter circle.*/
#else
# if GUI_HOR_RES > GUI_VER_RES
    coord_t curve_x[GUI_HOR_RES];
# else
    coord_t curve_x[GUI_VER_RES];
# endif
#endif
    point_t circ;
    coord_t circ_tmp;
    circ_init(&circ, &circ_tmp, radius);
    while(circ_cont(&circ)) {
        curve_x[GUI_CIRC_OCT1_Y(circ)] = GUI_CIRC_OCT1_X(circ);
        curve_x[GUI_CIRC_OCT2_Y(circ)] = GUI_CIRC_OCT2_X(circ);
        circ_next(&circ, &circ_tmp);
    }

    int16_t col;
    #if GUI_COMPILER_VLA_SUPPORTED
    opa_t line_1d_blur[swidth];
#else
# if GUI_HOR_RES > GUI_VER_RES
    opa_t line_1d_blur[GUI_HOR_RES];
# else
    opa_t line_1d_blur[GUI_VER_RES];
# endif
#endif

    for(col = 0; col < swidth; col++) {
        line_1d_blur[col] = (uint32_t)((uint32_t)(swidth - col) * style->body.opa / 2) / (swidth);
    }

    point_t point_l;
    point_t point_r;
    area_t area_mid;
    point_t ofs_l;
    point_t ofs_r;

    ofs_l.x = coords->x1 + radius;
    ofs_l.y = coords->y2 - radius + 1 - GUI_ANTIALIAS;

    ofs_r.x = coords->x2 - radius;
    ofs_r.y = coords->y2 - radius + 1 - GUI_ANTIALIAS;

    for(col = 0; col <= radius; col++) {
        point_l.x = ofs_l.x - col ;
        point_l.y = ofs_l.y + curve_x[col];

        point_r.x = ofs_r.x + col;
        point_r.y = ofs_r.y + curve_x[col];

        opa_t px_opa;
        int16_t diff = col == 0 ? 0 : curve_x[col-1] - curve_x[col];
        uint16_t d;
        for(d = 0; d < swidth; d++) {
            /*When stepping a pixel in y calculate the average with the pixel from the prev. column to make a blur */
            if(diff == 0) {
                px_opa = line_1d_blur[d];
            } else {
                px_opa = (uint16_t)((uint16_t)line_1d_blur[d] + line_1d_blur[d - diff]) >> 1;
            }
            px_fp(point_l.x, point_l.y, mask, style->body.shadow.color, px_opa);
            point_l.y ++;

            /*Don't overdraw the pixel on the middle*/
            if(point_r.x > ofs_l.x) {
                px_fp(point_r.x, point_r.y, mask, style->body.shadow.color, px_opa);
            }
            point_r.y ++;
        }

    }

    area_mid.x1 = ofs_l.x + 1;
    area_mid.y1 = ofs_l.y + radius;
    area_mid.x2 = ofs_r.x - 1;
    area_mid.y2 = area_mid.y1;

    uint16_t d;
    for(d = 0; d < swidth; d++) {
        fill_fp(&area_mid, mask, style->body.shadow.color, line_1d_blur[d]);
        area_mid.y1 ++;
        area_mid.y2 ++;
    }
}

static void draw_shadow_full_straight(const area_t * coords, const area_t * mask, const style_t * style, const opa_t * map)
{
    coord_t radius = style->body.radius;
    coord_t swidth = style->body.shadow.width + GUI_ANTIALIAS;
    coord_t width = area_get_width(coords);
    coord_t height = area_get_height(coords);

    radius = draw_cont_radius_corr(radius, width, height);
    if(radius == 0) radius += GUI_ANTIALIAS;

    area_t right_area;
    right_area.x1 = coords->x2 + 1 - GUI_ANTIALIAS;
    right_area.y1 = coords->y1 + radius;
    right_area.x2 = right_area.x1;
    right_area.y2 = coords->y2 -  radius;

    area_t left_area;
    left_area.x1 = coords->x1 - 1 + GUI_ANTIALIAS;
    left_area.y1 = coords->y1 + radius;
    left_area.x2 = left_area.x1;
    left_area.y2 = coords->y2 - radius;

    area_t top_area;
    top_area.x1 = coords->x1 + radius;
    top_area.y1 = coords->y1 - 1 + GUI_ANTIALIAS;
    top_area.x2 = coords->x2 - radius;
    top_area.y2 = top_area.y1;

    area_t bottom_area;
    bottom_area.x1 = coords->x1 + radius;
    bottom_area.y1 = coords->y2 + 1 - GUI_ANTIALIAS;
    bottom_area.x2 = coords->x2 - radius;
    bottom_area.y2 = bottom_area.y1;

    opa_t opa_act;
    int16_t d;
    for(d = 1; d <= swidth; d++) {
        opa_act = map[d];
        fill_fp(&right_area, mask, style->body.shadow.color, opa_act);
        right_area.x1++;
        right_area.x2++;

        fill_fp(&left_area, mask, style->body.shadow.color, opa_act);
        left_area.x1--;
        left_area.x2--;

        fill_fp(&top_area, mask, style->body.shadow.color, opa_act);
        top_area.y1--;
        top_area.y2--;

        fill_fp(&bottom_area, mask, style->body.shadow.color, opa_act);
        bottom_area.y1++;
        bottom_area.y2++;
    }

}

#endif

static uint16_t draw_cont_radius_corr(uint16_t r, coord_t w, coord_t h)
{
	if(r >= (w >> 1)){
		r = (w >> 1);
		if(r != 0) r--;
	}
	if(r >= (h >> 1)) {
		r = (h >> 1);
		if(r != 0) r--;
	}

    if(r > 0) r -= GUI_ANTIALIAS;

	return r;
}

#if GUI_ANTIALIAS != 0
static opa_t antialias_get_opa(coord_t seg, coord_t px_id, opa_t line_opa)
{
    /* How to calculate the opacity of pixels on the edges which makes the anti-aliasing?
     * For example we have a line like this (y = -0.5 * x):
     *
     *  | _ _
     *    * * |
     *
     * Anti-aliased pixels come to the '*' characters
     * Calculate what percentage of the pixels should be covered if real line (not rasterized) would be drawn:
     * 1. A real line should start on (0;0) and end on (2;1)
     * 2. So the line intersection coordinates on the first pixel: (0;0) (1;0.5) -> 25% covered pixel in average
     * 3. For the second pixel: (1;0.5) (2;1) -> 75% covered pixel in average
     * 4. The equation: (px_id * 2 + 1) / (segment_width * 2)
     *                   segment_width: the line segment which is being anti-aliased (was 2 in the example)
     *                   px_id: pixel ID from 0 to  (segment_width - 1)
     *                   result: [0..1] coverage of the pixel
     */

    /*Accelerate the common segment sizes to avoid division*/
    static const  opa_t seg1[1] = {128};
    static const  opa_t seg2[2] = {64, 192};
    static const  opa_t seg3[3] = {42, 128, 212};
    static const  opa_t seg4[4] = {32, 96, 159, 223};
    static const  opa_t seg5[5] = {26,  76, 128, 178, 230};
    static const  opa_t seg6[6] = {21,  64, 106, 148, 191, 234};
    static const  opa_t seg7[7] = {18,  55, 91, 128, 164, 200, 237};
    static const  opa_t seg8[8] = {16,  48, 80, 112, 143, 175, 207, 239};

    static const opa_t * seg_map[] = {seg1, seg2, seg3, seg4,
                                         seg5, seg6, seg7, seg8};

    if(seg == 0) return GUI_OPA_TRANSP;
    else if(seg < 8) return (uint32_t)((uint32_t)seg_map[seg - 1][px_id] * line_opa) >> 8;
    else {
        return ((px_id * 2 + 1) * line_opa) / (2 * seg);
    }

}

/**
 * Approximate the opacity for anti-aliasing.
 * Used  the first segment of a circle which is the longest and have the most non-linearity (cos)
 * @param seg
 * @param px_id
 * @param line_opa
 * @return
 */
static opa_t antialias_get_opa_circ(coord_t seg, coord_t px_id, opa_t opa)
{
    static const  opa_t opa_map[8] = {250, 242, 221, 196, 163, 122, 74, 18};

    if(seg == 0) return GUI_OPA_TRANSP;
    else if(seg == 1) return GUI_OPA_80;
    else {

        uint8_t id = (uint32_t) ((uint32_t)px_id * (sizeof(opa_map) - 1)) / (seg - 1);
        return (uint32_t) ((uint32_t) opa_map[id] * opa) >> 8;

    }

}

#endif


/**
 * Convert a hexadecimal characters to a number (0..15)
 * @param hex Pointer to a hexadecimal character (0..9, A..F)
 * @return the numerical value of `hex` or 0 on error
 */
static uint8_t hex_char_to_num(char hex)
{
    if(hex >= '0' && hex <= '9') {
        return hex - '0';
    }

    if(hex >= 'a') hex -= 'a' - 'A';    /*Convert to upper case*/

    switch(hex) {
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        default: return 0;
    }


}

#if GUI_USE_TRIANGLE != 0
/**
 * Swap two points
 * p1 pointer to the first point
 * p2 pointer to the second point
 */
static void point_swap(point_t * p1, point_t * p2)
{
    point_t tmp;
    tmp.x = p1->x;
    tmp.y = p1->y;

    p1->x = p2->x;
    p1->y = p2->y;

    p2->x = tmp.x;
    p2->y = tmp.y;

}

#endif


#endif
