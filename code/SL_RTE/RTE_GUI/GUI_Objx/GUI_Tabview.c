/**
 * @file tab.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_TABVIEW != 0

/*********************
 *      DEFINES
 *********************/
#if GUI_USE_ANIMATION
#  ifndef GUI_TABVIEW_ANIM_TIME
#    define GUI_TABVIEW_ANIM_TIME  300 /*Animation time of focusing to the a list element [ms] (0: no animation)  */
#  endif
#else
#  undef  GUI_TABVIEW_ANIM_TIME
#  define GUI_TABVIEW_ANIM_TIME	0	/*No animations*/
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t tabview_signal(obj_t * tabview, signal_t sign, void * param);
static res_t tabpage_signal(obj_t * tab_page, signal_t sign, void * param);
static res_t tabpage_scrl_signal(obj_t * tab_scrl, signal_t sign, void * param);

static void tabpage_pressed_handler(obj_t * tabview, obj_t * tabpage);
static void tabpage_pressing_handler(obj_t * tabview, obj_t * tabpage);
static void tabpage_press_lost_handler(obj_t * tabview, obj_t * tabpage);
static res_t tab_btnm_action(obj_t * tab_btnm, const char * tab_name);
static void tabview_realign(obj_t * tabview);

/**********************
 *  STATIC VARIABLES
 **********************/
static signal_func_t ancestor_signal;
static signal_func_t page_signal;
static signal_func_t page_scrl_signal;
static const char * tab_def[] = {""};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a Tab view object
 * @param par pointer to an object, it will be the parent of the new tab
 * @param copy pointer to a tab object, if not NULL then the new object will be copied from it
 * @return pointer to the created tab
 */
obj_t * tabview_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor of tab*/
    obj_t * new_tabview = obj_create(par, copy);
    RTE_AssertParam(new_tabview);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_tabview);
    
    /*Allocate the tab type specific extended data*/
    tabview_ext_t * ext = obj_allocate_ext_attr(new_tabview, sizeof(tabview_ext_t));
    RTE_AssertParam(ext);

    /*Initialize the allocated 'ext' */
    ext->drag_hor = 0;
    ext->draging = 0;
    ext->slide_enable = 1;
    ext->tab_cur = 0;
    ext->point_last.x = 0;
    ext->point_last.y = 0;
    ext->content = NULL;
    ext->indic = NULL;
    ext->btns = NULL;
    ext->tab_load_action = NULL;
    ext->anim_time = GUI_TABVIEW_ANIM_TIME;
    ext->tab_name_ptr = RTE_BGet(MEM_RTE,sizeof(char*));
    ext->tab_name_ptr[0] = "";
    ext->tab_cnt = 0;

    /*The signal and design functions are not copied so set them here*/
    obj_set_signal_func(new_tabview, tabview_signal);

    /*Init the new tab tab*/
    if(copy == NULL) {
        obj_set_size(new_tabview, GUI_HOR_RES, GUI_VER_RES);

        ext->btns = btnm_create(new_tabview, NULL);
        obj_set_height(ext->btns, 3 * GUI_DPI / 4);
        btnm_set_map(ext->btns, tab_def);
        btnm_set_action(ext->btns, tab_btnm_action);
        btnm_set_toggle(ext->btns, true, 0);

        ext->indic = obj_create(ext->btns, NULL);
        obj_set_width(ext->indic, GUI_DPI);
        obj_align(ext->indic, ext->btns, GUI_ALIGN_IN_BOTTOM_LEFT, 0, 0);
        obj_set_click(ext->indic, false);

        ext->content = cont_create(new_tabview, NULL);
        cont_set_fit(ext->content, true, false);
        cont_set_layout(ext->content, GUI_LAYOUT_ROW_T);
        cont_set_style(ext->content, &style_transp_tight);
        obj_set_height(ext->content, GUI_VER_RES - obj_get_height(ext->btns));
        obj_align(ext->content, ext->btns, GUI_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BG, th->tabview.bg);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_INDIC, th->tabview.indic);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_BG, th->tabview.btn.bg);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_REL, th->tabview.btn.rel);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_PR, th->tabview.btn.pr);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_TGL_REL, th->tabview.btn.tgl_rel);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_TGL_PR, th->tabview.btn.tgl_pr);
        } else {
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BG, &style_plain);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_BTN_BG, &style_transp);
            tabview_set_style(new_tabview, GUI_TABVIEW_STYLE_INDIC, &style_plain_color);
        }
    }
    /*Copy an existing tab view*/
    else {
    	tabview_ext_t * copy_ext = obj_get_ext_attr(copy);
        ext->point_last.x = 0;
        ext->point_last.y = 0;
        ext->btns = btnm_create(new_tabview, copy_ext->btns);
        ext->indic = obj_create(ext->btns, copy_ext->indic);
        ext->content = cont_create(new_tabview, copy_ext->content);
        ext->anim_time = copy_ext->anim_time;
        ext->tab_load_action = copy_ext->tab_load_action;

        ext->tab_name_ptr = RTE_BGet(MEM_RTE,sizeof(char*));
        ext->tab_name_ptr[0] = "";
        btnm_set_map(ext->btns, ext->tab_name_ptr);

        uint16_t i;
        obj_t *new_tab;
        obj_t *copy_tab;
        for (i = 0; i < copy_ext->tab_cnt; i++) {
            new_tab = tabview_add_tab(new_tabview, copy_ext->tab_name_ptr[i]);
            copy_tab = tabview_get_tab(copy, i);
            page_set_style(new_tab, GUI_PAGE_STYLE_BG, page_get_style(copy_tab, GUI_PAGE_STYLE_BG));
            page_set_style(new_tab, GUI_PAGE_STYLE_SCRL, page_get_style(copy_tab, GUI_PAGE_STYLE_SCRL));
            page_set_style(new_tab, GUI_PAGE_STYLE_SB, page_get_style(copy_tab, GUI_PAGE_STYLE_SB));
        }

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_tabview);
    }
    
    return new_tabview;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add a new tab with the given name
 * @param tabview pointer to Tab view object where to ass the new tab
 * @param name the text on the tab button
 * @return pointer to the created page object (page). You can create your content here
 */
