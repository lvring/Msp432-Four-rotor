/**
 * @file btnm.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_BTNM != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_BTNM_PR_NONE         0xFFFF

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t btnm_signal(obj_t * btnm, signal_t sign, void * param);
static bool btnm_design(obj_t * btnm, const area_t * mask, design_mode_t mode);
static uint8_t get_button_width(const char * btn_str);
static bool button_is_hidden(const char * btn_str);
static bool button_is_repeat_disabled(const char * btn_str);
static bool button_is_inactive(const char * btn_str);
const char * cut_ctrl_byte(const char * btn_str);
static uint16_t get_button_from_point(obj_t * btnm, point_t * p);
static uint16_t get_button_text(obj_t * btnm, uint16_t btn_id);
static void create_buttons(obj_t * btnm, const char ** map);

/**********************
 *  STATIC VARIABLES
 **********************/
static const char * btnm_def_map[] = {"Btn1","Btn2", "Btn3","\n",
										 "\002Btn4","Btn5", ""};

static design_func_t ancestor_design_f;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a button matrix objects
 * @param par pointer to an object, it will be the parent of the new button matrix
 * @param copy pointer to a button matrix object, if not NULL then the new object will be copied from it
 * @return pointer to the created button matrix
 */
obj_t * btnm_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor object*/
    obj_t * new_btnm = obj_create(par, copy);
    RTE_AssertParam(new_btnm);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_btnm);
    
    /*Allocate the object type specific extended data*/
    btnm_ext_t * ext = obj_allocate_ext_attr(new_btnm, sizeof(btnm_ext_t));
    RTE_AssertParam(ext);
    ext->btn_cnt = 0;
    ext->btn_id_pr = GUI_BTNM_PR_NONE;
    ext->btn_id_tgl = GUI_BTNM_PR_NONE;
    ext->button_areas = NULL;
    ext->action = NULL;
    ext->map_p = NULL;
    ext->toggle = 0;
    ext->styles_btn[GUI_BTN_STATE_REL] = &style_btn_rel;
    ext->styles_btn[GUI_BTN_STATE_PR] = &style_btn_pr;
    ext->styles_btn[GUI_BTN_STATE_TGL_REL] = &style_btn_tgl_rel;
    ext->styles_btn[GUI_BTN_STATE_TGL_PR] = &style_btn_tgl_pr;
    ext->styles_btn[GUI_BTN_STATE_INA] = &style_btn_ina;

    if(ancestor_design_f == NULL) ancestor_design_f = obj_get_design_func(new_btnm);

    obj_set_signal_func(new_btnm, btnm_signal);
    obj_set_design_func(new_btnm, btnm_design);

    /*Init the new button matrix object*/
    if(copy == NULL) {
    	obj_set_size(new_btnm, GUI_HOR_RES / 2, GUI_VER_RES / 4);
    	btnm_set_map(new_btnm, btnm_def_map);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BG, th->btnm.bg);
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BTN_REL, th->btnm.btn.rel);
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BTN_PR, th->btnm.btn.pr);
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BTN_TGL_REL, th->btnm.btn.tgl_rel);
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BTN_TGL_PR, th->btnm.btn.tgl_pr);
            btnm_set_style(new_btnm, GUI_BTNM_STYLE_BTN_INA, th->btnm.btn.ina);
        } else {
            obj_set_style(new_btnm, &style_pretty);
        }
    }
    /*Copy an existing object*/
    else {
        btnm_ext_t * copy_ext = obj_get_ext_attr(copy);
        memcpy(ext->styles_btn, copy_ext->styles_btn, sizeof(ext->styles_btn));
        ext->action = copy_ext->action;
        ext->toggle = copy_ext->toggle;
        ext->btn_id_tgl = copy_ext->btn_id_tgl;
        btnm_set_map(new_btnm, btnm_get_map(copy));
    }
    
    return new_btnm;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new map. Buttons will be created/deleted according to the map.
 * @param btnm pointer to a button matrix object
 * @param map pointer a string array. The last string has to be: "".
 *            Use "\n" to begin a new line.
 *            The first byte can be a control data:
 *             - bit 7: always 1
 *             - bit 6: always 0
 *             - bit 5: inactive (disabled) (\24x)
 *             - bit 4: no repeat (on long press) (\22x)
 *             - bit 3: hidden (\21x)
 *             - bit 2..0: button relative width
 *             Example (practically use octal numbers): "\224abc": "abc" text with 4 width and no long press
 */
