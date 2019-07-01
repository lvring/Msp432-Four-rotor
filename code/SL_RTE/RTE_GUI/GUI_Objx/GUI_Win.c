/**
 * @file win.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_WIN != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t win_signal(obj_t * win, signal_t sign, void * param);
static void win_realign(obj_t * win);

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
 * Create a window objects
 * @param par pointer to an object, it will be the parent of the new window
 * @param copy pointer to a window object, if not NULL then the new object will be copied from it
 * @return pointer to the created window
 */
obj_t * win_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor object*/
    obj_t * new_win = obj_create(par, copy);
    RTE_AssertParam(new_win);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_win);
    
    /*Allocate the object type specific extended data*/
    win_ext_t * ext = obj_allocate_ext_attr(new_win, sizeof(win_ext_t));
    RTE_AssertParam(ext);
    ext->page = NULL;
    ext->header = NULL;
    ext->title = NULL;
    ext->style_header = &style_plain_color;
    ext->style_btn_rel = &style_btn_rel;
    ext->style_btn_pr = &style_btn_pr;
    ext->btn_size = ( GUI_DPI) / 2;

    /*Init the new window object*/
    if(copy == NULL) {
        obj_set_size(new_win, GUI_HOR_RES, GUI_VER_RES);
        obj_set_pos(new_win, 0, 0);
        obj_set_style(new_win, &style_pretty);

        ext->page = page_create(new_win, NULL);
        obj_set_protect(ext->page, GUI_PROTECT_PARENT);
        page_set_sb_mode(ext->page, GUI_SB_MODE_AUTO);

    	/*Create a holder for the header*/
    	ext->header = obj_create(new_win, NULL);
    	/*Move back the header because it is automatically moved to the scrollable */
    	obj_set_protect(ext->header, GUI_PROTECT_PARENT);
    	obj_set_parent(ext->header, new_win);

    	/*Create a title on the header*/
    	ext->title = label_create(ext->header, NULL);
    	label_set_text(ext->title,"My title");

    	/*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            win_set_style(new_win, GUI_WIN_STYLE_BG, th->win.bg);
            win_set_style(new_win, GUI_WIN_STYLE_SB, th->win.sb);
            win_set_style(new_win, GUI_WIN_STYLE_HEADER, th->win.header);
            win_set_style(new_win, GUI_WIN_STYLE_CONTENT_BG, th->win.content.bg);
            win_set_style(new_win, GUI_WIN_STYLE_CONTENT_SCRL, th->win.content.scrl);
            win_set_style(new_win, GUI_WIN_STYLE_BTN_REL, th->win.btn.rel);
            win_set_style(new_win, GUI_WIN_STYLE_BTN_PR, th->win.btn.pr);

        } else {
            win_set_style(new_win, GUI_WIN_STYLE_BG, &style_plain);
            win_set_style(new_win, GUI_WIN_STYLE_CONTENT_BG, &style_transp);
            win_set_style(new_win, GUI_WIN_STYLE_CONTENT_SCRL, &style_transp);
            win_set_style(new_win, GUI_WIN_STYLE_HEADER, &style_plain_color);

        }


        obj_set_signal_func(new_win, win_signal);
        obj_set_size(new_win, GUI_HOR_RES, GUI_VER_RES);
    }
    /*Copy an existing object*/
    else {
    	win_ext_t * copy_ext = obj_get_ext_attr(copy);
    	/*Create the objects*/
    	ext->header = obj_create(new_win, copy_ext->header);
    	ext->title = label_create(ext->header, copy_ext->title);
        ext->page = page_create(new_win, copy_ext->page);
        ext->btn_size = copy_ext->btn_size;

    	/*Copy the control buttons*/
    	obj_t * child;
    	obj_t * cbtn;
    	child = obj_get_child_back(copy_ext->header, NULL);
        child = obj_get_child_back(copy_ext->header, child); /*Sip the title*/
    	while(child != NULL) {
    		cbtn = btn_create(ext->header, child);
    		img_create(cbtn, obj_get_child(child, NULL));
    		child = obj_get_child_back(copy_ext->header, child);
    	}

        obj_set_signal_func(new_win, win_signal);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_win);
    }
    
    win_realign(new_win);

    return new_win;
}


/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add control button to the header of the window
 * @param win pointer to a window object
 * @param img_src an image source ('img_t' variable, path to file or a symbol)
 * @param rel_action a function pointer to call when the button is released
 * @return pointer to the created button object
 */
