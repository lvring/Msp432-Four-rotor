/**
 * @file cb.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_CB != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool cb_design(obj_t * cb, const area_t * mask, design_mode_t mode);
static bool bullet_design(obj_t * bullet, const area_t * mask, design_mode_t mode);
static res_t cb_signal(obj_t * cb, signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static design_func_t ancestor_bg_design;
static design_func_t ancestor_bullet_design;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a check box objects
 * @param par pointer to an object, it will be the parent of the new check box
 * @param copy pointer to a check box object, if not NULL then the new object will be copied from it
 * @return pointer to the created check box
 */
obj_t * cb_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor basic object*/
    obj_t * new_cb = btn_create(par, copy);
    RTE_AssertParam(new_cb);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_cb);
    if(ancestor_bg_design == NULL) ancestor_bg_design = obj_get_design_func(new_cb);
    
    cb_ext_t * ext = obj_allocate_ext_attr(new_cb, sizeof(cb_ext_t));
    RTE_AssertParam(ext);
    ext->bullet = NULL;
    ext->label = NULL;

    obj_set_signal_func(new_cb, cb_signal);
    obj_set_design_func(new_cb, cb_design);

    /*Init the new checkbox object*/
    if(copy == NULL) {
        ext->bullet = btn_create(new_cb, NULL);
        if(ancestor_bullet_design == NULL) ancestor_bullet_design = obj_get_design_func(ext->bullet);
        obj_set_click(ext->bullet, false);

        ext->label = label_create(new_cb, NULL);

        cb_set_text(new_cb, "Check box");
        btn_set_layout(new_cb, GUI_LAYOUT_ROW_M);
        btn_set_fit(new_cb, true, true);
        btn_set_toggle(new_cb, true);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            cb_set_style(new_cb, GUI_CB_STYLE_BG, th->cb.bg);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_REL, th->cb.box.rel);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_PR, th->cb.box.pr);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_TGL_REL, th->cb.box.tgl_rel);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_TGL_PR, th->cb.box.tgl_pr);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_INA, th->cb.box.ina);
        } else {
            cb_set_style(new_cb,GUI_CB_STYLE_BG, &style_transp);
            cb_set_style(new_cb, GUI_CB_STYLE_BOX_REL,&style_pretty);
        }
    } else {
    	cb_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->bullet = btn_create(new_cb, copy_ext->bullet);
    	ext->label = label_create(new_cb, copy_ext->label);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_cb);
    }

    obj_set_design_func(ext->bullet, bullet_design);
    
    return new_cb;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of a check box
 * @param cb pointer to a check box
 * @param txt the text of the check box
 */
void cb_set_text(obj_t * cb, const char * txt)
{
	cb_ext_t * ext = obj_get_ext_attr(cb);
	label_set_text(ext->label, txt);
}

/**
 * Set a style of a check box
 * @param cb pointer to check box object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void cb_set_style(obj_t * cb, cb_style_t type, style_t *style)
{
    cb_ext_t * ext = obj_get_ext_attr(cb);

    switch (type) {
        case GUI_CB_STYLE_BG:
            btn_set_style(cb, GUI_BTN_STYLE_REL, style);
            btn_set_style(cb, GUI_BTN_STYLE_PR, style);
            btn_set_style(cb, GUI_BTN_STYLE_TGL_REL, style);
            btn_set_style(cb, GUI_BTN_STYLE_TGL_PR, style);
            btn_set_style(cb, GUI_BTN_STYLE_INA, style);
            break;
        case GUI_CB_STYLE_BOX_REL:
            btn_set_style(ext->bullet, GUI_BTN_STYLE_REL, style);
            break;
        case GUI_CB_STYLE_BOX_PR:
            btn_set_style(ext->bullet, GUI_BTN_STYLE_PR, style);
            break;
        case GUI_CB_STYLE_BOX_TGL_REL:
            btn_set_style(ext->bullet, GUI_BTN_STYLE_TGL_REL, style);
            break;
        case GUI_CB_STYLE_BOX_TGL_PR:
            btn_set_style(ext->bullet, GUI_BTN_STYLE_TGL_PR, style);
            break;
        case GUI_CB_STYLE_BOX_INA:
            btn_set_style(ext->bullet, GUI_BTN_STYLE_INA, style);
            break;
    }
}



/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a check box
 * @param cb pointer to check box object
 * @return pointer to the text of the check box
 */