void btnm_set_map(obj_t * btnm, const char ** map)
{
	if(map == NULL) return;

	btnm_ext_t * ext = obj_get_ext_attr(btnm);
	ext->map_p = map;

	/*Analyze the map and create the required number of buttons*/
	create_buttons(btnm, map);

	/*Set size and positions of the buttons*/
	style_t * style_bg = btnm_get_style(btnm, GUI_BTNM_STYLE_BG);
	coord_t max_w = obj_get_width(btnm) - 2 * style_bg->body.padding.hor;
	coord_t max_h = obj_get_height(btnm) - 2 * style_bg->body.padding.ver;
	coord_t act_y = style_bg->body.padding.ver;

	/*Count the lines to calculate button height*/
	uint8_t line_cnt = 1;
	uint8_t li;
	for(li = 0; strlen(map[li]) != 0; li++) {
			if(strcmp(map[li], "\n") == 0) line_cnt ++;
	}

	coord_t btn_h = max_h - ((line_cnt - 1) * style_bg->body.padding.inner);
	btn_h = btn_h / line_cnt;
	btn_h --;                              /*-1 because e.g. height = 100 means 101 pixels (0..100)*/

	/* Count the units and the buttons in a line
	 * (A button can be 1,2,3... unit wide)*/
	uint16_t unit_cnt;      /*Number of units in a row*/
    uint16_t unit_act_cnt;  /*Number of units currently put in a row*/
	uint16_t btn_cnt;		/*Number of buttons in a row*/
	uint16_t i_tot = 0;		/*Act. index in the str map*/
	uint16_t btn_i = 0;		/*Act. index of button areas*/
	const char  ** map_p_tmp = map;

	/*Count the units and the buttons in a line*/
	while(1) {
		unit_cnt = 0;
		btn_cnt = 0;
		/*Count the buttons in a line*/
		while(strcmp(map_p_tmp[btn_cnt], "\n") != 0 &&
			  strlen(map_p_tmp[btn_cnt]) != 0) { /*Check a line*/
			unit_cnt += get_button_width(map_p_tmp[btn_cnt]);
			btn_cnt ++;
		}

		/*Only deal with the non empty lines*/
		if(btn_cnt != 0) {
			/*Calculate the width of all units*/
			coord_t all_unit_w = max_w - ((btn_cnt-1) * style_bg->body.padding.inner);

			/*Set the button size and positions and set the texts*/
			uint16_t i;
			coord_t act_x = style_bg->body.padding.hor;
			coord_t act_unit_w;
			unit_act_cnt = 0;
			for(i = 0; i < btn_cnt; i++) {
				/* one_unit_w = all_unit_w / unit_cnt
				 * act_unit_w = one_unit_w * button_width
				 * do this two operations but the multiply first to divide a greater number */
				act_unit_w = (all_unit_w * get_button_width(map_p_tmp[i])) / unit_cnt;
				act_unit_w --;                              /*-1 because e.g. width = 100 means 101 pixels (0..100)*/

				/*Always recalculate act_x because of rounding errors */
				act_x = (unit_act_cnt * all_unit_w) / unit_cnt + i * style_bg->body.padding.inner + style_bg->body.padding.hor;

				/* Set the button's area.
				 * If inner padding is zero then use the prev. button x2 as x1 to avoid rounding errors*/
				if(style_bg->body.padding.inner == 0 && act_x != style_bg->body.padding.hor) {
                    area_set(&ext->button_areas[btn_i],  ext->button_areas[btn_i - 1].x2, act_y,
	                                                        act_x + act_unit_w, act_y + btn_h);
				} else {
                    area_set(&ext->button_areas[btn_i],  act_x, act_y,
                                                            act_x + act_unit_w, act_y + btn_h);
				}

				unit_act_cnt += get_button_width(map_p_tmp[i]);

				i_tot ++;
				btn_i ++;
			}
		}
		act_y += btn_h + style_bg->body.padding.inner;

		/*If no vertical padding then make sure the last row is at the bottom of 'btnm'*/
		if(style_bg->body.padding.ver == 0  &&
		   act_y + btn_h * 2 > max_h) {         /*Last row?*/
		    btn_h = max_h - act_y - 1;
		}

		if(strlen(map_p_tmp[btn_cnt]) == 0) break; /*Break on end of map*/
		map_p_tmp = &map_p_tmp[btn_cnt + 1]; /*Set the map to the next line*/
		i_tot ++;	/*Skip the '\n'*/
	}

	obj_invalidate(btnm);
}