obj_t * win_add_btn(obj_t * win, const void * img_src, action_t rel_action)
{
    win_ext_t * ext = obj_get_ext_attr(win);

    obj_t *btn = btn_create(ext->header, NULL);
    btn_set_style(btn, GUI_BTN_STYLE_REL, ext->style_btn_rel);
    btn_set_style(btn, GUI_BTN_STYLE_PR, ext->style_btn_pr);
    obj_set_size(btn, ext->btn_size, ext->btn_size);
    btn_set_action(btn, GUI_BTN_ACTION_CLICK, rel_action);

    obj_t * img = img_create(btn, NULL);
    obj_set_click(img, false);
    img_set_src(img, img_src);

    win_realign(win);

    return btn;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * A release action which can be assigned to a window control button to close it
 * @param btn pointer to the released button
 * @return always GUI_ACTION_RES_INV because the button is deleted with the window
 */
res_t win_close_action(obj_t * btn)
{
	obj_t * win = win_get_from_btn(btn);

	obj_del(win);

	return GUI_RES_INV;
}

/**
 * Set the title of a window
 * @param win pointer to a window object
 * @param title string of the new title
 */
void win_set_title(obj_t * win, const char * title)
{
	win_ext_t * ext = obj_get_ext_attr(win);

	label_set_text(ext->title, title);
	win_realign(win);
}

/**
 * Set the control button size of a window
 * @param win pointer to a window object
 * @param size control button size
 */
void win_set_btn_size(obj_t * win, coord_t size)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    if(ext->btn_size == size) return;

    ext->btn_size = size;

    win_realign(win);
}

/**
 * Set the layout of the window
 * @param win pointer to a window object
 * @param layout the layout from 'layout_t'
 */
void win_set_layout(obj_t *win, layout_t layout)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    page_set_scrl_layout(ext->page, layout);
}

/**
 * Set the scroll bar mode of a window
 * @param win pointer to a window object
 * @param sb_mode the new scroll bar mode from  'sb_mode_t'
 */
void win_set_sb_mode(obj_t *win, sb_mode_t sb_mode)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    page_set_sb_mode(ext->page, sb_mode);
}

/**
 * Set a style of a window
 * @param win pointer to a window object
 * @param type which style should be set
 * @param style pointer to a style
 */
void win_set_style(obj_t *win, win_style_t type, style_t *style)
{
    win_ext_t *ext = obj_get_ext_attr(win);

    switch (type) {
        case GUI_WIN_STYLE_BG:
            obj_set_style(win, style);
            win_realign(win);
            break;
        case GUI_WIN_STYLE_CONTENT_BG:
            page_set_style(ext->page, GUI_PAGE_STYLE_BG, style);
            break;
        case GUI_WIN_STYLE_CONTENT_SCRL:
            page_set_style(ext->page, GUI_PAGE_STYLE_SCRL, style);
            break;
        case GUI_WIN_STYLE_SB:
            page_set_style(ext->page, GUI_PAGE_STYLE_SB, style);
            break;
        case GUI_WIN_STYLE_HEADER:
            obj_set_style(ext->header, style);
            win_realign(win);
            break;
        case GUI_WIN_STYLE_BTN_REL:
            ext->style_btn_rel = style;
            break;
        case GUI_WIN_STYLE_BTN_PR:
            ext->style_btn_pr = style;
            break;
    }

    /*Refresh the existing buttons*/
    if(type == GUI_WIN_STYLE_BTN_REL || type == GUI_WIN_STYLE_BTN_PR) {
        obj_t *btn;
        btn = obj_get_child_back(ext->header, NULL);
        btn = obj_get_child_back(ext->header, btn);    /*Skip the title*/
        while(btn != NULL) {
            if(type == GUI_WIN_STYLE_BTN_REL) btn_set_style(btn, GUI_BTN_STYLE_REL, style);
            else btn_set_style(btn, GUI_BTN_STYLE_PR, style);
            btn = obj_get_child_back(ext->header, btn);
        }
    }

}


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the title of a window
 * @param win pointer to a window object
 * @return title string of the window
 */
const char * win_get_title(obj_t * win)
{
	win_ext_t * ext = obj_get_ext_attr(win);
	return label_get_text(ext->title);
}

/**
 * Get the control button size of a window
 * @param win pointer to a window object
 * @return control button size
 */
coord_t win_get_btn_size(obj_t * win)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    return ext->btn_size;
}

/**
 * Get the layout of a window
 * @param win pointer to a window object
 * @return the layout of the window (from 'layout_t')
 */
layout_t win_get_layout(obj_t *win)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    return page_get_scrl_layout(ext->page);
}

/**
 * Get the scroll bar mode of a window
 * @param win pointer to a window object
 * @return the scroll bar mode of the window (from 'sb_mode_t')
 */
sb_mode_t win_get_sb_mode(obj_t *win)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    return page_get_sb_mode(ext->page);
}

/**
 * Get width of the content area (page scrollable) of the window
 * @param win pointer to a window object
 * @return the width of the content_bg area
 */
coord_t win_get_width(obj_t * win)
{
    win_ext_t * ext = obj_get_ext_attr(win);
    obj_t * scrl = page_get_scrl(ext->page);
    style_t * style_scrl = obj_get_style(scrl);

    return obj_get_width(scrl) - 2 * style_scrl->body.padding.hor;
}

