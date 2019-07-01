/**
 * @file page.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_PAGE != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_PAGE_SB_MIN_SIZE    (GUI_DPI / 8)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void page_sb_refresh(obj_t * main);
static bool page_design(obj_t * page, const area_t * mask, design_mode_t mode);
static bool scrl_design(obj_t * scrl, const area_t * mask, design_mode_t mode);
static res_t page_signal(obj_t * page, signal_t sign, void * param);
static res_t page_scrollable_signal(obj_t * scrl, signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static design_func_t ancestor_design;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a page objects
 * @param par pointer to an object, it will be the parent of the new page
 * @param copy pointer to a page object, if not NULL then the new object will be copied from it
 * @return pointer to the created page
 */
obj_t * page_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor object*/
    obj_t * new_page = cont_create(par, copy);
    RTE_AssertParam(new_page);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_page);
    if(ancestor_design == NULL) ancestor_design = obj_get_design_func(new_page);

    /*Allocate the object type specific extended data*/
    page_ext_t * ext = obj_allocate_ext_attr(new_page, sizeof(page_ext_t));
    RTE_AssertParam(ext);
    ext->scrl = NULL;
    ext->pr_action = NULL;
    ext->rel_action = NULL;
    ext->sb.hor_draw = 0;
    ext->sb.ver_draw = 0;
    ext->sb.style = &style_pretty;
    ext->sb.mode = GUI_SB_MODE_AUTO;

    /*Init the new page object*/
    if(copy == NULL) {
	    ext->scrl = cont_create(new_page, NULL);
	    obj_set_signal_func(ext->scrl, page_scrollable_signal);
        obj_set_design_func(ext->scrl, scrl_design);
		obj_set_drag(ext->scrl, true);
		obj_set_drag_throw(ext->scrl, true);
		obj_set_protect(ext->scrl, GUI_PROTECT_PARENT | GUI_PROTECT_PRESS_LOST);
		cont_set_fit(ext->scrl, false, true);

		/* Add the signal function only if 'scrolling' is created
		 * because everything has to be ready before any signal is received*/
	    obj_set_signal_func(new_page, page_signal);
	    obj_set_design_func(new_page, page_design);

        page_set_sb_mode(new_page, (sb_mode_t)ext->sb.mode);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            if(par == NULL){ /*Different styles if it is screen*/
                page_set_style(new_page, GUI_PAGE_STYLE_BG, th->bg);
                page_set_style(new_page, GUI_PAGE_STYLE_SCRL, &style_transp);
            } else {
                page_set_style(new_page, GUI_PAGE_STYLE_BG, th->page.bg);
                page_set_style(new_page, GUI_PAGE_STYLE_SCRL, th->page.scrl);

            }
            page_set_style(new_page, GUI_PAGE_STYLE_SB, th->page.sb);
        } else {
            page_set_style(new_page, GUI_PAGE_STYLE_BG, &style_pretty_color);
            page_set_style(new_page, GUI_PAGE_STYLE_SCRL, &style_pretty);
            page_set_style(new_page, GUI_PAGE_STYLE_SB, &style_pretty_color);
        }

    } else {
    	page_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->scrl = cont_create(new_page, copy_ext->scrl);
	    obj_set_signal_func(ext->scrl, page_scrollable_signal);

        page_set_pr_action(new_page, copy_ext->pr_action);
        page_set_rel_action(new_page, copy_ext->rel_action);
        page_set_sb_mode(new_page, (sb_mode_t)copy_ext->sb.mode);

        page_set_style(new_page, GUI_PAGE_STYLE_BG, page_get_style(copy, GUI_PAGE_STYLE_BG));
        page_set_style(new_page, GUI_PAGE_STYLE_SCRL, page_get_style(copy, GUI_PAGE_STYLE_SCRL));
        page_set_style(new_page, GUI_PAGE_STYLE_SB, page_get_style(copy, GUI_PAGE_STYLE_SB));

		/* Add the signal function only if 'scrolling' is created
		 * because everything has to be ready before any signal is received*/
	    obj_set_signal_func(new_page, page_signal);
	    obj_set_design_func(new_page, page_design);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_page);
    }
    
    page_sb_refresh(new_page);
                
    return new_page;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a release action for the page
 * @param page pointer to a page object
 * @param rel_action a function to call when the page is released
 */
void page_set_rel_action(obj_t * page, action_t rel_action)
{
	page_ext_t * ext = obj_get_ext_attr(page);
	ext->rel_action = rel_action;
}