/**
 * Set a new callback function for the buttons (It will be called when a button is released)
 * @param btnm: pointer to button matrix object
 * @param cb pointer to a callback function
 */
void btnm_set_action(obj_t * btnm, btnm_action_t action)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);
	ext->action = action;
}

/**
 * Enable or disable button toggling
 * @param btnm pointer to button matrix object
 * @param en true: enable toggling; false: disable toggling
 * @param id index of the currently toggled button (ignored if 'en' == false)
 */
void btnm_set_toggle(obj_t * btnm, bool en, uint16_t id)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);

    ext->toggle = en == false ? 0 : 1;
    if(ext->toggle != 0) {
        if(id >= ext->btn_cnt) id = ext->btn_cnt - 1;
        ext->btn_id_tgl = id;
    } else {
        ext->btn_id_tgl = GUI_BTNM_PR_NONE;
    }

    obj_invalidate(btnm);
}

/**
 * Set a style of a button matrix
 * @param btnm pointer to a button matrix object
 * @param type which style should be set
 * @param style pointer to a style
 */
void btnm_set_style(obj_t *btnm, btnm_style_t type, style_t *style)
{
    btnm_ext_t *ext = obj_get_ext_attr(btnm);

    switch (type) {
        case GUI_BTNM_STYLE_BG:
            obj_set_style(btnm, style);
            break;
        case GUI_BTNM_STYLE_BTN_REL:
            ext->styles_btn[GUI_BTN_STATE_REL] = style;
            obj_invalidate(btnm);
            break;
        case GUI_BTNM_STYLE_BTN_PR:
            ext->styles_btn[GUI_BTN_STATE_PR] = style;
            obj_invalidate(btnm);
            break;
        case GUI_BTNM_STYLE_BTN_TGL_REL:
            ext->styles_btn[GUI_BTN_STATE_TGL_REL] = style;
            obj_invalidate(btnm);
            break;
        case GUI_BTNM_STYLE_BTN_TGL_PR:
            ext->styles_btn[GUI_BTN_STATE_TGL_PR] = style;
            obj_invalidate(btnm);
            break;
        case GUI_BTNM_STYLE_BTN_INA:
            ext->styles_btn[GUI_BTN_STATE_INA] = style;
            obj_invalidate(btnm);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the current map of a button matrix
 * @param btnm pointer to a button matrix object
 * @return the current map
 */
const char ** btnm_get_map(obj_t * btnm)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);
	return ext->map_p;
}

/**
 * Get a the callback function of the buttons on a button matrix
 * @param btnm: pointer to button matrix object
 * @return pointer to the callback function
 */
btnm_action_t btnm_get_action(obj_t * btnm)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);
	return ext->action;
}

/**
 * Get the toggled button
 * @param btnm pointer to button matrix object
 * @return  index of the currently toggled button (0: if unset)
 */
uint16_t btnm_get_toggled(obj_t * btnm)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);

    if(ext->toggle == 0) return 0;
    else return ext->btn_id_tgl;}