const char * cb_get_text(obj_t * cb)
{
	cb_ext_t * ext = obj_get_ext_attr(cb);
	return label_get_text(ext->label);
}


/**
 * Get a style of a button
 * @param cb pointer to check box object
 * @param type which style should be get
 * @return style pointer to the style
 *  */
style_t * cb_get_style(obj_t * cb, cb_style_t type)
{
    cb_ext_t * ext = obj_get_ext_attr(cb);

    switch (type) {
        case GUI_CB_STYLE_BOX_REL:     return btn_get_style(ext->bullet, GUI_BTN_STYLE_REL);
        case GUI_CB_STYLE_BOX_PR:      return btn_get_style(ext->bullet, GUI_BTN_STYLE_PR);
        case GUI_CB_STYLE_BOX_TGL_REL: return btn_get_style(ext->bullet, GUI_BTN_STYLE_TGL_REL);
        case GUI_CB_STYLE_BOX_TGL_PR:  return btn_get_style(ext->bullet, GUI_BTN_STYLE_TGL_PR);
        case GUI_CB_STYLE_BOX_INA:     return btn_get_style(ext->bullet, GUI_BTN_STYLE_INA);
        default: break;
    }

    /*To avoid awrning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the check boxes
 * @param cb pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool cb_design(obj_t * cb, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
    	/*Return false if the object is not covers the mask_p area*/
    	return ancestor_bg_design(cb, mask, mode);
    } 
		else if(mode == GUI_DESIGN_DRAW_MAIN || mode == GUI_DESIGN_DRAW_POST) {
        cb_ext_t * cb_ext = obj_get_ext_attr(cb);
        btn_ext_t * bullet_ext = obj_get_ext_attr(cb_ext->bullet);

        /*Be sure the state of the bullet is the same as the parent button*/
        bullet_ext->state = cb_ext->bg_btn.state;

        return ancestor_bg_design(cb, mask, mode);

    } 
		else {
        return ancestor_bg_design(cb, mask, mode);
    }
//    return true;
}

/**
 * Handle the drawing related tasks of the check boxes
 * @param bullet pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool bullet_design(obj_t * bullet, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
        return ancestor_bullet_design(bullet, mask, mode);
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
#if GUI_USE_GROUP
        /* If the check box is the active in a group and
         * the background is not visible (transparent or empty)
         * then activate the style of the bullet*/
        style_t * style_ori = obj_get_style(bullet);
        obj_t * bg = obj_get_parent(bullet);
        style_t * style_page = obj_get_style(bg);
        group_t * g = obj_get_group(bg);
        if(style_page->body.empty != 0 || style_page->body.opa == GUI_OPA_TRANSP) { /*Background is visible?*/
            if(group_get_focused(g) == bg) {
                style_t * style_mod;
                style_mod = group_mod_style(g, style_ori);
                bullet->style_p = style_mod;  /*Temporally change the style to the activated */
            }
        }
#endif
        ancestor_bullet_design(bullet, mask, mode);

#if GUI_USE_GROUP
        bullet->style_p = style_ori;  /*Revert the style*/
#endif
    } else if(mode == GUI_DESIGN_DRAW_POST) {
        ancestor_bullet_design(bullet, mask, mode);
    }

    return true;
}


/**
 * Signal function of the check box
 * @param cb pointer to a check box object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t cb_signal(obj_t * cb, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(cb, sign, param);
    if(res != GUI_RES_OK) return res;

    cb_ext_t * ext = obj_get_ext_attr(cb);

    if(sign == GUI_SIGNAL_STYLE_CHG) {
        style_t * label_style = label_get_style(ext->label);
        obj_set_size(ext->bullet, font_get_height(label_style->text.font), font_get_height(label_style->text.font));
        btn_set_state(ext->bullet, btn_get_state(cb));
    } else if(sign == GUI_SIGNAL_PRESSED ||
        sign == GUI_SIGNAL_RELEASED ||
        sign == GUI_SIGNAL_PRESS_LOST) {
        btn_set_state(ext->bullet, btn_get_state(cb));
    } else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT || c == GUI_GROUP_KEY_DOWN ||
           c == GUI_GROUP_KEY_LEFT || c == GUI_GROUP_KEY_UP ||
           c == GUI_GROUP_KEY_ENTER || c == GUI_GROUP_KEY_ENTER_LONG) {
            btn_set_state(ext->bullet, btn_get_state(cb));
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "cb";
    }

    return res;
}

#endif
