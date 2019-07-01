/**
 * @file mbox.c
 * 
 */


/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_MBOX != 0

/*********************
 *      DEFINES
 *********************/

#if GUI_USE_ANIMATION
#  ifndef GUI_MBOX_CLOSE_ANIM_TIME
#    define GUI_MBOX_CLOSE_ANIM_TIME  200 /*List close animation time)  */
#  endif
#else
#  undef  GUI_MBOX_CLOSE_ANIM_TIME
#  define GUI_MBOX_CLOSE_ANIM_TIME	0	/*No animations*/
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t mbox_signal(obj_t * mbox, signal_t sign, void * param);
static void mbox_realign(obj_t *mbox);
static res_t mbox_close_action(obj_t *btn, const char *txt);

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
 * Create a message box objects
 * @param par pointer to an object, it will be the parent of the new message box
 * @param copy pointer to a message box object, if not NULL then the new object will be copied from it
 * @return pointer to the created message box
 */
obj_t * mbox_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor message box*/
	obj_t * new_mbox = cont_create(par, copy);
    RTE_AssertParam(new_mbox);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_mbox);
    
    /*Allocate the message box type specific extended data*/
    mbox_ext_t * ext = obj_allocate_ext_attr(new_mbox, sizeof(mbox_ext_t));
    RTE_AssertParam(ext);
    ext->text = NULL;
    ext->btnm = NULL;
    ext->anim_time = GUI_MBOX_CLOSE_ANIM_TIME;

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_mbox, mbox_signal);

    /*Init the new message box message box*/
    if(copy == NULL) {
    	ext->text = label_create(new_mbox, NULL);
    	label_set_align(ext->text, GUI_LABEL_ALIGN_CENTER);
    	label_set_long_mode(ext->text, GUI_LABEL_LONG_BREAK);
    	label_set_text(ext->text, "Message");

        cont_set_layout(new_mbox, GUI_LAYOUT_COL_M);
        cont_set_fit(new_mbox, false, true);
        obj_set_width(new_mbox, GUI_HOR_RES / 2);
        obj_align(new_mbox, NULL, GUI_ALIGN_CENTER, 0, 0);

        /*Set the default styles*/
         theme_t *th = theme_get_current();
         if(th) {
             mbox_set_style(new_mbox, GUI_MBOX_STYLE_BG, th->mbox.bg);
         } else {
             mbox_set_style(new_mbox, GUI_MBOX_STYLE_BG, &style_pretty);
         }

    }
    /*Copy an existing message box*/
    else {
        mbox_ext_t * copy_ext = obj_get_ext_attr(copy);

        ext->text = label_create(new_mbox, copy_ext->text);

        /*Copy the buttons and the label on them*/
        if(copy_ext->btnm) ext->btnm = btnm_create(new_mbox, copy_ext->btnm);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_mbox);
    }
    
    return new_mbox;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add button to the message box
 * @param mbox pointer to message box object
 * @param btn_map button descriptor (button matrix map).
 *                E.g.  a const char *txt[] = {"ok", "close", ""} (Can not be local variable)
 * @param action a function which will be called when a button is released
 */
void mbox_add_btns(obj_t * mbox, const char **btn_map, btnm_action_t action)
{
    mbox_ext_t * ext = obj_get_ext_attr(mbox);

    /*Create a button matrix if not exists yet*/
    if(ext->btnm == NULL) {
        ext->btnm = btnm_create(mbox, NULL);

        /*Set the default styles*/
         theme_t *th = theme_get_current();
         if(th) {
             mbox_set_style(mbox, GUI_MBOX_STYLE_BTN_BG, th->mbox.btn.bg);
             mbox_set_style(mbox, GUI_MBOX_STYLE_BTN_REL, th->mbox.btn.rel);
             mbox_set_style(mbox, GUI_MBOX_STYLE_BTN_PR, th->mbox.btn.pr);
         } else {
             btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BG, &style_transp_fit);
         }
    }

    btnm_set_map(ext->btnm, btn_map);
    if(action == NULL) btnm_set_action(ext->btnm, mbox_close_action); /*Set a default action anyway*/
    else  btnm_set_action(ext->btnm, action);

    mbox_realign(mbox);
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of the message box
 * @param mbox pointer to a message box
 * @param txt a '\0' terminated character string which will be the message box text
 */