obj_t * tabview_add_tab(obj_t * tabview, const char * name)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);

    /*Create the container page*/
    obj_t * h = page_create(ext->content, NULL);
    obj_set_size(h, obj_get_width(tabview), obj_get_height(ext->content));
    page_set_sb_mode(h, GUI_SB_MODE_AUTO);
    page_set_style(h, GUI_PAGE_STYLE_BG, &style_transp);
    page_set_style(h, GUI_PAGE_STYLE_SCRL, &style_transp);

    if(page_signal == NULL) page_signal = obj_get_signal_func(h);
    if(page_scrl_signal == NULL) page_scrl_signal = obj_get_signal_func(page_get_scrl(h));
    obj_set_signal_func(h, tabpage_signal);
    obj_set_signal_func(page_get_scrl(h), tabpage_scrl_signal);

    /*Extend the button matrix map with the new name*/
    char *name_dm;
    if((name[0] & GUI_BTNM_CTRL_MASK) == GUI_BTNM_CTRL_CODE) { /*If control byte presented let is*/
        name_dm = RTE_BGet(MEM_RTE,strlen(name) + 1); /*+1 for the the closing '\0' */
        strcpy(name_dm, name);
    } else { /*Set a no long press control byte is not presented*/
        name_dm = RTE_BGet(MEM_RTE,strlen(name) + 2); /*+1 for the the closing '\0' and +1 for the control byte */
        name_dm[0] = '\221';
        strcpy(&name_dm[1], name);
    }
    ext->tab_cnt++;
    ext->tab_name_ptr = RTE_BGetr(MEM_RTE,ext->tab_name_ptr, sizeof(char *) * (ext->tab_cnt + 1));
    ext->tab_name_ptr[ext->tab_cnt - 1] = name_dm;
    ext->tab_name_ptr[ext->tab_cnt] = "";

    btnm_set_map(ext->btns, ext->tab_name_ptr);

    /*Modify the indicator size*/
    style_t * style_tabs = obj_get_style(ext->btns);
    coord_t indic_width = (obj_get_width(tabview) - style_tabs->body.padding.inner * (ext->tab_cnt - 1) - 2 * style_tabs->body.padding.hor) / ext->tab_cnt;
    obj_set_width(ext->indic, indic_width);
    obj_set_x(ext->indic, indic_width * ext->tab_cur + style_tabs->body.padding.inner * ext->tab_cur + style_tabs->body.padding.hor);

    /*Set the first btn as active*/
    if(ext->tab_cnt == 1) {
        ext->tab_cur = 0;
        tabview_set_tab_act(tabview, 0, false);
        tabview_realign(tabview);       /*To set the proper btns height*/
    }

    return h;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new tab
 * @param tabview pointer to Tab view object
 * @param id index of a tab to load
 * @param anim_en true: set with sliding animation; false: set immediately
 */
