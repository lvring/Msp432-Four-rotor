/**
 * @file list.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LIST != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_LIST_LAYOUT_DEF	GUI_LAYOUT_COL_M

#if GUI_USE_ANIMATION
#  ifndef GUI_LIST_FOCUS_TIME
#    define GUI_LIST_FOCUS_TIME  100 /*Animation time of focusing to the a list element [ms] (0: no animation)  */
#  endif
#else
#  undef  GUI_LIST_FOCUS_TIME
#  define GUI_LIST_FOCUS_TIME	0	/*No animations*/
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t list_signal(obj_t *list, signal_t sign, void *param);
static obj_t * get_next_btn(obj_t *list, obj_t *prev_btn);
static void refr_btn_width(obj_t *list);

/**********************
 *  STATIC VARIABLES
 **********************/
static signal_func_t btn_signal;
#if GUI_USE_IMG
static signal_func_t img_signal;
#endif
static signal_func_t label_signal;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a list objects
 * @param par pointer to an object, it will be the parent of the new list
 * @param copy pointer to a list object, if not NULL then the new object will be copied from it
 * @return pointer to the created list
 */
obj_t * list_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor basic object*/
    obj_t * new_list = page_create(par, copy);
    RTE_AssertParam(new_list);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_list);

    list_ext_t * ext = obj_allocate_ext_attr(new_list, sizeof(list_ext_t));
    RTE_AssertParam(ext);

    ext->style_img = NULL;
    ext->styles_btn[GUI_BTN_STATE_REL] = &style_btn_rel;
    ext->styles_btn[GUI_BTN_STATE_PR] = &style_btn_pr;
    ext->styles_btn[GUI_BTN_STATE_TGL_REL] = &style_btn_tgl_rel;
    ext->styles_btn[GUI_BTN_STATE_TGL_PR] = &style_btn_tgl_pr;
    ext->styles_btn[GUI_BTN_STATE_INA] = &style_btn_ina;
    ext->anim_time = GUI_LIST_FOCUS_TIME;

	obj_set_signal_func(new_list, list_signal);

    /*Init the new list object*/
    if(copy == NULL) {
    	obj_set_size(new_list, 2 * GUI_DPI, 3 * GUI_DPI);
		page_set_scrl_layout(new_list, GUI_LIST_LAYOUT_DEF);
		list_set_sb_mode(new_list, GUI_SB_MODE_DRAG);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            list_set_style(new_list, GUI_LIST_STYLE_BG, th->list.bg);
            list_set_style(new_list, GUI_LIST_STYLE_SCRL, th->list.scrl);
            list_set_style(new_list, GUI_LIST_STYLE_SB, th->list.sb);
            list_set_style(new_list, GUI_LIST_STYLE_BTN_REL, th->list.btn.rel);
            list_set_style(new_list, GUI_LIST_STYLE_BTN_PR, th->list.btn.pr);
            list_set_style(new_list, GUI_LIST_STYLE_BTN_TGL_REL, th->list.btn.tgl_rel);
            list_set_style(new_list, GUI_LIST_STYLE_BTN_TGL_PR, th->list.btn.tgl_pr);
            list_set_style(new_list, GUI_LIST_STYLE_BTN_INA, th->list.btn.ina);
        } else {
            list_set_style(new_list, GUI_LIST_STYLE_BG, &style_transp_fit);
            list_set_style(new_list, GUI_LIST_STYLE_SCRL, &style_pretty);
        }
    } else {
        list_ext_t * copy_ext = obj_get_ext_attr(copy);

        obj_t *copy_btn = obj_get_child_back(page_get_scrl(copy), NULL);
        obj_t *new_btn;
        while(copy_btn) {
            new_btn = btn_create(new_list, copy_btn);
#if GUI_USE_IMG
            obj_t *copy_img = list_get_btn_img(copy_btn);
            if(copy_img) img_create(new_btn, copy_img);
#endif
            label_create(new_btn, list_get_btn_label(copy_btn));
            copy_btn = obj_get_child_back(page_get_scrl(copy), copy_btn);
        }

        list_set_style(new_list, GUI_LIST_STYLE_BTN_REL, copy_ext->styles_btn[GUI_BTN_STATE_REL]);
        list_set_style(new_list, GUI_LIST_STYLE_BTN_PR, copy_ext->styles_btn[GUI_BTN_STATE_PR]);
        list_set_style(new_list, GUI_LIST_STYLE_BTN_TGL_REL, copy_ext->styles_btn[GUI_BTN_STATE_TGL_REL]);
        list_set_style(new_list, GUI_LIST_STYLE_BTN_TGL_PR, copy_ext->styles_btn[GUI_BTN_STATE_TGL_REL]);
        list_set_style(new_list, GUI_LIST_STYLE_BTN_INA, copy_ext->styles_btn[GUI_BTN_STATE_INA]);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_list);
    }
    
    return new_list;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add a list element to the list
 * @param list pointer to list object
 * @param img_fn file name of an image before the text (NULL if unused)
 * @param txt text of the list element (NULL if unused)
 * @param rel_action pointer to release action function (like with btn)
 * @return pointer to the new list element which can be customized (a button)
 */