void mbox_set_text(obj_t * mbox, const char * txt)
{
    mbox_ext_t * ext = obj_get_ext_attr(mbox);
    label_set_text(ext->text, txt);

    mbox_realign(mbox);
}


/**
 * Stop the action to call when button is released
 * @param mbox pointer to a message box object
 * @param pointer to an 'btnm_action_t' action
 */
void mbox_set_action(obj_t * mbox, btnm_action_t action)
{
    mbox_ext_t *ext = obj_get_ext_attr(mbox);
    btnm_set_action(ext->btnm, action);
}


/**
 * Set animation duration
 * @param mbox pointer to a message box object
 * @param anim_time animation length in  milliseconds (0: no animation)
 */
void mbox_set_anim_time(obj_t * mbox, uint16_t anim_time)
{
    mbox_ext_t * ext = obj_get_ext_attr(mbox);
#if GUI_USE_ANIMATION == 0
    anim_time = 0;
#endif

    ext->anim_time = anim_time;
}

/**
 * Automatically delete the message box after a given time
 * @param mbox pointer to a message box object
 * @param delay a time (in milliseconds) to wait before delete the message box
 */
void mbox_start_auto_close(obj_t * mbox, uint16_t delay)
{
#if GUI_USE_ANIMATION
    mbox_ext_t * ext = obj_get_ext_attr(mbox);

    if(ext->anim_time != 0) {
        /*Add shrinking animations*/
        obj_animate(mbox, (anim_builtin_t)(GUI_ANIM_GROW_H| GUI_ANIM_OUT), ext->anim_time, delay, NULL);
        obj_animate(mbox, (anim_builtin_t)(GUI_ANIM_GROW_V| GUI_ANIM_OUT), ext->anim_time, delay, (void (*)(obj_t*))obj_del);

        /*Disable fit to let shrinking work*/
        cont_set_fit(mbox, false, false);
    } else {
        obj_animate(mbox, GUI_ANIM_NONE, ext->anim_time, delay, (void (*)(obj_t*))obj_del);
    }
#else
    obj_del(mbox);
#endif
}

/**
 * Stop the auto. closing of message box
 * @param mbox pointer to a message box object
 */
void mbox_stop_auto_close(obj_t * mbox)
{
#if GUI_USE_ANIMATION
    anim_del(mbox, NULL);
#endif
}

/**
 * Set a style of a message box
 * @param mbox pointer to a message box object
 * @param type which style should be set
 * @param style pointer to a style
 */
void mbox_set_style(obj_t *mbox, mbox_style_t type, style_t *style)
{
    mbox_ext_t *ext = obj_get_ext_attr(mbox);

    switch (type) {
        case GUI_MBOX_STYLE_BG:
            obj_set_style(mbox, style);
            break;
        case GUI_MBOX_STYLE_BTN_BG:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BG, style);
            break;
        case GUI_MBOX_STYLE_BTN_REL:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BTN_REL, style);
            break;
        case GUI_MBOX_STYLE_BTN_PR:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BTN_PR, style);
            break;
        case GUI_MBOX_STYLE_BTN_TGL_REL:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BTN_TGL_REL, style);
            break;
        case GUI_MBOX_STYLE_BTN_TGL_PR:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BTN_TGL_PR, style);
            break;
        case GUI_MBOX_STYLE_BTN_INA:
            btnm_set_style(ext->btnm, GUI_BTNM_STYLE_BTN_INA, style);
            break;
    }

}


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of the message box
 * @param mbox pointer to a message box object
 * @return pointer to the text of the message box
 */
const char * mbox_get_text(obj_t * mbox)
{
	mbox_ext_t * ext = obj_get_ext_attr(mbox);

	return label_get_text(ext->text);
}

/**
 * Get the message box object from one of its button.
 * It is useful in the button release actions where only the button is known
 * @param btn pointer to a button of a message box
 * @return pointer to the button's message box
 */
