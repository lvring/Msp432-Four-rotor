/**
 * @file cont.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

#if GUI_USE_CONT != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t cont_signal(obj_t * cont, signal_t sign, void * param);
static void cont_refr_layout(obj_t * cont);
static void cont_layout_col(obj_t * cont);
static void cont_layout_row(obj_t * cont);
static void cont_layout_center(obj_t * cont);
static void cont_layout_pretty(obj_t * cont);
static void cont_layout_grid(obj_t * cont);
static void cont_refr_autofit(obj_t * cont);

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
 * Create a container objects
 * @param par pointer to an object, it will be the parent of the new container
 * @param copy pointer to a container object, if not NULL then the new object will be copied from it
 * @return pointer to the created container
 */
obj_t * cont_create(obj_t * par, obj_t * copy)
{
    /*Create a basic object*/
    obj_t * new_cont = obj_create(par, copy);
    RTE_AssertParam(new_cont);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_cont);

    obj_allocate_ext_attr(new_cont, sizeof(cont_ext_t));
    cont_ext_t * ext = obj_get_ext_attr(new_cont);
    RTE_AssertParam(ext);
    ext->hor_fit = 0;
    ext->ver_fit = 0;
    ext->layout = GUI_LAYOUT_OFF;

    obj_set_signal_func(new_cont, cont_signal);

    /*Init the new container*/
    if(copy == NULL) {
        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            cont_set_style(new_cont, th->cont);
        } else {
            cont_set_style(new_cont, &style_pretty);
        }
    }
    /*Copy an existing object*/
    else {
    	cont_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->hor_fit = copy_ext->hor_fit;
    	ext->ver_fit = copy_ext->ver_fit;
    	ext->layout = copy_ext->layout;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_cont);
    }

    return new_cont;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a layout on a container
 * @param cont pointer to a container object
 * @param layout a layout from 'cont_layout_t'
 */
void cont_set_layout(obj_t * cont, layout_t layout)
{
	cont_ext_t * ext = obj_get_ext_attr(cont);
	if(ext->layout == layout) return;

	ext->layout = layout;

	/*Send a signal to refresh the layout*/
	cont->signal_func(cont, GUI_SIGNAL_CHILD_CHG, NULL);
}


/**
 * Enable the horizontal or vertical fit.
 * The container size will be set to involve the children horizontally or vertically.
 * @param cont pointer to a container object
 * @param hor_en true: enable the horizontal fit
 * @param ver_en true: enable the vertical fit
 */