obj_t * list_add(obj_t * list, const void * img_src, const char * txt, action_t rel_action)
{
	style_t * style = obj_get_style(list);
    list_ext_t * ext = obj_get_ext_attr(list);

	/*Create a list element with the image an the text*/
	obj_t * liste;
	liste = btn_create(list, NULL);

    /*Set the default styles*/
    btn_set_style(liste, GUI_BTN_STYLE_REL, ext->styles_btn[GUI_BTN_STATE_REL]);
    btn_set_style(liste, GUI_BTN_STYLE_PR, ext->styles_btn[GUI_BTN_STATE_PR]);
    btn_set_style(liste, GUI_BTN_STYLE_TGL_REL, ext->styles_btn[GUI_BTN_STATE_TGL_REL]);
    btn_set_style(liste, GUI_BTN_STYLE_TGL_PR, ext->styles_btn[GUI_BTN_STATE_TGL_PR]);
    btn_set_style(liste, GUI_BTN_STYLE_INA, ext->styles_btn[GUI_BTN_STATE_INA]);

	btn_set_action(liste, GUI_BTN_ACTION_CLICK, rel_action);
	page_glue_obj(liste, true);
	btn_set_layout(liste, GUI_LAYOUT_ROW_M);
	btn_set_fit(liste, false, true);
    if(btn_signal == NULL) btn_signal = obj_get_signal_func(liste);

    /*Make the size adjustment*/
    coord_t w = obj_get_width(list);
    style_t *  style_scrl = obj_get_style(page_get_scrl(list));
    coord_t pad_hor_tot = style->body.padding.hor + style_scrl->body.padding.hor;
    w -= pad_hor_tot * 2;

    obj_set_width(liste, w);
#if GUI_USE_IMG != 0
    obj_t * img = NULL;
	if(img_src) {
		img = img_create(liste, NULL);
		img_set_src(img, img_src);
		obj_set_style(img, ext->style_img);
		obj_set_click(img, false);
		if(img_signal == NULL) img_signal = obj_get_signal_func(img);
	}
#endif
	if(txt != NULL) {
		obj_t * label = label_create(liste, NULL);
		label_set_text(label, txt);
		obj_set_click(label, false);
        label_set_long_mode(label, GUI_LABEL_LONG_ROLL);
		obj_set_width(label, liste->coords.x2 - label->coords.x1);
        if(label_signal == NULL) label_signal = obj_get_signal_func(label);
	}

	return liste;
}

/*=====================
 * Setter functions 
 *====================*/

/**
 * Set scroll animation duration on 'list_up()' 'list_down()' 'list_focus()'
 * @param list pointer to a list object
 * @param anim_time duration of animation [ms]
 */
void list_set_anim_time(obj_t *list, uint16_t anim_time)
{
    list_ext_t * ext = obj_get_ext_attr(list);
#if GUI_USE_ANIMATION == 0
    anim_time = 0;
#endif

    if(ext->anim_time == anim_time) return;
    ext->anim_time = anim_time;
}

/**
 * Set a style of a list
 * @param list pointer to a list object
 * @param type which style should be set
 * @param style pointer to a style
 */
