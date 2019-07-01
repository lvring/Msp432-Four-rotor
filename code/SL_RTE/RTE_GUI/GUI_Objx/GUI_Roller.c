/**
 * @file roller.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_ROLLER != 0

/*********************
 *      DEFINES
 *********************/
#if GUI_USE_ANIMATION
#  ifndef GUI_ROLLER_ANIM_TIME
#    define GUI_ROLLER_ANIM_TIME     200         /*ms*/
#  endif
#else
#  undef  GUI_ROLLER_ANIM_TIME
#  define GUI_ROLLER_ANIM_TIME     0             /*No animation*/
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool roller_design(obj_t * roller, const area_t * mask, design_mode_t mode);
static res_t roller_scrl_signal(obj_t * roller_scrl, signal_t sign, void * param);
static res_t roller_signal(obj_t * roller, signal_t sign, void * param);
static void refr_position(obj_t *roller, bool anim_en);
static void draw_bg(obj_t *roller, const area_t *mask);

/**********************
 *  STATIC VARIABLES
 **********************/
static signal_func_t ancestor_signal;
static signal_func_t ancestor_scrl_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a roller object
 * @param par pointer to an object, it will be the parent of the new roller
 * @param copy pointer to a roller object, if not NULL then the new object will be copied from it
 * @return pointer to the created roller
 */
obj_t * roller_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor of roller*/
	obj_t * new_roller = ddlist_create(par, copy);
    RTE_AssertParam(new_roller);
    if(ancestor_scrl_signal == NULL) ancestor_scrl_signal = obj_get_signal_func(page_get_scrl(new_roller));
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_roller);
    
    /*Allocate the roller type specific extended data*/
    roller_ext_t * ext = obj_allocate_ext_attr(new_roller, sizeof(roller_ext_t));
    RTE_AssertParam(ext);

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_roller, roller_signal);
    obj_set_design_func(new_roller, roller_design);

    /*Init the new roller roller*/
    if(copy == NULL) {
        obj_t * scrl = page_get_scrl(new_roller);
        obj_set_drag(scrl, true);                        /*In ddlist is might be disabled*/
        page_set_rel_action(new_roller, NULL);       /*Roller don't uses it (like ddlist)*/
        page_set_scrl_fit(new_roller, true, false);      /*Height is specified directly*/
        ddlist_open(new_roller, false);
        ddlist_set_anim_time(new_roller, GUI_ROLLER_ANIM_TIME);
        roller_set_visible_row_count(new_roller, 3);
        label_set_align(ext->ddlist.label, GUI_LABEL_ALIGN_CENTER);

        obj_set_signal_func(scrl, roller_scrl_signal);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            roller_set_style(new_roller, GUI_ROLLER_STYLE_BG, th->roller.bg);
            roller_set_style(new_roller, GUI_ROLLER_STYLE_SEL, th->roller.sel);
        } else {
            /*Let the ddlist's style*/
            obj_refresh_style(new_roller);                /*To set scrollable size automatically*/
        }
    }
    /*Copy an existing roller*/
    else {
        obj_t * scrl = page_get_scrl(new_roller);
        ddlist_open(new_roller, false);
        obj_set_signal_func(scrl, roller_scrl_signal);

        obj_refresh_style(new_roller);        /*Refresh the style with new signal function*/
    }
    
    return new_roller;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the selected option
 * @param roller pointer to a roller object
 * @param sel_opt id of the selected option (0 ... number of option - 1);
 * @param anim_en true: set with animation; false set immediately
 */
void roller_set_selected(obj_t *roller, uint16_t sel_opt, bool anim_en)
{
#if GUI_USE_ANIMATION == 0
    anim_en = false;
#endif

    if(roller_get_selected(roller) == sel_opt) return;

    ddlist_set_selected(roller, sel_opt);
    refr_position(roller, anim_en);
}

/**
 * Set the height to show the given number of rows (options)
 * @param roller pointer to a roller object
 * @param row_cnt number of desired visible rows
 */