/**
 * Get a style of a button matrix
 * @param btnm pointer to a button matrix object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * btnm_get_style(obj_t *btnm, btnm_style_t type)
{
    btnm_ext_t *ext = obj_get_ext_attr(btnm);

    switch (type) {
        case GUI_BTNM_STYLE_BG:          return obj_get_style(btnm);
        case GUI_BTNM_STYLE_BTN_REL:     return ext->styles_btn[GUI_BTN_STATE_REL];
        case GUI_BTNM_STYLE_BTN_PR:      return ext->styles_btn[GUI_BTN_STATE_PR];
        case GUI_BTNM_STYLE_BTN_TGL_REL: return ext->styles_btn[GUI_BTN_STATE_TGL_REL];
        case GUI_BTNM_STYLE_BTN_TGL_PR:  return ext->styles_btn[GUI_BTN_STATE_TGL_PR];
        case GUI_BTNM_STYLE_BTN_INA:     return ext->styles_btn[GUI_BTN_STATE_INA];
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the button matrixs
 * @param btnm pointer to a button matrix object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool btnm_design(obj_t * btnm, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
        return ancestor_design_f(btnm, mask, mode);
    	/*Return false if the object is not covers the mask_p area*/
    }
    /*Draw the object*/
    else if (mode == GUI_DESIGN_DRAW_MAIN) {
    	ancestor_design_f(btnm, mask, mode);

    	btnm_ext_t * ext = obj_get_ext_attr(btnm);
        style_t * bg_style = obj_get_style(btnm);
        style_t * btn_style;

    	area_t area_btnm;
        obj_get_coords(btnm, &area_btnm);

    	area_t area_tmp;
    	coord_t btn_w;
    	coord_t btn_h;
    	bool border_mod = false;

    	uint16_t btn_i = 0;
    	uint16_t txt_i = 0;
    	for(btn_i = 0; btn_i < ext->btn_cnt; btn_i ++, txt_i ++) {
            /*Search the next valid text in the map*/
            while(strcmp(ext->map_p[txt_i], "\n") == 0) txt_i ++;

            /*Skip hidden buttons*/
            if(button_is_hidden(ext->map_p[txt_i])) continue;

			area_copy(&area_tmp, &ext->button_areas[btn_i]);
			area_tmp.x1 += area_btnm.x1;
			area_tmp.y1 += area_btnm.y1;
			area_tmp.x2 += area_btnm.x1;
			area_tmp.y2 += area_btnm.y1;

			btn_w = area_get_width(&area_tmp);
			btn_h = area_get_height(&area_tmp);

			/*Load the style*/
			if(button_is_inactive(ext->map_p[txt_i])) btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_INA);
			else if(btn_i != ext->btn_id_pr && btn_i != ext->btn_id_tgl) btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_REL);
			else if(btn_i == ext->btn_id_pr && btn_i != ext->btn_id_tgl) btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_PR);
            else if(btn_i != ext->btn_id_pr && btn_i == ext->btn_id_tgl) btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_TGL_REL);
            else if(btn_i == ext->btn_id_pr && btn_i == ext->btn_id_tgl) btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_TGL_PR);
            else btn_style = btnm_get_style(btnm, GUI_BTNM_STYLE_BTN_REL);    /*Not possible option, just to be sure*/

			/*On the right buttons clear the border if only right borders are drawn*/
			if(ext->map_p[txt_i + 1][0] == '\0' || ext->map_p[txt_i + 1][0] == '\n') {
			    if(btn_style->body.border.part == GUI_BORDER_RIGHT) {
			        btn_style->body.border.part  = GUI_BORDER_NONE;
			        border_mod = true;
			    }
			}

			draw_rect(&area_tmp, mask, btn_style);

			if(border_mod) {
			    border_mod = false;
                btn_style->body.border.part = GUI_BORDER_RIGHT;
			}


			/*Calculate the size of the text*/
			const font_t * font = btn_style->text.font;
			point_t txt_size;
			txt_get_size(&txt_size, ext->map_p[txt_i], font,
			             btn_style->text.letter_space, btn_style->text.line_space,
					     area_get_width(&area_btnm), GUI_TXT_FLAG_NONE);

			area_tmp.x1 += (btn_w - txt_size.x) / 2;
			area_tmp.y1 += (btn_h - txt_size.y) / 2;
			area_tmp.x2 = area_tmp.x1 + txt_size.x;
			area_tmp.y2 = area_tmp.y1 + txt_size.y;


			if(btn_style->glass) btn_style = bg_style;
			draw_label(&area_tmp, mask, btn_style, ext->map_p[txt_i], GUI_TXT_FLAG_NONE, NULL);
    	}
    }
    return true;
}