void list_set_style(obj_t *list, list_style_t type, style_t *style)
{
    list_ext_t *ext = obj_get_ext_attr(list);
    btn_style_t btn_style_refr = GUI_BTN_STYLE_REL;
    obj_t *btn;

    switch (type) {
        case GUI_LIST_STYLE_BG:
            page_set_style(list, GUI_PAGE_STYLE_BG, style);
            /*style change signal will call 'refr_btn_width' */
            break;
        case GUI_LIST_STYLE_SCRL:
            page_set_style(list, GUI_PAGE_STYLE_SCRL, style);
            refr_btn_width(list);
            break;
        case GUI_LIST_STYLE_SB:
            page_set_style(list, GUI_PAGE_STYLE_SB, style);
            break;
        case GUI_LIST_STYLE_BTN_REL:
            ext->styles_btn[GUI_BTN_STATE_REL] = style;
            btn_style_refr = GUI_BTN_STYLE_REL;
            break;
        case GUI_LIST_STYLE_BTN_PR:
            ext->styles_btn[GUI_BTN_STATE_PR] = style;
            btn_style_refr = GUI_BTN_STYLE_PR;
            break;
        case GUI_LIST_STYLE_BTN_TGL_REL:
            ext->styles_btn[GUI_BTN_STATE_TGL_REL] = style;
            btn_style_refr = GUI_BTN_STYLE_TGL_REL;
            break;
        case GUI_LIST_STYLE_BTN_TGL_PR:
            ext->styles_btn[GUI_BTN_STATE_TGL_PR] = style;
            btn_style_refr = GUI_BTN_STYLE_TGL_PR;
            break;
        case GUI_LIST_STYLE_BTN_INA:
            ext->styles_btn[GUI_BTN_STATE_INA] = style;
            btn_style_refr = GUI_BTN_STYLE_INA;
            break;
    }


    /*Refresh existing buttons' style*/
    if(type == GUI_LIST_STYLE_BTN_PR || type == GUI_LIST_STYLE_BTN_REL ||
       type == GUI_LIST_STYLE_BTN_TGL_REL || type == GUI_LIST_STYLE_BTN_TGL_PR ||
       type == GUI_LIST_STYLE_BTN_INA)
    {
        btn= get_next_btn(list, NULL);
        while(btn != NULL) {
            btn_set_style(btn, btn_style_refr, ext->styles_btn[btn_style_refr]);
            btn = get_next_btn(list, btn);
        }
    }
}

/*=====================
 * Getter functions 
 *====================*/

/**
 * Get the text of a list element
 * @param btn pointer to list element
 * @return pointer to the text
 */
const char * list_get_btn_text(obj_t * btn)
{
    obj_t * label = list_get_btn_label(btn);
    if(label == NULL) return "";
    return label_get_text(label);
}

/**
 * Get the label object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the label from the list element or NULL if not found
 */
obj_t * list_get_btn_label(obj_t * btn)
{
    obj_t * label = obj_get_child(btn, NULL);
    if(label == NULL) return NULL;

    while(label->signal_func != label_signal) {
        label = obj_get_child(btn, label);
        if(label == NULL) break;
    }

    return label;
}

/**
 * Get the image object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the image from the list element or NULL if not found
 */
obj_t * list_get_btn_img(obj_t * btn)
{
#if GUI_USE_IMG != 0
    obj_t * img = obj_get_child(btn, NULL);
    if(img == NULL) return NULL;

    while(img->signal_func != img_signal) {
        img = obj_get_child(btn, img);
        if(img == NULL) break;
    }

    return img;
#else
    return NULL;
#endif
}

/**
 * Get scroll animation duration
 * @param list pointer to a list object
 * @return duration of animation [ms]
 */
uint16_t list_get_anim_time(obj_t *list)
{
    list_ext_t * ext = obj_get_ext_attr(list);
    return ext->anim_time;
}

/**
 * Get a style of a list
 * @param list pointer to a list object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
style_t * list_get_style(obj_t *list, list_style_t type)
{
    list_ext_t *ext = obj_get_ext_attr(list);

    switch (type) {
        case GUI_LIST_STYLE_BG:          return page_get_style(list, GUI_PAGE_STYLE_BG);
        case GUI_LIST_STYLE_SCRL:        return page_get_style(list, GUI_PAGE_STYLE_SB);
        case GUI_LIST_STYLE_SB:          return page_get_style(list, GUI_PAGE_STYLE_SCRL);
        case GUI_LIST_STYLE_BTN_REL:     return ext->styles_btn[GUI_BTN_STATE_REL];
        case GUI_LIST_STYLE_BTN_PR:      return ext->styles_btn[GUI_BTN_STATE_PR];
        case GUI_LIST_STYLE_BTN_TGL_REL: return ext->styles_btn[GUI_BTN_STATE_TGL_REL];
        case GUI_LIST_STYLE_BTN_TGL_PR:  return ext->styles_btn[GUI_BTN_STATE_TGL_PR];
        case GUI_LIST_STYLE_BTN_INA:     return ext->styles_btn[GUI_BTN_STATE_INA];
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}
/*=====================
 * Other functions
 *====================*/