void tabview_set_tab_act(obj_t * tabview, uint16_t id, bool anim_en)
{
#if GUI_USE_ANIMATION == 0
    anim_en = false;
#endif
    tabview_ext_t * ext = obj_get_ext_attr(tabview);

    style_t * style = obj_get_style(ext->content);

    if(id >= ext->tab_cnt) id = ext->tab_cnt - 1;
    if(ext->tab_load_action) ext->tab_load_action(tabview, id);

    ext->tab_cur = id;

    coord_t cont_x = -(obj_get_width(tabview) * id + style->body.padding.inner * id + style->body.padding.hor);
    if(ext->anim_time == 0 || anim_en == false) {
        obj_set_x(ext->content, cont_x);
    } else {
#if GUI_USE_ANIMATION
        anim_t a;
        a.var = ext->content;
        a.start = obj_get_x(ext->content);
        a.end = cont_x;
        a.fp = (anim_fp_t)obj_set_x;
        a.path = anim_path_linear;
        a.end_cb = NULL;
        a.act_time = 0;
        a.time = ext->anim_time;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        anim_create(&a);
#endif
    }

    /*Move the indicator*/
    coord_t indic_width = obj_get_width(ext->indic);
    style_t * tabs_style = obj_get_style(ext->btns);
    coord_t indic_x = indic_width * id + tabs_style->body.padding.inner * id + tabs_style->body.padding.hor;

    if(ext->anim_time == 0 || anim_en == false ) {
        obj_set_x(ext->indic, indic_x);
    } else {
#if GUI_USE_ANIMATION
        anim_t a;
        a.var = ext->indic;
        a.start = obj_get_x(ext->indic);
        a.end = indic_x;
        a.fp = (anim_fp_t)obj_set_x;
        a.path = anim_path_linear;
        a.end_cb = NULL;
        a.act_time = 0;
        a.time = ext->anim_time;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        anim_create(&a);
#endif
    }

    btnm_set_toggle(ext->btns, true, ext->tab_cur);
}

/**
 * Set an action to call when a tab is loaded (Good to create content only if required)
 * tabview_get_act() still gives the current (old) tab (to remove content from here)
 * @param tabview pointer to a tabview object
 * @param action pointer to a function to call when a btn is loaded
 */
void tabview_set_tab_load_action(obj_t *tabview, tabview_action_t action)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    ext->tab_load_action = action;
}

/**
 * Enable horizontal sliding with touch pad
 * @param tabview pointer to Tab view object
 * @param en true: enable sliding; false: disable sliding
 */
void tabview_set_sliding(obj_t * tabview, bool en)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    ext->slide_enable = en == false ? 0 : 1;
}

/**
 * Set the animation time of tab view when a new tab is loaded
 * @param tabview pointer to Tab view object
 * @param anim_time_ms time of animation in milliseconds
 */
void tabview_set_anim_time(obj_t * tabview, uint16_t anim_time)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
#if GUI_USE_ANIMATION == 0
    anim_time = 0;
#endif
    ext->anim_time = anim_time;
}

/**
 * Set the style of a tab view
 * @param tabview pointer to a tan view object
 * @param type which style should be set
 * @param style pointer to the new style
 */
