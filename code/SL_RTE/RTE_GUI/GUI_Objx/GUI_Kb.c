
/**
 * @file kb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_KB != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t kb_signal(obj_t * kb, signal_t sign, void * param);
static res_t app_kb_action(obj_t * kb, const char * txt);

/**********************
 *  STATIC VARIABLES
 **********************/
static signal_func_t ancestor_signal;

static const char * kb_map_lc[] = {
"\2051#", "\204q", "\204w", "\204e", "\204r", "\204t", "\204y", "\204u", "\204i", "\204o", "\204p", "\207Del", "\n",
"\226ABC", "\203a", "\203s", "\203d", "\203f", "\203g", "\203h", "\203j", "\203k", "\203l", "\207Enter", "\n",
"_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
"\202"SYMBOL_CLOSE, "\202"SYMBOL_LEFT, "\206 ", "\202"SYMBOL_RIGHT, "\202"SYMBOL_OK, ""
};

static const char * kb_map_uc[] = {
"\2051#", "\204Q", "\204W", "\204E", "\204R", "\204T", "\204Y", "\204U", "\204I", "\204O", "\204P", "\207Del", "\n",
"\226abc", "\203A", "\203S", "\203D", "\203F", "\203G", "\203H", "\203J", "\203K", "\203L", "\207Enter", "\n",
"_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
"\202"SYMBOL_CLOSE, "\202"SYMBOL_LEFT, "\206 ", "\202"SYMBOL_RIGHT, "\202"SYMBOL_OK, ""
};

static const char * kb_map_spec[] = {
"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "\202Del", "\n",
"\222abc", "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
"\\", "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
"\202"SYMBOL_CLOSE, "\202"SYMBOL_LEFT, "\206 ", "\202"SYMBOL_RIGHT, "\202"SYMBOL_OK, ""
};

static const char * kb_map_num[] = {
"1", "2", "3", "\202"SYMBOL_CLOSE,"\n",
"4", "5", "6", "\202"SYMBOL_OK, "\n",
"7", "8", "9", "\202Del", "\n",
"+/-", "0", ".", SYMBOL_LEFT, SYMBOL_RIGHT, ""
};
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a keyboard objects
 * @param par pointer to an object, it will be the parent of the new keyboard
 * @param copy pointer to a keyboard object, if not NULL then the new object will be copied from it
 * @return pointer to the created keyboard
 */
obj_t * kb_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor of keyboard*/
    obj_t * new_kb = btnm_create(par, copy);
    RTE_AssertParam(new_kb);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_kb);

    /*Allocate the keyboard type specific extended data*/
    kb_ext_t * ext = obj_allocate_ext_attr(new_kb, sizeof(kb_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */

    ext->ta = NULL;
    ext->mode = GUI_KB_MODE_TEXT;
    ext->cursor_mng = 0;
    ext->hide_action = NULL;
    ext->ok_action = NULL;

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_kb, kb_signal);

    /*Init the new keyboard keyboard*/
    if(copy == NULL) {
        obj_set_size(new_kb, GUI_HOR_RES, GUI_VER_RES / 2);
        obj_align(new_kb, NULL, GUI_ALIGN_IN_BOTTOM_MID, 0, 0);
        btnm_set_action(new_kb, app_kb_action);
        btnm_set_map(new_kb, kb_map_lc);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            kb_set_style(new_kb, GUI_KB_STYLE_BG, th->kb.bg);
            kb_set_style(new_kb, GUI_KB_STYLE_BTN_REL, th->kb.btn.rel);
            kb_set_style(new_kb, GUI_KB_STYLE_BTN_PR, th->kb.btn.pr);
            kb_set_style(new_kb, GUI_KB_STYLE_BTN_TGL_REL, th->kb.btn.tgl_rel);
            kb_set_style(new_kb, GUI_KB_STYLE_BTN_TGL_PR, th->kb.btn.tgl_pr);
            kb_set_style(new_kb, GUI_KB_STYLE_BTN_INA, th->kb.btn.ina);
        } else {
            /*Let the button matrix's styles*/
        }
    }
    /*Copy an existing keyboard*/
    else {
    	kb_ext_t * copy_ext = obj_get_ext_attr(copy);ext->ta = NULL;
        ext->ta = copy_ext->ta;
        ext->mode = copy_ext->mode;
        ext->cursor_mng = copy_ext->cursor_mng;
        ext->hide_action = copy_ext->hide_action;
        ext->ok_action = copy_ext->ok_action;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_kb);
    }

    return new_kb;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @param ta pointer to a Text Area object to write there
 */