void cont_set_fit(obj_t * cont, bool hor_en, bool ver_en)
{
	obj_invalidate(cont);
	cont_ext_t * ext = obj_get_ext_attr(cont);
	if(ext->hor_fit == hor_en && ext->ver_fit == ver_en) return;

	ext->hor_fit = hor_en == false ? 0 : 1;
	ext->ver_fit = ver_en == false ? 0 : 1;

	/*Send a signal to set a new size*/
	area_t area;
	obj_get_coords(cont, &area);
	cont->signal_func(cont, GUI_SIGNAL_CORD_CHG, &area);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the layout of a container
 * @param cont pointer to container object
 * @return the layout from 'cont_layout_t'
 */
layout_t cont_get_layout(obj_t * cont)
{
	cont_ext_t * ext = obj_get_ext_attr(cont);
	return (layout_t)ext->layout;
}

/**
 * Get horizontal fit enable attribute of a container
 * @param cont pointer to a container object
 * @return true: horizontal fit is enabled; false: disabled
 */
bool cont_get_hor_fit(obj_t * cont)
{
	cont_ext_t * ext = obj_get_ext_attr(cont);
	return ext->hor_fit == 0 ? false : true;
}

/**
 * Get vertical fit enable attribute of a container
 * @param cont pointer to a container object
 * @return true: vertical fit is enabled; false: disabled
 */
bool cont_get_ver_fit(obj_t * cont)
{
	cont_ext_t * ext = obj_get_ext_attr(cont);
	return ext->ver_fit == 0 ? false : true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the container
 * @param cont pointer to a container object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t cont_signal(obj_t * cont, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(cont, sign, param);
    if(res != GUI_RES_OK) return res;

    if(sign == GUI_SIGNAL_STYLE_CHG) { /*Recalculate the padding if the style changed*/
        cont_refr_layout(cont);
        cont_refr_autofit(cont);
    } else if(sign == GUI_SIGNAL_CHILD_CHG) {
        cont_refr_layout(cont);
        cont_refr_autofit(cont);
    } else if(sign == GUI_SIGNAL_CORD_CHG) {
        if(obj_get_width(cont) != area_get_width(param) ||
           obj_get_height(cont) != area_get_height(param)) {
            cont_refr_layout(cont);
            cont_refr_autofit(cont);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "cont";
    }

    return res;
}


/**
 * Refresh the layout of a container
 * @param cont pointer to an object which layout should be refreshed
 */
static void cont_refr_layout(obj_t * cont)
{
	layout_t type = cont_get_layout(cont);

	/*'cont' has to be at least 1 child*/
	if(obj_get_child(cont, NULL) == NULL) return;

	if(type == GUI_LAYOUT_OFF) return;

	if(type == GUI_LAYOUT_CENTER) {
		cont_layout_center(cont);
	} else if(type == GUI_LAYOUT_COL_L || type == GUI_LAYOUT_COL_M || type == GUI_LAYOUT_COL_R) {
		cont_layout_col(cont);
	} else if(type == GUI_LAYOUT_ROW_T || type == GUI_LAYOUT_ROW_M || type == GUI_LAYOUT_ROW_B) {
		cont_layout_row(cont);
	} else if(type == GUI_LAYOUT_PRETTY) {
		cont_layout_pretty(cont);
	}  else if(type == GUI_LAYOUT_GRID) {
		cont_layout_grid(cont);
	}
}

/**
 * Handle column type layouts
 * @param cont pointer to an object which layout should be handled
 */
static void cont_layout_col(obj_t * cont)
{
	layout_t type = cont_get_layout(cont);
	obj_t * child;

	/*Adjust margin and get the alignment type*/
	align_t align;
	style_t * style = obj_get_style(cont);
	coord_t hpad_corr;

	switch(type) {
		case GUI_LAYOUT_COL_L:
            hpad_corr = style->body.padding.hor;
			align = GUI_ALIGN_IN_TOP_LEFT;
			break;
		case GUI_LAYOUT_COL_M:
			hpad_corr = 0;
			align = GUI_ALIGN_IN_TOP_MID;
			break;
		case GUI_LAYOUT_COL_R:
			hpad_corr = -style->body.padding.hor;
			align = GUI_ALIGN_IN_TOP_RIGHT;
			break;
		default:
			hpad_corr = 0;
			align = GUI_ALIGN_IN_TOP_LEFT;
			break;
	}

	/* Disable child change action because the children will be moved a lot
	 * an unnecessary child change signals could be sent*/
	obj_set_protect(cont, GUI_PROTECT_CHILD_CHG);
	/* Align the children */
	coord_t last_cord = style->body.padding.ver;
	LL_READ_BACK(cont->child_ll, child) {
        if(obj_get_hidden(child) != false ||
           obj_is_protected(child, GUI_PROTECT_POS) != false) continue;

		obj_align(child, cont, align, hpad_corr , last_cord);
		last_cord += obj_get_height(child) + style->body.padding.inner;
	}

    obj_clear_protect(cont, GUI_PROTECT_CHILD_CHG);
}

/**
 * Handle row type layouts
 * @param cont pointer to an object which layout should be handled
 */
static void cont_layout_row(obj_t * cont)
{
	layout_t type = cont_get_layout(cont);
	obj_t * child;

	/*Adjust margin and get the alignment type*/
	align_t align;
	style_t * style = obj_get_style(cont);
	coord_t vpad_corr = style->body.padding.ver;

	switch(type) {
		case GUI_LAYOUT_ROW_T:
			vpad_corr = style->body.padding.ver;
			align = GUI_ALIGN_IN_TOP_LEFT;
			break;
		case GUI_LAYOUT_ROW_M:
			vpad_corr = 0;
			align = GUI_ALIGN_IN_LEFT_MID;
			break;
		case GUI_LAYOUT_ROW_B:
			vpad_corr = -style->body.padding.ver;
			align = GUI_ALIGN_IN_BOTTOM_LEFT;
			break;
		default:
			vpad_corr = 0;
			align = GUI_ALIGN_IN_TOP_LEFT;
			break;
	}

	/* Disable child change action because the children will be moved a lot
	 * an unnecessary child change signals could be sent*/
    obj_set_protect(cont, GUI_PROTECT_CHILD_CHG);

	/* Align the children */
	coord_t last_cord = style->body.padding.hor;
	LL_READ_BACK(cont->child_ll, child) {
		if(obj_get_hidden(child) != false ||
           obj_is_protected(child, GUI_PROTECT_POS) != false) continue;

		obj_align(child, cont, align, last_cord, vpad_corr);
		last_cord += obj_get_width(child) + style->body.padding.inner;
	}

    obj_clear_protect(cont, GUI_PROTECT_CHILD_CHG);
}

/**
 * Handle the center layout
 * @param cont pointer to an object which layout should be handled
 */
static void cont_layout_center(obj_t * cont)
{
	obj_t * child;
	style_t * style = obj_get_style(cont);
	uint32_t obj_num = 0;
	coord_t h_tot = 0;

	LL_READ(cont->child_ll, child) {
        if(obj_get_hidden(child) != false ||
           obj_is_protected(child, GUI_PROTECT_POS) != false) continue;
		h_tot += obj_get_height(child) + style->body.padding.inner;
		obj_num ++;
	}

	if(obj_num == 0) return;

	h_tot -= style->body.padding.inner;

	/* Disable child change action because the children will be moved a lot
	 * an unnecessary child change signals could be sent*/
    obj_set_protect(cont, GUI_PROTECT_CHILD_CHG);

	/* Align the children */
	coord_t last_cord = - (h_tot / 2);
	LL_READ_BACK(cont->child_ll, child) {
        if(obj_get_hidden(child) != false ||
           obj_is_protected(child, GUI_PROTECT_POS) != false) continue;

		obj_align(child, cont, GUI_ALIGN_CENTER, 0, last_cord + obj_get_height(child) / 2);
		last_cord += obj_get_height(child) + style->body.padding.inner;
	}

    obj_clear_protect(cont, GUI_PROTECT_CHILD_CHG);
}

/**
 * Handle the pretty layout. Put as many object as possible in row
 * then begin a new row
 * @param cont pointer to an object which layout should be handled
 */
static void cont_layout_pretty(obj_t * cont)
{
	obj_t * child_rs;    /* Row starter child */
	obj_t * child_rc;    /* Row closer child */
	obj_t * child_tmp;   /* Temporary child */
	style_t * style = obj_get_style(cont);
	coord_t w_obj = obj_get_width(cont);
	coord_t act_y = style->body.padding.ver;
	/* Disable child change action because the children will be moved a lot
	 * an unnecessary child change signals could be sent*/

	child_rs = ll_get_tail(&cont->child_ll); /*Set the row starter child*/
	if(child_rs == NULL) return;	/*Return if no child*/

    obj_set_protect(cont, GUI_PROTECT_CHILD_CHG);

	child_rc = child_rs; /*Initially the the row starter and closer is the same*/
	while(child_rs != NULL) {
		coord_t h_row = 0;
		coord_t w_row = style->body.padding.hor * 2; /*The width is at least the left+right hpad*/
		uint32_t obj_num = 0;

		/*Find the row closer object and collect some data*/
		do {
			if(obj_get_hidden(child_rc) == false &&
			   obj_is_protected(child_rc, GUI_PROTECT_POS) == false) {
			    /*If this object is already not fit then break*/
				if(w_row + obj_get_width(child_rc) > w_obj) {
			        /*Step back one child because the last already not fit, so the previous is the closer*/
			        if(child_rc != NULL  && obj_num != 0 ) {
			            child_rc = ll_get_next(&cont->child_ll, child_rc);
			        }
				    break;
				}
				w_row += obj_get_width(child_rc) + style->body.padding.inner; /*Add the object width + opad*/
				h_row = RTE_MATH_MAX(h_row, obj_get_height(child_rc)); /*Search the highest object*/
				obj_num ++;
				if(obj_is_protected(child_rc, GUI_PROTECT_FOLLOW)) break; /*If can not be followed by an other object then break here*/

			}
			child_rc = ll_get_prev(&cont->child_ll, child_rc); /*Load the next object*/
			if(obj_num == 0) child_rs = child_rc; /*If the first object was hidden (or too long) then set the next as first */
		}while(child_rc != NULL);

		/*If the object is too long  then align it to the middle*/
		if(obj_num == 0) {
			if(child_rc != NULL) {
				obj_align(child_rc, cont, GUI_ALIGN_IN_TOP_MID, 0, act_y);
				h_row = obj_get_height(child_rc);    /*Not set previously because of the early break*/
			}
		}
		/*If there is only one object in the row then align it to the middle*/
		else if (obj_num == 1) {
			obj_align(child_rs, cont, GUI_ALIGN_IN_TOP_MID, 0, act_y);
		}
        /*If there are two object in the row then align them proportionally*/
        else if (obj_num == 2) {
            obj_t * obj1 = child_rs;
            obj_t * obj2 = ll_get_prev(&cont->child_ll, child_rs);
            w_row = obj_get_width(obj1) + obj_get_width(obj2);
            coord_t pad = (w_obj - w_row) / 3;
            obj_align(obj1, cont, GUI_ALIGN_IN_TOP_LEFT, pad, act_y + (h_row - obj_get_height(obj1)) / 2);
            obj_align(obj2, cont, GUI_ALIGN_IN_TOP_RIGHT, -pad, act_y + (h_row - obj_get_height(obj2)) / 2);
        }
		/* Align the children (from child_rs to child_rc)*/
		else {
			w_row -= style->body.padding.inner * obj_num;
			coord_t new_opad = (w_obj -  w_row) / (obj_num  - 1);
			coord_t act_x = style->body.padding.hor; /*x init*/
			child_tmp = child_rs;
			while(child_tmp != NULL) {
				if(obj_get_hidden(child_tmp) == false &&
				   obj_is_protected(child_tmp, GUI_PROTECT_POS) == false) {
					obj_align(child_tmp, cont, GUI_ALIGN_IN_TOP_LEFT, act_x, act_y + (h_row - obj_get_height(child_tmp)) / 2);
					act_x += obj_get_width(child_tmp) + new_opad;
				}
				if(child_tmp == child_rc) break;
				child_tmp = ll_get_prev(&cont->child_ll, child_tmp);
			}

		}

		if(child_rc == NULL) break;
		act_y += style->body.padding.inner + h_row; /*y increment*/
		child_rs = ll_get_prev(&cont->child_ll, child_rc); /*Go to the next object*/
		child_rc = child_rs;
	}
    obj_clear_protect(cont, GUI_PROTECT_CHILD_CHG);
}

/**
 * Handle the grid layout. Align same-sized objects in a grid
 * @param cont pointer to an object which layout should be handled
 */
static void cont_layout_grid(obj_t * cont)
{
	obj_t * child;
	style_t * style = obj_get_style(cont);
	coord_t w_tot = obj_get_width(cont);
	coord_t w_obj = obj_get_width(obj_get_child(cont, NULL));
	coord_t h_obj = obj_get_height(obj_get_child(cont, NULL));
	uint16_t obj_row = (w_tot - (2 * style->body.padding.hor)) / (w_obj + style->body.padding.inner); /*Obj. num. in a row*/
	coord_t x_ofs;
	if(obj_row > 1) {
		x_ofs = w_obj + (w_tot - (2 * style->body.padding.hor) - (obj_row * w_obj)) / (obj_row - 1);
	} else {
		x_ofs = w_tot / 2 - w_obj / 2;
	}
	coord_t y_ofs = h_obj + style->body.padding.inner;

	/* Disable child change action because the children will be moved a lot
	 * an unnecessary child change signals could be sent*/
    obj_set_protect(cont, GUI_PROTECT_CHILD_CHG);

	/* Align the children */
	coord_t act_x = style->body.padding.hor;
	coord_t act_y = style->body.padding.ver;
	uint16_t obj_cnt = 0;
	LL_READ_BACK(cont->child_ll, child) {
        if(obj_get_hidden(child) != false ||
           obj_is_protected(child, GUI_PROTECT_POS) != false) continue;

		if(obj_row > 1) {
			obj_set_pos(child, act_x, act_y);
			act_x += x_ofs;
		} else {
			obj_set_pos(child, x_ofs, act_y);
		}
		obj_cnt ++;

		if(obj_cnt >= obj_row) {
			obj_cnt = 0;
			act_x = style->body.padding.hor;
			act_y += y_ofs;
		}
	}

    obj_clear_protect(cont, GUI_PROTECT_CHILD_CHG);
}

/**
 * Handle auto fit. Set the size of the object to involve all children.
 * @param cont pointer to an object which size will be modified
 */
static void cont_refr_autofit(obj_t * cont)
{
	cont_ext_t * ext = obj_get_ext_attr(cont);

	if(ext->hor_fit == 0 &&
	   ext->ver_fit == 0) {
		return;
	}

	area_t new_cords;
	area_t ori;
	style_t * style = obj_get_style(cont);
	obj_t * i;
	coord_t hpad = style->body.padding.hor;
	coord_t vpad = style->body.padding.ver;

	/*Search the side coordinates of the children*/
	obj_get_coords(cont, &ori);
	obj_get_coords(cont, &new_cords);

	new_cords.x1 = GUI_COORD_MAX;
	new_cords.y1 = GUI_COORD_MAX;
	new_cords.x2 = GUI_COORD_MIN;
	new_cords.y2 = GUI_COORD_MIN;

    LL_READ(cont->child_ll, i) {
		if(obj_get_hidden(i) != false) continue;
    	new_cords.x1 = RTE_MATH_MIN(new_cords.x1, i->coords.x1);
    	new_cords.y1 = RTE_MATH_MIN(new_cords.y1, i->coords.y1);
        new_cords.x2 = RTE_MATH_MAX(new_cords.x2, i->coords.x2);
        new_cords.y2 = RTE_MATH_MAX(new_cords.y2, i->coords.y2);
    }

    /*If the value is not the init value then the page has >=1 child.*/
    if(new_cords.x1 != GUI_COORD_MAX) {
    	if(ext->hor_fit != 0) {
			new_cords.x1 -= hpad;
			new_cords.x2 += hpad;
    	} else {
    		new_cords.x1 = cont->coords.x1;
    		new_cords.x2 = cont->coords.x2;
    	}
    	if(ext->ver_fit != 0) {
			new_cords.y1 -= vpad;
			new_cords.y2 += vpad;
    	} else {
    		new_cords.y1 = cont->coords.y1;
    		new_cords.y2 = cont->coords.y2;
    	}

    	/*Do nothing if the coordinates are not changed*/
    	if(cont->coords.x1 != new_cords.x1 ||
    	   cont->coords.y1 != new_cords.y1 ||
           cont->coords.x2 != new_cords.x2 ||
           cont->coords.y2 != new_cords.y2) {

            obj_invalidate(cont);
            area_copy(&cont->coords, &new_cords);
            obj_invalidate(cont);

            /*Notify the object about its new coordinates*/
            cont->signal_func(cont, GUI_SIGNAL_CORD_CHG, &ori);

            /*Inform the parent about the new coordinates*/
            obj_t * par = obj_get_parent(cont);
            par->signal_func(par, GUI_SIGNAL_CHILD_CHG, cont);
    	}
    }
}

#endif