void tabview_set_style(obj_t *tabview, tabview_style_t type, style_t *style)
{
    tabview_ext_t *ext = obj_get_ext_attr(tabview);

    switch(type) {
        case GUI_TABVIEW_STYLE_BG:
            obj_set_style(tabview, style);
            break;
        case GUI_TABVIEW_STYLE_BTN_BG:
            btnm_set_style(ext->btns, GUI_BTNM_STYLE_BG, style);
            tabview_realign(tabview);
            break;
        case GUI_TABVIEW_STYLE_BTN_REL:
            btnm_set_style(ext->btns, GUI_BTNM_STYLE_BTN_REL, style);
            tabview_realign(tabview);
            break;
        case GUI_TABVIEW_STYLE_BTN_PR:
            btnm_set_style(ext->btns, GUI_BTNM_STYLE_BTN_PR, style);
            break;
        case GUI_TABVIEW_STYLE_BTN_TGL_REL:
            btnm_set_style(ext->btns, GUI_BTNM_STYLE_BTN_TGL_REL, style);
            break;
        case GUI_TABVIEW_STYLE_BTN_TGL_PR:
            btnm_set_style(ext->btns, GUI_BTNM_STYLE_BTN_TGL_PR, style);
            break;
        case GUI_TABVIEW_STYLE_INDIC:
            obj_set_style(ext->indic, style);
            obj_set_height(ext->indic, style->body.padding.inner);
            tabview_realign(tabview);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the index of the currently active tab
 * @param tabview pointer to Tab view object
 * @return the active btn index
 */
uint16_t tabview_get_tab_act(obj_t * tabview)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    return ext->tab_cur;
}

/**
 * Get the number of tabs
 * @param tabview pointer to Tab view object
 * @return btn count
 */
uint16_t tabview_get_tab_count(obj_t * tabview)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    return ext->tab_cnt;
}

/**
 * Get the page (content area) of a tab
 * @param tabview pointer to Tab view object
 * @param id index of the btn (>= 0)
 * @return pointer to page (page) object
 */
obj_t * tabview_get_tab(obj_t * tabview, uint16_t id)
{
    tabview_ext_t * ext = obj_get_ext_attr(tabview);
    uint16_t i = 0;
    obj_t * page = obj_get_child_back(ext->content, NULL);

    while(page != NULL && i != id) {
        i++;
        page = obj_get_child_back(ext->content, page);
    }

    if(i == id) return page;

    return NULL;
}

/**
 * Get the tab load action
 * @param tabview pointer to a tabview object
 * @param return the current btn load action
 */
tabview_action_t tabview_get_tab_load_action(obj_t *tabview)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    return ext->tab_load_action;
}

/**
 * Get horizontal sliding is enabled or not
 * @param tabview pointer to Tab view object
 * @return true: enable sliding; false: disable sliding
 */
bool tabview_get_sliding(obj_t * tabview)
{
    tabview_ext_t *ext = obj_get_ext_attr(tabview);
    return ext->slide_enable ? true : false;
}

/**
 * Get the animation time of tab view when a new tab is loaded
 * @param tabview pointer to Tab view object
 * @return time of animation in milliseconds
 */
uint16_t tabview_get_anim_time(obj_t * tabview)
{
    tabview_ext_t  * ext = obj_get_ext_attr(tabview);
    return ext->anim_time;
}

/**
 * Get a style of a tab view
 * @param tabview pointer to a ab view object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * tabview_get_style(obj_t *tabview, tabview_style_t type)
{
    tabview_ext_t *ext = obj_get_ext_attr(tabview);

    switch (type) {
        case GUI_TABVIEW_STYLE_BG:           return obj_get_style(tabview);
        case GUI_TABVIEW_STYLE_BTN_BG:       return btnm_get_style(ext->btns, GUI_BTNM_STYLE_BG);
        case GUI_TABVIEW_STYLE_BTN_REL:      return btnm_get_style(ext->btns, GUI_BTNM_STYLE_BTN_REL);
        case GUI_TABVIEW_STYLE_BTN_PR:       return btnm_get_style(ext->btns, GUI_BTNM_STYLE_BTN_PR);
        case GUI_TABVIEW_STYLE_BTN_TGL_REL:  return btnm_get_style(ext->btns, GUI_BTNM_STYLE_BTN_TGL_REL);
        case GUI_TABVIEW_STYLE_BTN_TGL_PR:   return btnm_get_style(ext->btns, GUI_BTNM_STYLE_BTN_TGL_PR);
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the Tab view
 * @param tabview pointer to a Tab view object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t tabview_signal(obj_t * tabview, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(tabview, sign, param);
    if(res != GUI_RES_OK) return res;

    tabview_ext_t * ext = obj_get_ext_attr(tabview);
    if(sign == GUI_SIGNAL_CLEANUP) {
        uint8_t i;
        for(i = 0; ext->tab_name_ptr[i][0] != '\0'; i++) RTE_BRel(MEM_RTE,(void *)ext->tab_name_ptr[i]);

        RTE_BRel(MEM_RTE,ext->tab_name_ptr);
        ext->tab_name_ptr = NULL;
        ext->btns = NULL;     /*These objects were children so they are already invalid*/
        ext->content= NULL;
    }
    else if(sign == GUI_SIGNAL_CORD_CHG) {
        if(ext->content != NULL &&
          (obj_get_width(tabview) != area_get_width(param) ||
           obj_get_height(tabview) != area_get_height(param)))
        {
            tabview_realign(tabview);
        }
    }
    else if(sign == GUI_SIGNAL_FOCUS || sign == GUI_SIGNAL_DEFOCUS || sign == GUI_SIGNAL_CONTROLL) {
        if(ext->btns) {
            ext->btns->signal_func(ext->btns, sign, param);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "tabview";
    }

    return res;
}