obj_t * mbox_get_from_btn(obj_t * btn)
{
	obj_t * mbox = obj_get_parent(btn);

	return mbox;
}

/**
 * Get the animation duration (close animation time)
 * @param mbox pointer to a message box object
 * @return animation length in  milliseconds (0: no animation)
 */
uint16_t mbox_get_anim_time(obj_t * mbox )
{
    mbox_ext_t * ext = obj_get_ext_attr(mbox);
    return ext->anim_time;
}

/**
 * Get a style of a message box
 * @param mbox pointer to a message box object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * mbox_get_style(obj_t *mbox, mbox_style_t type)
{
    mbox_ext_t *ext = obj_get_ext_attr(mbox);

    switch (type) {
        case GUI_MBOX_STYLE_BG:          return obj_get_style(mbox);
        case GUI_MBOX_STYLE_BTN_BG:      return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BG);
        case GUI_MBOX_STYLE_BTN_REL:     return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BTN_REL);
        case GUI_MBOX_STYLE_BTN_PR:      return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BTN_PR);
        case GUI_MBOX_STYLE_BTN_TGL_REL: return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BTN_TGL_REL);
        case GUI_MBOX_STYLE_BTN_TGL_PR:  return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BTN_TGL_PR);
        case GUI_MBOX_STYLE_BTN_INA:     return btnm_get_style(ext->btnm, GUI_BTNM_STYLE_BTN_INA);
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the message box
 * @param mbox pointer to a message box object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t mbox_signal(obj_t * mbox, signal_t sign, void * param)
{
    res_t res;

    /*Translate GUI_GROUP_KEY_UP/DOWN to GUI_GROUP_KEY_LEFT/RIGHT */
    char c_trans = 0;
    if(sign == GUI_SIGNAL_CONTROLL) {
        c_trans = *((char*)param);
        if(c_trans == GUI_GROUP_KEY_DOWN) c_trans = GUI_GROUP_KEY_LEFT;
        if(c_trans == GUI_GROUP_KEY_UP) c_trans = GUI_GROUP_KEY_RIGHT;

        param = &c_trans;
    }

    /* Include the ancient signal function */
    res = ancestor_signal(mbox, sign, param);
    if(res != GUI_RES_OK) return res;

    mbox_ext_t * ext = obj_get_ext_attr(mbox);
    if(sign == GUI_SIGNAL_CORD_CHG) {
        if(obj_get_width(mbox) != area_get_width(param)) {
            mbox_realign(mbox);
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
        mbox_realign(mbox);

    }
    else if(sign == GUI_SIGNAL_FOCUS || sign == GUI_SIGNAL_DEFOCUS || sign == GUI_SIGNAL_CONTROLL) {
        if(ext->btnm) {
            ext->btnm->signal_func(ext->btnm, sign, param);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "mbox";
    }

    return res;
}

/**
 * Resize the button holder to fit
 * @param mbox pointer to message box object
 */
static void mbox_realign(obj_t *mbox)
{
    mbox_ext_t * ext = obj_get_ext_attr(mbox);

    style_t *style = mbox_get_style(mbox, GUI_MBOX_STYLE_BG);
    coord_t w = obj_get_width(mbox) - 2 * style->body.padding.hor;

    if(ext->text) {
        obj_set_width(ext->text, w);
    }

    if(ext->btnm) {
        style_t *btn_bg_style = mbox_get_style(mbox, GUI_MBOX_STYLE_BTN_BG);
        style_t *btn_rel_style = mbox_get_style(mbox, GUI_MBOX_STYLE_BTN_REL);
        coord_t font_h = font_get_height(btn_rel_style->text.font);
        obj_set_size(ext->btnm, w, font_h + 2 * btn_rel_style->body.padding.ver + 2 * btn_bg_style->body.padding.ver);
    }
}

static res_t mbox_close_action(obj_t *btn, const char *txt)
{
    obj_t *mbox = mbox_get_from_btn(btn);

    if(txt[0] != '\0') {
        mbox_start_auto_close(mbox, 0);
        return GUI_RES_INV;
    }

    return GUI_RES_OK;
}

#endif