void kb_set_ta(obj_t * kb, obj_t * ta)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    cursor_type_t cur_type;

    /*Hide the cursor of the old Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        cur_type = ta_get_cursor_type(ext->ta);
        ta_set_cursor_type(ext->ta,  (cursor_type_t)(cur_type | GUI_CURSOR_HIDDEN));
    }

    ext->ta = ta;

    /*Show the cursor of the new Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        cur_type = ta_get_cursor_type(ext->ta);
        ta_set_cursor_type(ext->ta,  (cursor_type_t)(cur_type & (~GUI_CURSOR_HIDDEN)));
    }
}

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @param mode the mode from 'kb_mode_t'
 */
void kb_set_mode(obj_t * kb, kb_mode_t mode)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    if(ext->mode == mode) return;

    ext->mode = mode;
    if(mode == GUI_KB_MODE_TEXT) btnm_set_map(kb, kb_map_lc);
    else if(mode == GUI_KB_MODE_NUM) btnm_set_map(kb, kb_map_num);
}


/**
 * Automatically hide or show the cursor of Text Area
 * @param kb pointer to a Keyboard object
 * @param en true: show cursor on the current text area, false: hide cursor
 */
void kb_set_cursor_manage(obj_t * kb, bool en)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    if(ext->cursor_mng == en) return;

    ext->cursor_mng = en == false? 0 : 1;

    if(ext->ta) {
        cursor_type_t cur_type;
        cur_type = ta_get_cursor_type(ext->ta);

        if(ext->cursor_mng){
            ta_set_cursor_type(ext->ta,  (cursor_type_t)(cur_type & (~GUI_CURSOR_HIDDEN)));
        }else{
            ta_set_cursor_type(ext->ta,  (cursor_type_t)(cur_type | GUI_CURSOR_HIDDEN));
        }
    }
}

/**
 * Set call back to call when the "Ok" button is pressed
 * @param kb pointer to Keyboard object
 * @param action a callback with 'action_t' type
 */
void kb_set_ok_action(obj_t * kb, action_t action)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    ext->ok_action = action;
}

/**
 * Set call back to call when the "Hide" button is pressed
 * @param kb pointer to Keyboard object
 * @param action a callback with 'action_t' type
 */
void kb_set_hide_action(obj_t * kb, action_t action)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    ext->hide_action = action;
}

/**
 * Set a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be set
 * @param style pointer to a style
 */
void kb_set_style(obj_t *kb, kb_style_t type, style_t *style)
{
    switch (type) {
        case GUI_KB_STYLE_BG:
            btnm_set_style(kb, GUI_BTNM_STYLE_BG, style);
            break;
        case GUI_KB_STYLE_BTN_REL:
            btnm_set_style(kb, GUI_BTNM_STYLE_BTN_REL, style);
            break;
        case GUI_KB_STYLE_BTN_PR:
            btnm_set_style(kb, GUI_BTNM_STYLE_BTN_PR, style);
            break;
        case GUI_KB_STYLE_BTN_TGL_REL:
            btnm_set_style(kb, GUI_BTNM_STYLE_BTN_TGL_REL, style);
            break;
        case GUI_KB_STYLE_BTN_TGL_PR:
            btnm_set_style(kb, GUI_BTNM_STYLE_BTN_TGL_PR, style);
            break;
        case GUI_KB_STYLE_BTN_INA:
            btnm_set_style(kb, GUI_BTNM_STYLE_BTN_INA, style);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @return pointer to the assigned Text Area object
 */
obj_t * kb_get_ta(obj_t * kb)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    return ext->ta;
}

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @return the current mode from 'kb_mode_t'
 */
kb_mode_t kb_get_mode(obj_t * kb)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    return ext->mode;
}


/**
 * Get the current cursor manage mode.
 * @param kb pointer to a Keyboard object
 * @return true: show cursor on the current text area, false: hide cursor
 */
bool kb_get_cursor_manage(obj_t * kb)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    return ext->cursor_mng == 0 ? false : true;
}

/**
 * Get the callback to call when the "Ok" button is pressed
 * @param kb pointer to Keyboard object
 * @return the ok callback
 */
action_t kb_get_ok_action(obj_t * kb)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    return ext->ok_action;
}