/**
 * Signal function of a tab's page
 * @param tab pointer to a tab page object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t tabpage_signal(obj_t * tab_page, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = page_signal(tab_page, sign, param);
    if(res != GUI_RES_OK) return res;

    obj_t * cont = obj_get_parent(tab_page);
    obj_t * tabview = obj_get_parent(cont);

    if(tabview_get_sliding(tabview) == false) return res;

    if(sign == GUI_SIGNAL_PRESSED) {
        tabpage_pressed_handler(tabview, tab_page);
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        tabpage_pressing_handler(tabview, tab_page);
    }
    else if(sign == GUI_SIGNAL_RELEASED || sign == GUI_SIGNAL_PRESS_LOST) {
        tabpage_press_lost_handler(tabview, tab_page);
    }

    return res;
}
/**
 * Signal function of the tab page's scrollable object
 * @param tab_scrl pointer to a tab page's scrollable object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t tabpage_scrl_signal(obj_t * tab_scrl, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = page_scrl_signal(tab_scrl, sign, param);
    if(res != GUI_RES_OK) return res;

    obj_t * tab_page = obj_get_parent(tab_scrl);
    obj_t * cont = obj_get_parent(tab_page);
    obj_t * tabview = obj_get_parent(cont);

    if(tabview_get_sliding(tabview) == false) return res;

    if(sign == GUI_SIGNAL_PRESSED) {
        tabpage_pressed_handler(tabview, tab_page);
    }
    else if(sign == GUI_SIGNAL_PRESSING) {
        tabpage_pressing_handler(tabview, tab_page);
    }
    else if(sign == GUI_SIGNAL_RELEASED || sign == GUI_SIGNAL_PRESS_LOST) {
        tabpage_press_lost_handler(tabview, tab_page);
    }

    return res;
}

/**
 * Called when a tab's page or scrollable object is pressed
 * @param tabview pointer to the btn view object
 * @param tabpage pointer to the page of a btn
 */
static void tabpage_pressed_handler(obj_t * tabview, obj_t * tabpage)
{
    (void)tabpage;

    tabview_ext_t * ext = obj_get_ext_attr(tabview);
    indev_t * indev = indev_get_act();
    indev_get_point(indev, &ext->point_last);
}

/**
 * Called when a tab's page or scrollable object is being pressed
 * @param tabview pointer to the btn view object
 * @param tabpage pointer to the page of a btn
 */
static void tabpage_pressing_handler(obj_t * tabview, obj_t * tabpage)
{
    tabview_ext_t * ext = obj_get_ext_attr(tabview);
    indev_t * indev = indev_get_act();
    point_t point_act;
    indev_get_point(indev, &point_act);
    coord_t x_diff = point_act.x - ext->point_last.x;
    coord_t y_diff = point_act.y - ext->point_last.y;

    if(ext->draging == 0) {
        if(x_diff >= GUI_INDEV_DRAG_LIMIT || x_diff<= -GUI_INDEV_DRAG_LIMIT) {
            ext->drag_hor = 1;
            ext->draging = 1;
            obj_set_drag(page_get_scrl(tabpage), false);
        } else if(y_diff >= GUI_INDEV_DRAG_LIMIT || y_diff <= -GUI_INDEV_DRAG_LIMIT) {
            ext->drag_hor = 0;
            ext->draging = 1;
        }
    }
    if(ext->drag_hor) {
        obj_set_x(ext->content, obj_get_x(ext->content) + point_act.x - ext->point_last.x);
        ext->point_last.x = point_act.x;
        ext->point_last.y = point_act.y;

        /*Move the indicator*/
        coord_t indic_width = obj_get_width(ext->indic);
        style_t * tabs_style = obj_get_style(ext->btns);
        style_t * indic_style = obj_get_style(ext->indic);
        coord_t p = ((tabpage->coords.x1 - tabview->coords.x1) * (indic_width + tabs_style->body.padding.inner)) / obj_get_width(tabview);

        obj_set_x(ext->indic, indic_width * ext->tab_cur + tabs_style->body.padding.inner * ext->tab_cur + indic_style->body.padding.hor - p);
    }
}