/**
 * Set a press action for the page
 * @param page pointer to a page object
 * @param pr_action a function to call when the page is pressed
 */
void page_set_pr_action(obj_t * page, action_t pr_action)
{
	page_ext_t * ext = obj_get_ext_attr(page);
	ext->pr_action = pr_action;
}

/**
 * Set the scroll bar mode on a page
 * @param page pointer to a page object
 * @param sb.mode the new mode from 'page_sb.mode_t' enum
 */
void page_set_sb_mode(obj_t * page, sb_mode_t sb_mode)
{
    page_ext_t * ext = obj_get_ext_attr(page);
    if(ext->sb.mode == sb_mode) return;

    ext->sb.mode = sb_mode;
    ext->sb.hor_draw = 0;
    ext->sb.ver_draw = 0;
    page_sb_refresh(page);
    obj_invalidate(page);
}

/**
 * Set a style of a page
 * @param page pointer to a page object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void page_set_style(obj_t *page, page_style_t type, style_t *style)
{
    page_ext_t * ext = obj_get_ext_attr(page);

    switch (type) {
        case GUI_PAGE_STYLE_BG:
            obj_set_style(page, style);
            break;
        case GUI_PAGE_STYLE_SCRL:
            obj_set_style(ext->scrl, style);
            break;
        case GUI_PAGE_STYLE_SB:
            ext->sb.style = style;
            area_set_height(&ext->sb.hor_area, ext->sb.style->body.padding.inner);
            area_set_width(&ext->sb.ver_area, ext->sb.style->body.padding.inner);
            page_sb_refresh(page);
            obj_refresh_ext_size(page);
            obj_invalidate(page);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the scrollable object of a page
 * @param page pointer to a page object
 * @return pointer to a container which is the scrollable part of the page
 */
obj_t * page_get_scrl(obj_t * page)
{
	page_ext_t * ext = obj_get_ext_attr(page);

	return ext->scrl;
}

/**
 * Set the scroll bar mode on a page
 * @param page pointer to a page object
 * @return the mode from 'page_sb.mode_t' enum
 */
sb_mode_t page_get_sb_mode(obj_t * page)
{
    page_ext_t * ext = obj_get_ext_attr(page);
    return (sb_mode_t)ext->sb.mode;
}

/**
 * Get a style of a page
 * @param page pointer to page object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
style_t * page_get_style(obj_t *page, page_style_t type)
{
    page_ext_t * ext = obj_get_ext_attr(page);

    switch (type) {
        case GUI_PAGE_STYLE_BG:     return obj_get_style(page);
        case GUI_PAGE_STYLE_SCRL:   return obj_get_style(ext->scrl);
        case GUI_PAGE_STYLE_SB:     return ext->sb.style;
        default: break;
    }

    /*To avoid warning*/
    return NULL;
}

/*=====================
 * Other functions
 *====================*/

/**
 * Glue the object to the page. After it the page can be moved (dragged) with this object too.
 * @param obj pointer to an object on a page
 * @param glue true: enable glue, false: disable glue
 */
void page_glue_obj(obj_t * obj, bool glue)
{
    obj_set_drag_parent(obj, glue);
    obj_set_drag(obj, glue);
}

/**
 * Focus on an object. It ensures that the object will be visible on the page.
 * @param page pointer to a page object
 * @param obj pointer to an object to focus (must be on the page)
 * @param anim_time scroll animation time in milliseconds (0: no animation)
 */