/**
 * Signal function of the button matrix
 * @param btnm pointer to a button matrix object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t btnm_signal(obj_t * btnm, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(btnm, sign, param);
    if(res != GUI_RES_OK) return res;

    btnm_ext_t * ext = obj_get_ext_attr(btnm);
    area_t btnm_area;
    area_t btn_area;
    point_t p;
    if(sign == GUI_SIGNAL_CLEANUP) {
        RTE_BRel(MEM_RTE,ext->button_areas);
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG || sign == GUI_SIGNAL_CORD_CHG) {
        btnm_set_map(btnm, ext->map_p);
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        uint16_t btn_pr;
        /*Search the pressed area*/
        indev_get_point(param, &p);
        btn_pr = get_button_from_point(btnm, &p);
        /*Invalidate to old and the new areas*/;
        obj_get_coords(btnm, &btnm_area);
        if(btn_pr != ext->btn_id_pr) {
            indev_reset_lpr(param);
            if(ext->btn_id_pr != GUI_BTNM_PR_NONE) {
                area_copy(&btn_area, &ext->button_areas[ext->btn_id_pr]);
                btn_area.x1 += btnm_area.x1;
                btn_area.y1 += btnm_area.y1;
                btn_area.x2 += btnm_area.x1;
                btn_area.y2 += btnm_area.y1;
                inv_area(&btn_area,true);
            }
            if(btn_pr != GUI_BTNM_PR_NONE) {
                area_copy(&btn_area, &ext->button_areas[btn_pr]);
                btn_area.x1 += btnm_area.x1;
                btn_area.y1 += btnm_area.y1;
                btn_area.x2 += btnm_area.x1;
                btn_area.y2 += btnm_area.y1;
                inv_area(&btn_area,true);
            }
        }

        ext->btn_id_pr = btn_pr;
    }

    else if(sign == GUI_SIGNAL_LONG_PRESS_REP) {
        if(ext->action && ext->btn_id_pr != GUI_BTNM_PR_NONE) {
            uint16_t txt_i = get_button_text(btnm, ext->btn_id_pr);
            if(txt_i != GUI_BTNM_PR_NONE) {
                if(button_is_repeat_disabled(ext->map_p[txt_i]) == false &&
                   button_is_inactive(ext->map_p[txt_i]) == false) {
                    ext->action(btnm, cut_ctrl_byte(ext->map_p[txt_i]));
                }
            }
        }
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        if(ext->btn_id_pr != GUI_BTNM_PR_NONE) {
            if(ext->action) {
                uint16_t txt_i = get_button_text(btnm, ext->btn_id_pr);
                if(txt_i != GUI_BTNM_PR_NONE && button_is_inactive(ext->map_p[txt_i]) == false) {
                    ext->action(btnm, cut_ctrl_byte(ext->map_p[txt_i]));
                }
            }

            /*Invalidate to old pressed area*/;
            obj_get_coords(btnm, &btnm_area);
            area_copy(&btn_area, &ext->button_areas[ext->btn_id_pr]);
            btn_area.x1 += btnm_area.x1;
            btn_area.y1 += btnm_area.y1;
            btn_area.x2 += btnm_area.x1;
            btn_area.y2 += btnm_area.y1;
            inv_area(&btn_area,true);

            if(ext->toggle != 0) {
                /*Invalidate to old toggled area*/;
                area_copy(&btn_area, &ext->button_areas[ext->btn_id_tgl]);
                btn_area.x1 += btnm_area.x1;
                btn_area.y1 += btnm_area.y1;
                btn_area.x2 += btnm_area.x1;
                btn_area.y2 += btnm_area.y1;
                inv_area(&btn_area,true);

                ext->btn_id_tgl = ext->btn_id_pr;
            }

            ext->btn_id_pr = GUI_BTNM_PR_NONE;
        }
    }
    else if(sign == GUI_SIGNAL_PRESS_LOST || sign == GUI_SIGNAL_DEFOCUS) {
        ext->btn_id_pr = GUI_BTNM_PR_NONE;
        obj_invalidate(btnm);
    }
    else if(sign == GUI_SIGNAL_FOCUS) {
        ext->btn_id_pr = 0;
        obj_invalidate(btnm);
    }
    else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT) {
            if(ext->btn_id_pr  == GUI_BTNM_PR_NONE) ext->btn_id_pr = 0;
            else ext->btn_id_pr++;
            if(ext->btn_id_pr >= ext->btn_cnt - 1) ext->btn_id_pr = ext->btn_cnt - 1;
            obj_invalidate(btnm);
        } else if(c == GUI_GROUP_KEY_LEFT) {
            if(ext->btn_id_pr  == GUI_BTNM_PR_NONE) ext->btn_id_pr = 0;
            if(ext->btn_id_pr > 0) ext->btn_id_pr--;
            obj_invalidate(btnm);
        } else if(c == GUI_GROUP_KEY_DOWN) {
            style_t * style = btnm_get_style(btnm, GUI_BTNM_STYLE_BG);
            /*Find the area below the the current*/
            if(ext->btn_id_pr  == GUI_BTNM_PR_NONE) {
                ext->btn_id_pr = 0;
            } else {
                uint16_t area_below;
                coord_t pr_center = ext->button_areas[ext->btn_id_pr].x1 + (area_get_width(&ext->button_areas[ext->btn_id_pr]) >> 1);

                for(area_below = ext->btn_id_pr; area_below < ext->btn_cnt; area_below ++) {
                    if(ext->button_areas[area_below].y1 >  ext->button_areas[ext->btn_id_pr].y1 &&
                       pr_center >=  ext->button_areas[area_below].x1 &&
                       pr_center <=  ext->button_areas[area_below].x2 + style->body.padding.hor)
                    {
                        break;
                    }
                }

                if(area_below < ext->btn_cnt) ext->btn_id_pr = area_below;
            }
            obj_invalidate(btnm);
        } else if(c == GUI_GROUP_KEY_UP) {
            style_t * style = btnm_get_style(btnm, GUI_BTNM_STYLE_BG);
            /*Find the area below the the current*/
            if(ext->btn_id_pr  == GUI_BTNM_PR_NONE) {
                ext->btn_id_pr = 0;
            } else {
                int16_t area_above;
                coord_t pr_center = ext->button_areas[ext->btn_id_pr].x1 + (area_get_width(&ext->button_areas[ext->btn_id_pr]) >> 1);

                for(area_above = ext->btn_id_pr; area_above >= 0; area_above --) {
                    if(ext->button_areas[area_above].y1 < ext->button_areas[ext->btn_id_pr].y1 &&
                       pr_center >=  ext->button_areas[area_above].x1 - style->body.padding.hor &&
                       pr_center <=  ext->button_areas[area_above].x2)
                    {
                        break;
                    }
                }
                if(area_above >= 0) ext->btn_id_pr = area_above;

            }
            obj_invalidate(btnm);
        }else if(c == GUI_GROUP_KEY_ENTER || c == GUI_GROUP_KEY_ENTER_LONG) {
            if(ext->action != NULL) {
                uint16_t txt_i = get_button_text(btnm, ext->btn_id_pr);
                if(txt_i != GUI_BTNM_PR_NONE) {
                    ext->action(btnm, cut_ctrl_byte(ext->map_p[txt_i]));
                }
            }
        }
        else if(sign == GUI_SIGNAL_GET_TYPE) {
            obj_type_t * buf = param;
            uint8_t i;
            for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
                if(buf->type[i] == NULL) break;
            }
            buf->type[i] = "btnm";
        }
    }

    return res;
}