/**
 * Move the list elements up by one
 * @param list pointer a to list object
 */
void list_up(obj_t * list)
{
    /*Search the first list element which 'y' coordinate is below the parent
     * and position the list to show this element on the bottom*/
    obj_t * scrl = page_get_scrl(list);
    obj_t * e;
    obj_t * e_prev = NULL;
    e = get_next_btn(list, NULL);
    while(e != NULL) {
        if(e->coords.y2 <= list->coords.y2) {
            if(e_prev != NULL) {
                coord_t new_y = obj_get_height(list) - (obj_get_y(e_prev) + obj_get_height(e_prev));
                list_ext_t *ext = obj_get_ext_attr(list);
                if(ext->anim_time == 0) {
                    obj_set_y(scrl, new_y);
                } else {
#if GUI_USE_ANIMATION
                    anim_t a;
                    a.var = scrl;
                    a.start = obj_get_y(scrl);
                    a.end = new_y;
                    a.fp = (anim_fp_t)obj_set_y;
                    a.path = anim_path_linear;
                    a.end_cb = NULL;
                    a.act_time = 0;
                    a.time = GUI_LIST_FOCUS_TIME;
                    a.playback = 0;
                    a.playback_pause = 0;
                    a.repeat = 0;
                    a.repeat_pause = 0;
                    anim_create(&a);
#endif
                }
            }
            break;
        }
        e_prev = e;
        e = get_next_btn(list, e);
    }
}

/**
 * Move the list elements down by one
 * @param list pointer to a list object
 */
void list_down(obj_t * list)
{
    /*Search the first list element which 'y' coordinate is above the parent
     * and position the list to show this element on the top*/
    obj_t * scrl = page_get_scrl(list);
    obj_t * e;
    e = get_next_btn(list, NULL);
    while(e != NULL) {
        if(e->coords.y1 < list->coords.y1) {
            coord_t new_y = -obj_get_y(e);
            list_ext_t *ext = obj_get_ext_attr(list);
            if(ext->anim_time == 0) {
                obj_set_y(scrl, new_y);
            } else {
#if GUI_USE_ANIMATION
                anim_t a;
                a.var = scrl;
                a.start = obj_get_y(scrl);
                a.end = new_y;
                a.fp = (anim_fp_t)obj_set_y;
                a.path = anim_path_linear;
                a.end_cb = NULL;
                a.act_time = 0;
                a.time = GUI_LIST_FOCUS_TIME;
                a.playback = 0;
                a.playback_pause = 0;
                a.repeat = 0;
                a.repeat_pause = 0;
                anim_create(&a);

#endif
            }
            break;
        }
        e = get_next_btn(list, e);
    }
}

/**
 * Focus on a list button. It ensures that the button will be visible on the list.
 * @param btn pointer to a list button to focus
 * @param anim_en true: scroll with animation, false: without animation
 */