/**
 * Called when a tab's page or scrollable object is released or the press id lost
 * @param tabview pointer to the btn view object
 * @param tabpage pointer to the page of a btn
 */
static void tabpage_press_lost_handler(obj_t * tabview, obj_t * tabpage)
{
    tabview_ext_t * ext = obj_get_ext_attr(tabview);
    ext->drag_hor = 0;
    ext->draging = 0;

    obj_set_drag(page_get_scrl(tabpage), true);

    indev_t * indev = indev_get_act();
    point_t point_act;
    indev_get_point(indev, &point_act);
    point_t vect;
    indev_get_vect(indev, &vect);
    coord_t x_predict = 0;

    while(vect.x != 0)   {
        x_predict += vect.x;
        vect.x = vect.x * (100 - GUI_INDEV_DRAG_THROW) / 100;
    }

    coord_t page_x1 = tabpage->coords.x1 - tabview->coords.x1 + x_predict;
    coord_t page_x2 = page_x1 + obj_get_width(tabpage);
    coord_t treshold = obj_get_width(tabview) / 2;

    uint16_t tab_cur = ext->tab_cur;
    if(page_x1 > treshold) {
        if(tab_cur != 0) tab_cur--;
    } else if(page_x2 < treshold) {
        if(tab_cur < ext->tab_cnt - 1) tab_cur++;
    }

    tabview_set_tab_act(tabview, tab_cur, true);
}

/**
 * Called when a tab button is released
 * @param tab_btnm pointer to the tab's button matrix object
 * @param id the id of the tab (>= 0)
 * @return GUI_ACTION_RES_OK because the button matrix in not deleted in the function
 */
static res_t tab_btnm_action(obj_t * tab_btnm, const char * tab_name)
{
    obj_t * tab = obj_get_parent(tab_btnm);
    const char ** tabs_map = btnm_get_map(tab_btnm);

    uint8_t i = 0;

    while(tabs_map[i][0] != '\0') {
        if(strcmp(&tabs_map[i][1], tab_name) == 0) break;   /*[1] to skip the control byte*/
        i++;
    }

    tabview_set_tab_act(tab, i, true);

    return GUI_RES_OK;
}

/**
 * Realign and resize the elements of Tab view
 * @param tabview pointer to a Tab view object
 */
static void tabview_realign(obj_t * tabview)
{
    tabview_ext_t * ext = obj_get_ext_attr(tabview);

    obj_set_width(ext->btns, obj_get_width(tabview));

    if(ext->tab_cnt != 0) {
        style_t * style_btn_bg = tabview_get_style(tabview, GUI_TABVIEW_STYLE_BTN_BG);
        style_t * style_btn_rel = tabview_get_style(tabview, GUI_TABVIEW_STYLE_BTN_REL);

        /*Set the indicator widths*/
        coord_t indic_width = (obj_get_width(tabview) - style_btn_bg->body.padding.inner * (ext->tab_cnt - 1) -
                2 * style_btn_bg->body.padding.hor) / ext->tab_cnt;
        obj_set_width(ext->indic, indic_width);

        /*Set the tabs height*/
        coord_t btns_height = font_get_height(style_btn_rel->text.font) +
                              2 * style_btn_rel->body.padding.ver +
                              2 * style_btn_bg->body.padding.ver;
        obj_set_height(ext->btns, btns_height);
    }

    obj_set_height(ext->content, obj_get_height(tabview) - obj_get_height(ext->btns));
    obj_align(ext->content, ext->btns, GUI_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    obj_t * pages = obj_get_child(ext->content, NULL);
    while(pages != NULL) {
        if(obj_get_signal_func(pages) == tabpage_signal) {  /*Be sure adjust only the pages (user can other things)*/
            obj_set_size(pages, obj_get_width(tabview), obj_get_height(ext->content));
        }
        pages = obj_get_child(ext->content, pages);
    }


    obj_align(ext->indic, ext->btns, GUI_ALIGN_IN_BOTTOM_LEFT, 0, 0);

    tabview_set_tab_act(tabview, ext->tab_cur, false);
}
#endif