/**
 * Get the pointer of a widow from one of  its control button.
 * It is useful in the action of the control buttons where only button is known.
 * @param ctrl_btn pointer to a control button of a window
 * @return pointer to the window of 'ctrl_btn'
 */
obj_t * win_get_from_btn(obj_t * ctrl_btn)
{
	obj_t * header = obj_get_parent(ctrl_btn);
	obj_t * win = obj_get_parent(header);

	return win;
}

/**
 * Get a style of a window
 * @param win pointer to a button object
 * @param type which style window be get
 * @return style pointer to a style
 */
style_t * win_get_style(obj_t *win, win_style_t type)
{
    win_ext_t *ext = obj_get_ext_attr(win);

    switch (type) {
        case GUI_WIN_STYLE_BG:           return obj_get_style(win);
        case GUI_WIN_STYLE_CONTENT_BG:   return page_get_style(ext->page, GUI_PAGE_STYLE_BG);
        case GUI_WIN_STYLE_CONTENT_SCRL: return page_get_style(ext->page, GUI_PAGE_STYLE_SCRL);
        case GUI_WIN_STYLE_SB:           return page_get_style(ext->page, GUI_PAGE_STYLE_SB);
        case GUI_WIN_STYLE_HEADER:       return obj_get_style(ext->header);
        case GUI_WIN_STYLE_BTN_REL:      return ext->style_btn_rel;
        case GUI_WIN_STYLE_BTN_PR:       return ext->style_btn_pr;
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/*=====================
 * Other functions
 *====================*/

/**
 * Focus on an object. It ensures that the object will be visible in the window.
 * @param win pointer to a window object
 * @param obj pointer to an object to focus (must be in the window)
 * @param anim_time scroll animation time in milliseconds (0: no animation)
 */
void win_focus(obj_t * win, obj_t * obj, uint16_t anim_time)
{
    win_ext_t *ext = obj_get_ext_attr(win);
    page_focus(ext->page, obj, anim_time);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the window
 * @param win pointer to a window object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t win_signal(obj_t * win, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(win, sign, param);
    if(res != GUI_RES_OK) return res;

    win_ext_t * ext = obj_get_ext_attr(win);
    if(sign == GUI_SIGNAL_CHILD_CHG) { /*Move children to the page*/
        obj_t * page = ext->page;
        if(page != NULL) {
            obj_t * child;
            child = obj_get_child(win, NULL);
            while(child != NULL) {
                if(obj_is_protected(child, GUI_PROTECT_PARENT) == false) {
                    obj_t * tmp = child;
                    child = obj_get_child(win, child); /*Get the next child before move this*/
                    obj_set_parent(tmp, page);
                } else {
                    child = obj_get_child(win, child);
                }
            }
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
        win_realign(win);
    }
    else if(sign == GUI_SIGNAL_CORD_CHG) {
        /*If the size is changed refresh the window*/
        if(area_get_width(param) != obj_get_width(win) ||
           area_get_height(param) != obj_get_height(win)) {
            win_realign(win);
        }
    }
    else if(sign == GUI_SIGNAL_CLEANUP) {
        ext->header = NULL;     /*These objects were children so they are already invalid*/
        ext->page = NULL;
        ext->title = NULL;
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "win";
    }


    return res;
}

/**
 * Realign the building elements of a window
 * @param win pointer to window objectker
 */
static void win_realign(obj_t * win)
{
    win_ext_t * ext = obj_get_ext_attr(win);

    if(ext->page == NULL || ext->header == NULL || ext->title == NULL) return;

    style_t *header_style = win_get_style(win, GUI_WIN_STYLE_HEADER);
    obj_set_size(ext->header, obj_get_width(win), ext->btn_size + 2 * header_style->body.padding.ver);

    bool first_btn = true;
    obj_t *btn;
    obj_t *btn_prev = NULL;
    /*Refresh the size of all control buttons*/
    btn = obj_get_child_back(ext->header, NULL);
    btn = obj_get_child_back(ext->header, btn);    /*Skip the title*/
    while(btn != NULL) {
        obj_set_size(btn, ext->btn_size, ext->btn_size);
        if(first_btn) {
            obj_align(btn, ext->header, GUI_ALIGN_IN_RIGHT_MID, - header_style->body.padding.hor, 0);
            first_btn = false;
        } else {
            obj_align(btn, btn_prev, GUI_ALIGN_OUT_LEFT_MID, - header_style->body.padding.inner, 0);
        }
        btn_prev = btn;
        btn = obj_get_child_back(ext->header, btn);
    }


    obj_align(ext->title, NULL, GUI_ALIGN_IN_LEFT_MID, ext->style_header->body.padding.hor, 0);

    obj_set_pos(ext->header, 0, 0);

    obj_set_size( ext->page, obj_get_width(win), obj_get_height(win) - obj_get_height(ext->header));
    obj_align( ext->page, ext->header, GUI_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}

#endif