/**
 * Get the callback to call when the "Hide" button is pressed
 * @param kb pointer to Keyboard object
 * @return the close callback
 */
action_t kb_get_hide_action(obj_t * kb)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);
    return ext->hide_action;
}

/**
 * Get a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * kb_get_style(obj_t *kb, kb_style_t type)
{
    switch (type) {
        case GUI_KB_STYLE_BG:          return btnm_get_style(kb, GUI_BTNM_STYLE_BG);
        case GUI_KB_STYLE_BTN_REL:     return btnm_get_style(kb, GUI_BTNM_STYLE_BTN_REL);
        case GUI_KB_STYLE_BTN_PR:      return btnm_get_style(kb, GUI_BTNM_STYLE_BTN_PR);
        case GUI_KB_STYLE_BTN_TGL_REL: return btnm_get_style(kb, GUI_BTNM_STYLE_BTN_TGL_REL);
        case GUI_KB_STYLE_BTN_TGL_PR:  return btnm_get_style(kb, GUI_BTNM_STYLE_BTN_TGL_PR);
        case GUI_KB_STYLE_BTN_INA:     return btnm_get_style(kb, GUI_BTNM_STYLE_BTN_INA);
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the keyboard
 * @param kb pointer to a keyboard object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t kb_signal(obj_t * kb, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(kb, sign, param);
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
        buf->type[i] = "kb";
    }

    return res;
}

/**
 * Called when a button of 'kb_btnm' is released
 * @param btnm pointer to 'kb_btnm'
 * @param i the index of the released button from the current btnm map
 * @return GUI_ACTION_RES_INV if the btnm is deleted else GUI_ACTION_RES_OK
 */
static res_t app_kb_action(obj_t * kb, const char * txt)
{
    kb_ext_t * ext = obj_get_ext_attr(kb);

    /*Do the corresponding action according to the text of the button*/
    if(strcmp(txt, "abc") == 0) {
        btnm_set_map(kb, kb_map_lc);
        return GUI_RES_OK;
    }
    else if(strcmp(txt, "ABC") == 0) {
        btnm_set_map(kb, kb_map_uc);
        return GUI_RES_OK;
    }
    else if(strcmp(txt, "1#") == 0) {
        btnm_set_map(kb, kb_map_spec);
        return GUI_RES_OK;
    }
    else if(strcmp(txt, SYMBOL_CLOSE) == 0) {
        if(ext->hide_action) ext->hide_action(kb);
        else {
            kb_set_ta(kb, NULL);         /*De-assign the text area  to hide it cursor if needed*/
            obj_del(kb);
        }
        return GUI_RES_INV;
    } else if(strcmp(txt, SYMBOL_OK) == 0) {
        if(ext->ok_action) ext->ok_action(kb);
        else {
            kb_set_ta(kb, NULL);         /*De-assign the text area to hide it cursor if needed*/
            obj_del(kb);
        }
        return GUI_RES_INV;
    }

    if(ext->ta == NULL) return GUI_RES_OK;

    if(strcmp(txt, "Enter") == 0)ta_add_char(ext->ta, '\n');
    else if(strcmp(txt, SYMBOL_LEFT) == 0) ta_cursor_left(ext->ta);
    else if(strcmp(txt, SYMBOL_RIGHT) == 0) ta_cursor_right(ext->ta);
    else if(strcmp(txt, "Del") == 0)  ta_del_char(ext->ta);
    else if(strcmp(txt, "+/-") == 0) {
        uint16_t cur = ta_get_cursor_pos(ext->ta);
        const char * ta_txt = ta_get_text(ext->ta);
        if(ta_txt[0] == '-') {
            ta_set_cursor_pos(ext->ta, 1);
            ta_del_char(ext->ta);
            ta_add_char(ext->ta, '+');
            ta_set_cursor_pos(ext->ta, cur);
        } else if(ta_txt[0] == '+') {
            ta_set_cursor_pos(ext->ta, 1);
            ta_del_char(ext->ta);
            ta_add_char(ext->ta, '-');
            ta_set_cursor_pos(ext->ta, cur);
        } else {
            ta_set_cursor_pos(ext->ta, 0);
            ta_add_char(ext->ta, '-');
            ta_set_cursor_pos(ext->ta, cur + 1);
        }
    } else {
        ta_add_text(ext->ta, txt);
    }
    return GUI_RES_OK;
}

#endif