void list_focus(obj_t *btn, bool anim_en)
{

#if GUI_USE_ANIMATION == 0
    anim_en = false;
#endif

    obj_t *list = obj_get_parent(obj_get_parent(btn));

    page_focus(list, btn, anim_en == false ? 0 :list_get_anim_time(list));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the list
 * @param list pointer to a list object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t list_signal(obj_t * list, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(list, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_CORD_CHG) {
        /*Be sure the width of the buttons are correct*/
        coord_t w = obj_get_width(list);
        if(w != area_get_width(param)) {   /*Width changed*/
           refr_btn_width(list);
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
        /*Because of the possible change of horizontal and vertical padding refresh buttons width */
        refr_btn_width(list);
    }
    else if(sign == GUI_SIGNAL_FOCUS) {
        /*Get the first button*/
        obj_t * btn = NULL;
        obj_t * btn_prev = NULL;
        btn = get_next_btn(list, btn);
        while(btn != NULL) {
            btn_prev = btn;
            btn = get_next_btn(list, btn);
        }
        if(btn_prev != NULL) {
            btn_set_state(btn_prev, GUI_BTN_STATE_TGL_REL);
        }
    }
    else if(sign == GUI_SIGNAL_DEFOCUS) {
        /*Get the 'pressed' button*/
        obj_t * btn = NULL;
        btn = get_next_btn(list, btn);
        while(btn != NULL) {
            if(btn_get_state(btn) == GUI_BTN_STATE_TGL_REL) break;
            btn = get_next_btn(list, btn);
        }

        if(btn != NULL) {
            btn_set_state(btn, GUI_BTN_STATE_REL);
        }
    }
    else if(sign == GUI_SIGNAL_CONTROLL) {
        char c = *((char*)param);
        if(c == GUI_GROUP_KEY_RIGHT || c == GUI_GROUP_KEY_DOWN) {
            /*Get the last pressed button*/
            obj_t * btn = NULL;
            obj_t * btn_prev = NULL;
            list_ext_t *ext = obj_get_ext_attr(list);
            btn = get_next_btn(list, btn);
            while(btn != NULL) {
                if(btn_get_state(btn) == GUI_BTN_STATE_TGL_REL) break;
                btn_prev = btn;
                btn = get_next_btn(list, btn);
            }

            if(btn_prev != NULL && btn != NULL) {
                btn_set_state(btn, GUI_BTN_STATE_REL);
                btn_set_state(btn_prev, GUI_BTN_STATE_TGL_REL);
                page_focus(list, btn_prev, ext->anim_time);
            }
        }
        else if(c == GUI_GROUP_KEY_LEFT || c == GUI_GROUP_KEY_UP) {
            /*Get the last pressed button*/
            obj_t * btn = NULL;
            list_ext_t *ext = obj_get_ext_attr(list);
            btn = get_next_btn(list, btn);
            while(btn != NULL) {
                if(btn_get_state(btn) == GUI_BTN_STATE_TGL_REL) break;
                btn = get_next_btn(list, btn);
            }

            if(btn != NULL) {
                obj_t * btn_prev = get_next_btn(list, btn);
                if(btn_prev != NULL) {
                    btn_set_state(btn, GUI_BTN_STATE_REL);
                    btn_set_state(btn_prev, GUI_BTN_STATE_TGL_REL);
                    page_focus(list, btn_prev, ext->anim_time);
                }
            }
        } else if(c == GUI_GROUP_KEY_ENTER || c == GUI_GROUP_KEY_ENTER_LONG) {
            /*Get the 'pressed' button*/
            obj_t * btn = NULL;
            btn = get_next_btn(list, btn);
            while(btn != NULL) {
                if(btn_get_state(btn) == GUI_BTN_STATE_TGL_REL) break;
                btn = get_next_btn(list, btn);
            }

            if(btn != NULL) {
                action_t rel_action;
                rel_action = btn_get_action(btn, GUI_BTN_ACTION_CLICK);
                if(rel_action != NULL) rel_action(btn);
            }
        }
        else if(sign == GUI_SIGNAL_GET_TYPE) {
            obj_type_t * buf = param;
            uint8_t i;
            for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
                if(buf->type[i] == NULL) break;
            }
            buf->type[i] = "list";
        }
    }
    return res;
}

/**
 * Get the next button from list
 * @param list pointer to a list object
 * @param prev_btn pointer to button. Search the next after it.
 * @return pointer to the next button or NULL
 */
static obj_t * get_next_btn(obj_t * list, obj_t * prev_btn)
{
    /* Not a good practice but user can add/create objects to the lists manually.
     * When getting the next button try to be sure that it is at least a button */

    obj_t * btn ;
    obj_t * scrl = page_get_scrl(list);

    btn = obj_get_child(scrl, prev_btn);
    if(btn == NULL) return NULL;

    while(btn->signal_func != btn_signal) {
        btn = obj_get_child(scrl, prev_btn);
        if(btn == NULL) break;
    }

    return btn;
}

static void refr_btn_width(obj_t *list)
{
   style_t *style = list_get_style(list, GUI_LIST_STYLE_BG);
   style_t *style_scrl = obj_get_style(page_get_scrl(list));
   coord_t w = obj_get_width(list);
   coord_t btn_w = w - (style->body.padding.hor + style_scrl->body.padding.hor) * 2;

   obj_t *btn = get_next_btn(list, NULL);
   while(btn) {
       /*Make the size adjustment for each buttons*/
       if(obj_get_width(btn) != btn_w) {
           obj_set_width(btn, btn_w);
           /*Set the label size to roll its text*/
           obj_t *label = list_get_btn_label(btn);
           obj_set_width(label, btn->coords.x2 - label->coords.x1);
           label_set_text(label, NULL);
       }
       btn = get_next_btn(list, btn);
   }
}


#endif