void roller_set_visible_row_count(obj_t *roller, uint8_t row_cnt)
{
    roller_ext_t *ext = obj_get_ext_attr(roller);
    style_t * style_label = obj_get_style(ext->ddlist.label);
    ddlist_set_fix_height(roller, font_get_height(style_label->text.font) * row_cnt + style_label->text.line_space * (row_cnt));

}
/**
 * Set a style of a roller
 * @param roller pointer to a roller object
 * @param type which style should be set
 * @param style pointer to a style
 */
void roller_set_style(obj_t *roller, roller_style_t type, style_t *style)
{
    switch (type) {
        case GUI_ROLLER_STYLE_BG:
            obj_set_style(roller, style);
            break;
        case GUI_ROLLER_STYLE_SEL:
            ddlist_set_style(roller, GUI_DDLIST_STYLE_SEL, style);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the auto width set attribute
 * @param roller pointer to a roller object
 * @return true: auto size enabled; false: manual width settings enabled
 */
bool roller_get_hor_fit(obj_t *roller)
{
    return page_get_scrl_hor_fit(roller);
}

/**
 * Get a style of a roller
 * @param roller pointer to a roller object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
style_t * roller_get_style(obj_t *roller, roller_style_t type)
{
    switch (type) {
        case GUI_ROLLER_STYLE_BG:        return obj_get_style(roller);
        case GUI_ROLLER_STYLE_SEL:  return ddlist_get_style(roller, GUI_DDLIST_STYLE_SEL);
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * Handle the drawing related tasks of the rollers
 * @param roller pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool roller_design(obj_t * roller, const area_t * mask, design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == GUI_DESIGN_COVER_CHK) {
    	return false;
    }
    /*Draw the object*/
    else if(mode == GUI_DESIGN_DRAW_MAIN) {
        draw_bg(roller, mask);

        style_t *style = roller_get_style(roller, GUI_ROLLER_STYLE_BG);
        const font_t * font = style->text.font;
        roller_ext_t * ext = obj_get_ext_attr(roller);
        coord_t font_h = font_get_height(font);
        area_t rect_area;
        rect_area.y1 = roller->coords.y1 + obj_get_height(roller) / 2 - font_h / 2 - style->text.line_space / 2;
        rect_area.y2 = rect_area.y1 + font_h + style->text.line_space;
        rect_area.x1 = roller->coords.x1;
        rect_area.x2 = roller->coords.x2;

        draw_rect(&rect_area, mask, ext->ddlist.sel_style);
    }
    /*Post draw when the children are drawn*/
    else if(mode == GUI_DESIGN_DRAW_POST) {
        style_t *style = roller_get_style(roller, GUI_ROLLER_STYLE_BG);
        roller_ext_t * ext = obj_get_ext_attr(roller);
        const font_t * font = style->text.font;
        coord_t font_h = font_get_height(font);

        /*Redraw the text on the selected area with a different color*/
        area_t rect_area;
        rect_area.y1 = roller->coords.y1 + obj_get_height(roller) / 2 - font_h / 2 - style->text.line_space / 2;
        rect_area.y2 = rect_area.y1 + font_h + style->text.line_space;
        rect_area.x1 = roller->coords.x1;
        rect_area.x2 = roller->coords.x2;
        area_t mask_sel;
        bool area_ok;
        area_ok = area_union(&mask_sel, mask, &rect_area);
        if(area_ok) {
            style_t *sel_style = roller_get_style(roller, GUI_ROLLER_STYLE_SEL);
            style_t new_style;
            style_copy(&new_style, style);
            new_style.text.color = sel_style->text.color;
            new_style.text.opa = sel_style->text.opa;
            draw_label(&ext->ddlist.label->coords, &mask_sel, &new_style,
                          label_get_text(ext->ddlist.label), GUI_TXT_FLAG_CENTER, NULL);
        }
    }

    return true;
}

/**
 * Signal function of the roller
 * @param roller pointer to a roller object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t roller_signal(obj_t * roller, signal_t sign, void * param)
{
    res_t res  = GUI_RES_OK;

    /*Don't let the drop down list to handle the control signals. It works differently*/
    if(sign != GUI_SIGNAL_CONTROLL && sign != GUI_SIGNAL_FOCUS && sign != GUI_SIGNAL_DEFOCUS) {
         /* Include the ancient signal function */
        res = ancestor_signal(roller, sign, param);
        if(res != GUI_RES_OK) return res;
    }

    roller_ext_t * ext = obj_get_ext_attr(roller);
    if(sign == GUI_SIGNAL_STYLE_CHG) {
        obj_set_height(page_get_scrl(roller),
                             obj_get_height(ext->ddlist.label) + obj_get_height(roller));
        obj_align(ext->ddlist.label, NULL, GUI_ALIGN_CENTER, 0, 0);
        ddlist_set_selected(roller, ext->ddlist.sel_opt_id);
        refr_position(roller, false);
    } else if(sign == GUI_SIGNAL_CORD_CHG) {

        if(obj_get_width(roller) != area_get_width(param) ||
           obj_get_height(roller) != area_get_height(param)) {

            ddlist_set_fix_height(roller, obj_get_height(roller));
            obj_set_height(page_get_scrl(roller),
                                 obj_get_height(ext->ddlist.label) + obj_get_height(roller));

            obj_align(ext->ddlist.label, NULL, GUI_ALIGN_CENTER, 0, 0);
            ddlist_set_selected(roller, ext->ddlist.sel_opt_id);
            refr_position(roller, false);
        }
    }
    else if(sign == GUI_SIGNAL_FOCUS) {
        ext->ddlist.sel_opt_id_ori = ext->ddlist.sel_opt_id;
    }
    else if(sign == GUI_SIGNAL_DEFOCUS) {
        /*Revert the original state*/
        if(ext->ddlist.sel_opt_id != ext->ddlist.sel_opt_id_ori) {
            ext->ddlist.sel_opt_id = ext->ddlist.sel_opt_id_ori;
            refr_position(roller, true);
        }

    }
    else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT || c == GUI_GROUP_KEY_DOWN) {
            if(ext->ddlist.sel_opt_id +1 < ext->ddlist.option_cnt) {
                roller_set_selected(roller, ext->ddlist.sel_opt_id + 1, true);
            }
        } else if(c == GUI_GROUP_KEY_LEFT || c == GUI_GROUP_KEY_UP) {
            if(ext->ddlist.sel_opt_id > 0) {
                roller_set_selected(roller, ext->ddlist.sel_opt_id - 1, true);
            }
        } else if(c == GUI_GROUP_KEY_ENTER || c == GUI_GROUP_KEY_ENTER_LONG) {
            if(ext->ddlist.action) ext->ddlist.action(roller);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "roller";
    }

    return res;
}