void page_focus(obj_t * page, obj_t * obj, uint16_t anim_time)
{
    page_ext_t * ext = obj_get_ext_attr(page);
    style_t * style = page_get_style(page, GUI_PAGE_STYLE_BG);
    style_t * style_scrl = page_get_style(page, GUI_PAGE_STYLE_SCRL);

    coord_t obj_y = obj->coords.y1 - ext->scrl->coords.y1;
    coord_t obj_h = obj_get_height(obj);
    coord_t scrlable_y = obj_get_y(ext->scrl);
    coord_t page_h = obj_get_height(page);

    coord_t top_err = -(scrlable_y + obj_y);
    coord_t bot_err = scrlable_y + obj_y + obj_h - page_h;

    /*If obj is higher then the page focus where the "error" is smaller*/

    /*Out of the page on the top*/
    if((obj_h <= page_h && top_err > 0) ||
       (obj_h > page_h && top_err < bot_err)) {
        /*Calculate a new position and let some space above*/
        scrlable_y = -(obj_y - style_scrl->body.padding.ver - style->body.padding.ver);
        scrlable_y += style_scrl->body.padding.ver;
    }
    /*Out of the page on the bottom*/
    else if((obj_h <= page_h && bot_err > 0) ||
            (obj_h > page_h && top_err >= bot_err)) {
        /*Calculate a new position and let some space below*/
        scrlable_y = -obj_y;
        scrlable_y += page_h - obj_h;
        scrlable_y -= style_scrl->body.padding.ver;
    } else {
        /*Already in focus*/
        return;
    }

    if(anim_time == 0) {
        obj_set_y(ext->scrl, scrlable_y);
    }
    else {
#if GUI_USE_ANIMATION
        anim_t a;
        a.act_time = 0;
        a.start = obj_get_y(ext->scrl);
        a.end = scrlable_y;
        a.time = anim_time;
        a.end_cb = NULL;
        a.playback = 0;
        a.repeat = 0;
        a.var = ext->scrl;
        a.path = anim_path_linear;
        a.fp = (anim_fp_t) obj_set_y;
        anim_create(&a);
#endif
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the pages
 * @param page pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool page_design(obj_t * page, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
    	return ancestor_design(page, mask, mode);
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
        /*Draw without border*/
        style_t *style = page_get_style(page, GUI_PAGE_STYLE_BG);
        coord_t border_width_tmp =  style->body.border.width;
        style->body.border.width = 0;
        draw_rect(&page->coords, mask, style);
        style->body.border.width = border_width_tmp;

	} else if(mode == GUI_DESIGN_DRAW_POST) { /*Draw the scroll bars finally*/

        /*Draw only a border*/
        style_t *style = page_get_style(page, GUI_PAGE_STYLE_BG);
        coord_t shadow_width_tmp =  style->body.shadow.width;
        uint8_t empty_tmp =  style->body.empty;
        style->body.shadow.width = 0;
        style->body.empty = 1;
        draw_rect(&page->coords, mask, style);
        style->body.shadow.width = shadow_width_tmp;
        style->body.empty = empty_tmp;


		page_ext_t * ext = obj_get_ext_attr(page);

		/*Draw the scrollbars*/
		area_t sb_area;
		if(ext->sb.hor_draw) {
		    /*Convert the relative coordinates to absolute*/
            area_copy(&sb_area, &ext->sb.hor_area);
		    sb_area.x1 += page->coords.x1;
            sb_area.y1 += page->coords.y1;
            sb_area.x2 += page->coords.x1;
            sb_area.y2 += page->coords.y1;
			draw_rect(&sb_area, mask, ext->sb.style);
		}

		if(ext->sb.ver_draw) {
            /*Convert the relative coordinates to absolute*/
            area_copy(&sb_area, &ext->sb.ver_area);
            sb_area.x1 += page->coords.x1;
            sb_area.y1 += page->coords.y1;
            sb_area.x2 += page->coords.x1;
            sb_area.y2 += page->coords.y1;
			draw_rect(&sb_area, mask, ext->sb.style);
		}
	}

	return true;
}

/**
 * Handle the drawing related tasks of the scrollable object
 * @param scrl pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool scrl_design(obj_t * scrl, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
        return ancestor_design(scrl, mask, mode);
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
#if GUI_USE_GROUP
        /* If the page is the active in a group and
         * the background (page) is not visible (transparent or empty)
         * then activate the style of the scrollable*/
        style_t * style_ori = obj_get_style(scrl);
        obj_t * page = obj_get_parent(scrl);
        style_t * style_page = obj_get_style(page);
        group_t * g = obj_get_group(page);
        if(style_page->body.empty != 0 || style_page->body.opa == GUI_OPA_TRANSP) { /*Background is visible?*/
            if(group_get_focused(g) == page) {
                style_t * style_mod;
                style_mod = group_mod_style(g, style_ori);
                scrl->style_p = style_mod;  /*Temporally change the style to the activated */
            }
        }
#endif
        ancestor_design(scrl, mask, mode);

#if GUI_USE_GROUP
        scrl->style_p = style_ori;  /*Revert the style*/
#endif
    } else if(mode == GUI_DESIGN_DRAW_POST) {
        ancestor_design(scrl, mask, mode);
    }

    return true;
}