/**
 * Create the required number of buttons according to a map
 * @param btnm pointer to button matrix object
 * @param map_p pointer to a string array
 */
static void create_buttons(obj_t * btnm, const char ** map)
{
	/*Count the buttons in the map*/
	uint16_t btn_cnt = 0;
	uint16_t i = 0;
	while(strlen(map[i]) != 0) {
			if(strcmp(map[i], "\n") != 0) { /*Do not count line breaks*/
			btn_cnt ++;
		}
		i++;
	}

	btnm_ext_t * ext = obj_get_ext_attr(btnm);

	if(ext->button_areas != NULL) {
		RTE_BRel(MEM_RTE,ext->button_areas);
		ext->button_areas = NULL;
	}

	ext->button_areas = RTE_BGet(MEM_RTE,sizeof(area_t) * btn_cnt);
	ext->btn_cnt = btn_cnt;
}

/**
 * Get the width of a button in units. It comes from the first "letter".
 * @param btn_str The descriptor string of a button. E.g. "apple" or "\004banana"
 * @return the width of the button in units
 */
static uint8_t get_button_width(const char * btn_str)
{
	if((btn_str[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE){
	    return btn_str[0] & GUI_BTNM_WIDTH_MASK;
	}

	return 1;   /*Default width is 1*/
}

static bool button_is_hidden(const char * btn_str)
{
    /*If control byte presents and hidden bit is '1' then the button is hidden*/
    if(((btn_str[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE) &&
        (btn_str[0] & GUI_BTNM_HIDE_MASK)) {
       return true;
    }

    return false;
}

static bool button_is_repeat_disabled(const char * btn_str)
{
    /*If control byte presents and hidden bit is '1' then the button is hidden*/
    if(((btn_str[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE) &&
        (btn_str[0] & GUI_BTNM_REPEAT_DISABLE_MASK)) {
       return true;
    }

    return false;
}

static bool button_is_inactive(const char * btn_str)
{
    /*If control byte presents and hidden bit is '1' then the button is hidden*/
    if(((btn_str[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE) &&
        (btn_str[0] & GUI_BTNM_INACTIVE_MASK)) {
       return true;
    }

    return false;
}


const char * cut_ctrl_byte(const char * btn_str)
{
    /*Cut the control byte if present*/
    if((btn_str[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE) return &btn_str[1];
    else return btn_str;
}

/**
 * Gives the button id of a button under a given point
 * @param btnm pointer to a button matrix object
 * @param p a point with absolute coordinates
 * @return the id of the button or GUI_BTNM_PR_NONE.
 */
static uint16_t get_button_from_point(obj_t * btnm, point_t * p)
{
    area_t btnm_cords;
    area_t btn_area;
    btnm_ext_t * ext = obj_get_ext_attr(btnm);
    uint16_t i;
    obj_get_coords(btnm, &btnm_cords);

    for(i = 0; i < ext->btn_cnt; i++) {
        area_copy(&btn_area, &ext->button_areas[i]);
        btn_area.x1 += btnm_cords.x1;
        btn_area.y1 += btnm_cords.y1;
        btn_area.x2 += btnm_cords.x1;
        btn_area.y2 += btnm_cords.y1;
        if(area_is_point_on(&btn_area, p) != false) {
            break;
        }
    }

    if(i == ext->btn_cnt) i = GUI_BTNM_PR_NONE;

    return i;
}

/**
 * Get the text of a button
 * @param btnm pointer to a button matrix object
 * @param btn_id button id
 * @return text id in ext->map_p or GUI_BTNM_PR_NONE if 'btn_id' was invalid
 */
static uint16_t get_button_text(obj_t * btnm, uint16_t btn_id)
{
    btnm_ext_t * ext = obj_get_ext_attr(btnm);
    if(btn_id > ext->btn_cnt) return GUI_BTNM_PR_NONE;

    uint16_t txt_i = 0;
    uint16_t btn_i = 0;

    /* Search the text of ext->btn_pr the buttons text in the map
     * Skip "\n"-s*/
    while(btn_i != btn_id) {
        btn_i ++;
        txt_i ++;
        if(strcmp(ext->map_p[txt_i], "\n") == 0) txt_i ++;
    }

    if(btn_i == ext->btn_cnt) return  GUI_BTNM_PR_NONE;

    return txt_i;
}


#endif