/**
 * Signal function of the scrollable part of the roller.
 * @param roller_scrl ointer to the scrollable part of roller (page)
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t roller_scrl_signal(obj_t * roller_scrl, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_scrl_signal(roller_scrl, sign, param);
    if(res != GUI_RES_OK) return res;

    indev_t * indev = indev_get_act();
    int32_t id = -1;
    obj_t * roller = obj_get_parent(roller_scrl);
    roller_ext_t * ext = obj_get_ext_attr(roller);

    if(ext->ddlist.label == NULL) return GUI_RES_INV;    /*On delete the ddlist signal deletes the label so nothing left to do here*/

    style_t * style_label = obj_get_style(ext->ddlist.label);
    const font_t * font = style_label->text.font;
    coord_t font_h = font_get_height(font);

    if(sign == GUI_SIGNAL_DRAG_END) {
        /*If dragged then align the list to there be an element in the middle*/
        coord_t label_y1 = ext->ddlist.label->coords.y1 - roller->coords.y1;
        coord_t label_unit = font_h + style_label->text.line_space;
        coord_t mid = (roller->coords.y2 - roller->coords.y1) / 2;
        id = (mid - label_y1 + style_label->text.line_space / 2) / label_unit;
        if(id < 0) id = 0;
        if(id >= ext->ddlist.option_cnt) id = ext->ddlist.option_cnt - 1;
        ext->ddlist.sel_opt_id = id;
        if(ext->ddlist.action) ext->ddlist.action(roller);
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        /*If picked an option by clicking then set it*/
        if(!indev_is_dragging(indev)) {
            point_t p;
            indev_get_point(indev, &p);
            p.y = p.y - ext->ddlist.label->coords.y1;
            id = p.y / (font_h + style_label->text.line_space);
            if(id < 0) id = 0;
            if(id >= ext->ddlist.option_cnt) id = ext->ddlist.option_cnt - 1;
            ext->ddlist.sel_opt_id = id;
            if(ext->ddlist.action) ext->ddlist.action(roller);
        }
    }

    /*Position the scrollable according to the new selected option*/
    if(id != -1) {
        refr_position(roller, true);
    }

    return res;
}