/**
 * Signal function of the page
 * @param page pointer to a page object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t page_signal(obj_t * page, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(page, sign, param);
    if(res != GUI_RES_OK) return res;

    page_ext_t * ext = obj_get_ext_attr(page);
    style_t * style = obj_get_style(page);
    obj_t * child;
    if(sign == GUI_SIGNAL_CHILD_CHG) { /*Automatically move children to the scrollable object*/
        child = obj_get_child(page, NULL);
        while(child != NULL) {
            if(obj_is_protected(child, GUI_PROTECT_PARENT) == false) {
                obj_t * tmp = child;
                child = obj_get_child(page, child); /*Get the next child before move this*/
                obj_set_parent(tmp, ext->scrl);
            } else {
                child = obj_get_child(page, child);
            }
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
        /*If no hor_fit enabled set the scrollable's width to the page's width*/
        if(cont_get_hor_fit(ext->scrl) == false) {
            obj_set_width(ext->scrl, obj_get_width(page) - 2 * style->body.padding.hor);
        } else {
            ext->scrl->signal_func(ext->scrl, GUI_SIGNAL_CORD_CHG, &ext->scrl->coords);
        }

        /*The scrollbars are important only if they are visible now*/
        if(ext->sb.hor_draw || ext->sb.ver_draw) page_sb_refresh(page);

        /*Refresh the ext. size because the scrollbars might be positioned out of the page*/
        obj_refresh_ext_size(page);
    }
    else if(sign == GUI_SIGNAL_CORD_CHG) {
        /*Refresh the scrollbar and notify the scrl if the size is changed*/
        if(ext->scrl != NULL && (obj_get_width(page) != area_get_width(param) ||
                                 obj_get_height(page) != area_get_height(param)))
        {
            /*If no hor_fit enabled set the scrollable's width to the page's width*/
            if(cont_get_hor_fit(ext->scrl) == false) {
                obj_set_width(ext->scrl, obj_get_width(page) - 2 * style->body.padding.hor);
            }

            ext->scrl->signal_func(ext->scrl, GUI_SIGNAL_CORD_CHG, &ext->scrl->coords);

            /*The scrollbars are important only if they are visible now*/
            if(ext->sb.hor_draw || ext->sb.ver_draw) page_sb_refresh(page);
        }
    }
    else if(sign == GUI_SIGNAL_PRESSED) {
        if(ext->pr_action != NULL) {
            ext->pr_action(page);
        }
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        if(indev_is_dragging(indev_get_act()) == false) {
            if(ext->rel_action != NULL) {
                ext->rel_action(page);
            }
        }
    }
    else if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        /*Ensure ext. size for the scrollbars if they are out of the page*/
        if(page->ext_size < (-ext->sb.style->body.padding.hor)) page->ext_size = -ext->sb.style->body.padding.hor;
        if(page->ext_size < (-ext->sb.style->body.padding.ver)) page->ext_size = -ext->sb.style->body.padding.ver;
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "page";
    }

    return res;
}