/**
 * Draw a rectangle which has gradient on its top and bottom
 * @param roller pointer to a roller object
 * @param mask pointer to the current mask (from the design function)
 */
static void draw_bg(obj_t *roller, const area_t *mask)
{
    style_t *style = roller_get_style(roller, GUI_ROLLER_STYLE_BG);
    area_t half_mask;
    area_t half_roller;
    coord_t h = obj_get_height(roller);
    bool union_ok;
    area_copy(&half_roller, &roller->coords);

    half_roller.x1 -= roller->ext_size; /*Add ext size too (e.g. because of shadow draw) */
    half_roller.x2 += roller->ext_size;
    half_roller.y1 -= roller->ext_size;
    half_roller.y2 = roller->coords.y1 + h / 2;

    union_ok = area_union(&half_mask, &half_roller, mask);

    half_roller.x1 += roller->ext_size; /*Revert ext. size adding*/
    half_roller.x2 -= roller->ext_size;
    half_roller.y1 += roller->ext_size;
    half_roller.y2 += style->body.radius;

    if(union_ok) {
        draw_rect(&half_roller, &half_mask, style);
    }

    half_roller.x1 -= roller->ext_size; /*Add ext size too (e.g. because of shadow draw) */
    half_roller.x2 += roller->ext_size;
    half_roller.y2 = roller->coords.y2 + roller->ext_size;
    half_roller.y1 = roller->coords.y1 + h / 2;
    if((h & 0x1) == 0) half_roller.y1++;    /*With even height the pixels in the middle would be drawn twice*/

    union_ok = area_union(&half_mask, &half_roller, mask);

    half_roller.x1 += roller->ext_size; /*Revert ext. size adding*/
    half_roller.x2 -= roller->ext_size;
    half_roller.y2 -= roller->ext_size;
    half_roller.y1 -= style->body.radius;

    if(union_ok){
        color_t main_tmp = style->body.main_color;
        color_t grad_tmp = style->body.grad_color;

        style->body.main_color = grad_tmp;
        style->body.grad_color = main_tmp;
        draw_rect(&half_roller, &half_mask, style);
        style->body.main_color = main_tmp;
        style->body.grad_color = grad_tmp;
    }

}

/**
 * Refresh the position of the roller. It uses the id stored in: ext->ddlist.selected_option_id
 * @param roller pointer to a roller object
 * @param anim_en true: refresh with animation; false: without animation
 */
static void refr_position(obj_t *roller, bool anim_en)
{
#if GUI_USE_ANIMATION == 0
    anim_en = false;
#endif
    obj_t *roller_scrl = page_get_scrl(roller);
    roller_ext_t * ext = obj_get_ext_attr(roller);
    style_t * style_label = obj_get_style(ext->ddlist.label);
    const font_t * font = style_label->text.font;
    coord_t font_h = font_get_height(font);
    coord_t h = obj_get_height(roller);
    int32_t id = ext->ddlist.sel_opt_id;
    coord_t line_y1 = id * (font_h + style_label->text.line_space) + ext->ddlist.label->coords.y1 - roller_scrl->coords.y1;
    coord_t new_y = - line_y1 + (h - font_h) / 2;

    if(ext->ddlist.anim_time == 0 || anim_en == false) {
        obj_set_y(roller_scrl, new_y);
    } else {
#if GUI_USE_ANIMATION
        anim_t a;
        a.var = roller_scrl;
        a.start = obj_get_y(roller_scrl);
        a.end = new_y;
        a.fp = (anim_fp_t)obj_set_y;
        a.path = anim_path_linear;
        a.end_cb = NULL;
        a.act_time = 0;
        a.time = ext->ddlist.anim_time;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        anim_create(&a);
#endif
    }
}

#endif