/**
 * Signal function of the scrollable part of a page
 * @param scrl pointer to the scrollable object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t page_scrollable_signal(obj_t * scrl, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(scrl, sign, param);
    if(res != GUI_RES_OK) return res;

    obj_t * page = obj_get_parent(scrl);
    style_t * page_style = obj_get_style(page);
    page_ext_t * page_ext = obj_get_ext_attr(page);

    if(sign == GUI_SIGNAL_CORD_CHG) {
        /*Be sure the width of the scrollable is correct*/
        if(cont_get_hor_fit(scrl) == false) {
            obj_set_width(scrl, obj_get_width(page) - 2 * page_style->body.padding.hor);
        }

        /*Limit the position of the scrollable object to be always visible
         * (Do not let its edge inner then its parent respective edge)*/
        coord_t new_x;
        coord_t new_y;
        bool refr_x = false;
        bool refr_y = false;
        area_t page_cords;
        area_t scrl_cords;
        coord_t hpad = page_style->body.padding.hor;
        coord_t vpad = page_style->body.padding.ver;

        new_x = obj_get_x(scrl);
        new_y = obj_get_y(scrl);
        obj_get_coords(scrl, &scrl_cords);
        obj_get_coords(page, &page_cords);

        /*scrollable width smaller then page width? -> align to left*/
        if(area_get_width(&scrl_cords) + 2 * hpad < area_get_width(&page_cords)) {
            if(scrl_cords.x1 != page_cords.x1 + hpad) {
                new_x = hpad;
                refr_x = true;
            }
        } else {
            /*The edges of the scrollable can not be in the page (minus hpad) */
            if(scrl_cords.x2  < page_cords.x2 - hpad) {
               new_x =  area_get_width(&page_cords) - area_get_width(&scrl_cords) - hpad;   /* Right align */
               refr_x = true;
            }
            if (scrl_cords.x1 > page_cords.x1 + hpad) {
                new_x = hpad;  /*Left align*/
                refr_x = true;
            }
        }

        /*scrollable height smaller then page height? -> align to left*/
        if(area_get_height(&scrl_cords) + 2 * vpad < area_get_height(&page_cords)) {
            if(scrl_cords.y1 != page_cords.y1 + vpad) {
                new_y = vpad;
                refr_y = true;
            }
        } else {
            /*The edges of the scrollable can not be in the page (minus vpad) */
            if(scrl_cords.y2 < page_cords.y2 - vpad) {
               new_y =  area_get_height(&page_cords) - area_get_height(&scrl_cords) - vpad;   /* Bottom align */
               refr_y = true;
            }
            if (scrl_cords.y1  > page_cords.y1 + vpad) {
                new_y = vpad;  /*Top align*/
                refr_y = true;
            }
        }
        if(refr_x != false || refr_y != false) {
            obj_set_pos(scrl, new_x, new_y);
        }

        page_sb_refresh(page);
    }
    else if(sign == GUI_SIGNAL_DRAG_END) {
        /*Hide scrollbars if required*/
        if(page_ext->sb.mode == GUI_SB_MODE_DRAG) {
            area_t sb_area_tmp;
            if(page_ext->sb.hor_draw) {
                area_copy(&sb_area_tmp, &page_ext->sb.hor_area);
                sb_area_tmp.x1 += page->coords.x1;
                sb_area_tmp.y1 += page->coords.y1;
                sb_area_tmp.x2 += page->coords.x2;
                sb_area_tmp.y2 += page->coords.y2;
                inv_area(&sb_area_tmp,true);
                page_ext->sb.hor_draw = 0;
            }
            if(page_ext->sb.ver_draw)  {
                area_copy(&sb_area_tmp, &page_ext->sb.ver_area);
                sb_area_tmp.x1 += page->coords.x1;
                sb_area_tmp.y1 += page->coords.y1;
                sb_area_tmp.x2 += page->coords.x2;
                sb_area_tmp.y2 += page->coords.y2;
                inv_area(&sb_area_tmp,true);
                page_ext->sb.ver_draw = 0;
            }
        }
    }
    else if(sign == GUI_SIGNAL_PRESSED) {
        if(page_ext->pr_action != NULL) {
            page_ext->pr_action(page);
        }
    }
    else if(sign == GUI_SIGNAL_RELEASED) {
        if(indev_is_dragging(indev_get_act()) == false) {
            if(page_ext->rel_action != NULL) {
                page_ext->rel_action(page);
            }
        }
    }

    return res;
}


/**
 * Refresh the position and size of the scroll bars.
 * @param page pointer to a page object
 */
static void page_sb_refresh(obj_t * page)
{

    page_ext_t * ext = obj_get_ext_attr(page);
    style_t * style = obj_get_style(page);
    obj_t * scrl = ext->scrl;
    coord_t size_tmp;
    coord_t scrl_w = obj_get_width(scrl);
    coord_t scrl_h =  obj_get_height(scrl);
    coord_t hpad = style->body.padding.hor;
    coord_t vpad = style->body.padding.ver;
    coord_t obj_w = obj_get_width(page);
    coord_t obj_h = obj_get_height(page);

    /*Always let 'scrollbar width' padding above, under, left and right to the scrollbars
     * else:
     * - horizontal and vertical scrollbars can overlap on the corners
     * - if the page has radius the scrollbar can be out of the radius  */
    coord_t sb_hor_pad = RTE_MATH_MAX(ext->sb.style->body.padding.inner, style->body.padding.hor);
    coord_t sb_ver_pad = RTE_MATH_MAX(ext->sb.style->body.padding.inner, style->body.padding.ver);

    if(ext->sb.mode == GUI_SB_MODE_OFF) return;

    if(ext->sb.mode == GUI_SB_MODE_ON) {
        ext->sb.hor_draw = 1;
        ext->sb.ver_draw = 1;
    }

    /*Invalidate the current (old) scrollbar areas*/
    area_t sb_area_tmp;
    if(ext->sb.hor_draw != 0) {
        area_copy(&sb_area_tmp, &ext->sb.hor_area);
        sb_area_tmp.x1 += page->coords.x1;
        sb_area_tmp.y1 += page->coords.y1;
        sb_area_tmp.x2 += page->coords.x2;
        sb_area_tmp.y2 += page->coords.y2;
        inv_area(&sb_area_tmp,true);
    }
    if(ext->sb.ver_draw != 0)  {
        area_copy(&sb_area_tmp, &ext->sb.ver_area);
        sb_area_tmp.x1 += page->coords.x1;
        sb_area_tmp.y1 += page->coords.y1;
        sb_area_tmp.x2 += page->coords.x2;
        sb_area_tmp.y2 += page->coords.y2;
        inv_area(&sb_area_tmp,true);
    }


    if(ext->sb.mode == GUI_SB_MODE_DRAG && indev_is_dragging(indev_get_act()) == false) {
        ext->sb.hor_draw = 0;
        ext->sb.ver_draw = 0;
        return;

    }

    /*Horizontal scrollbar*/
    if(scrl_w <= obj_w - 2 * hpad) {        /*Full sized scroll bar*/
        area_set_width(&ext->sb.hor_area, obj_w - 2 * sb_hor_pad);
        area_set_pos(&ext->sb.hor_area, sb_hor_pad, obj_h - ext->sb.style->body.padding.inner - ext->sb.style->body.padding.ver);
        if(ext->sb.mode == GUI_SB_MODE_AUTO || ext->sb.mode == GUI_SB_MODE_DRAG)  ext->sb.hor_draw = 0;
    } else {
        size_tmp = (obj_w * (obj_w - (2 * sb_hor_pad))) / (scrl_w + 2 * hpad);
        if(size_tmp < GUI_PAGE_SB_MIN_SIZE) size_tmp = GUI_PAGE_SB_MIN_SIZE;
        area_set_width(&ext->sb.hor_area,  size_tmp);

        area_set_pos(&ext->sb.hor_area, sb_hor_pad +
                   (-(obj_get_x(scrl) - hpad) * (obj_w - size_tmp -  2 * sb_hor_pad)) /
                   (scrl_w + 2 * hpad - obj_w ),
                   obj_h - ext->sb.style->body.padding.inner - ext->sb.style->body.padding.ver);

        if(ext->sb.mode == GUI_SB_MODE_AUTO || ext->sb.mode == GUI_SB_MODE_DRAG)  ext->sb.hor_draw = 1;
    }
    
    /*Vertical scrollbar*/
    if(scrl_h <= obj_h - 2 * vpad) {        /*Full sized scroll bar*/
        area_set_height(&ext->sb.ver_area,  obj_h - 2 * sb_ver_pad);
        area_set_pos(&ext->sb.ver_area, obj_w - ext->sb.style->body.padding.inner - ext->sb.style->body.padding.hor, sb_ver_pad);
        if(ext->sb.mode == GUI_SB_MODE_AUTO || ext->sb.mode == GUI_SB_MODE_DRAG)  ext->sb.ver_draw = 0;
    } else {
        size_tmp = (obj_h * (obj_h - (2 * sb_ver_pad))) / (scrl_h + 2 * vpad);
        if(size_tmp < GUI_PAGE_SB_MIN_SIZE) size_tmp = GUI_PAGE_SB_MIN_SIZE;
        area_set_height(&ext->sb.ver_area,  size_tmp);

        area_set_pos(&ext->sb.ver_area,  obj_w - ext->sb.style->body.padding.inner - ext->sb.style->body.padding.hor,
        		    sb_ver_pad +
                   (-(obj_get_y(scrl) - vpad) * (obj_h - size_tmp -  2 * sb_ver_pad)) /
                                      (scrl_h + 2 * vpad - obj_h ));

        if(ext->sb.mode == GUI_SB_MODE_AUTO || ext->sb.mode == GUI_SB_MODE_DRAG)  ext->sb.ver_draw = 1;
    }

    /*Invalidate the new scrollbar areas*/
    if(ext->sb.hor_draw != 0) {
        area_copy(&sb_area_tmp, &ext->sb.hor_area);
        sb_area_tmp.x1 += page->coords.x1;
        sb_area_tmp.y1 += page->coords.y1;
        sb_area_tmp.x2 += page->coords.x2;
        sb_area_tmp.y2 += page->coords.y2;
        inv_area(&sb_area_tmp,true);
    }
    if(ext->sb.ver_draw != 0)  {
        area_copy(&sb_area_tmp, &ext->sb.ver_area);
        sb_area_tmp.x1 += page->coords.x1;
        sb_area_tmp.y1 += page->coords.y1;
        sb_area_tmp.x2 += page->coords.x2;
        sb_area_tmp.y2 += page->coords.y2;
        inv_area(&sb_area_tmp,true);
    }
}

#endif
