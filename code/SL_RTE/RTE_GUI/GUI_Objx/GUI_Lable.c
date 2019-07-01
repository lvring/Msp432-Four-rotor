/**
 * @file rect.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LABEL != 0

/*********************
 *      DEFINES
 *********************/
/*Test configurations*/
#ifndef GUI_LABEL_SCROLL_SPEED
#define GUI_LABEL_SCROLL_SPEED       (25) /*Hor, or ver. scroll speed (px/sec) in 'GUI_LABEL_LONG_SCROLL/ROLL' mode*/
#endif

#define ANIM_WAIT_CHAR_COUNT 3

#define GUI_LABEL_DOT_END_INV 0xFFFF

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static res_t label_signal(obj_t * label, signal_t sign, void * param);
static bool label_design(obj_t * label, const area_t * mask, design_mode_t mode);
static void label_refr_text(obj_t * label);
static void label_revert_dots(obj_t *label);

#if GUI_USE_ANIMATION
static void label_set_offset_x(obj_t * label, coord_t x);
static void label_set_offset_y(obj_t * label, coord_t y);
#endif
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
 * Create a label objects
 * @param par pointer to an object, it will be the parent of the new label
 * @param copy pointer to a button object, if not NULL then the new object will be copied from it
 * @return pointer to the created button
 */
obj_t * label_create(obj_t * par, obj_t * copy)
{
    /*Create a basic object*/
    obj_t * new_label = obj_create(par, copy);
    RTE_AssertParam(new_label);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_label);
    
    /*Extend the basic object to a label object*/
    obj_allocate_ext_attr(new_label, sizeof(label_ext_t));
    
    label_ext_t * ext = obj_get_ext_attr(new_label);
    RTE_AssertParam(ext);
    ext->text = NULL;
    ext->static_txt = 0;
    ext->recolor = 0;
    ext->no_break = 0;
    ext->body_draw = 0;
    ext->align = GUI_LABEL_ALIGN_LEFT;
    ext->dot_end = GUI_LABEL_DOT_END_INV;
    ext->long_mode = GUI_LABEL_LONG_EXPAND;
    ext->anim_speed = GUI_LABEL_SCROLL_SPEED;
    ext->offset.x = 0;
    ext->offset.y = 0;
	obj_set_design_func(new_label, label_design);
	obj_set_signal_func(new_label, label_signal);

    /*Init the new label*/
    if(copy == NULL) {
		obj_set_click(new_label, false);
		label_set_long_mode(new_label, GUI_LABEL_LONG_EXPAND);
		label_set_text(new_label, "Text");
        label_set_style(new_label, NULL);        /*Inherit parent's style*/
    }
    /*Copy 'copy' if not NULL*/
    else {
        label_ext_t * copy_ext = obj_get_ext_attr(copy);
        label_set_long_mode(new_label, label_get_long_mode(copy));
        label_set_recolor(new_label, label_get_recolor(copy));
        label_set_body_draw(new_label, label_get_body_draw(copy));
        label_set_align(new_label, label_get_align(copy));
        if(copy_ext->static_txt == 0) label_set_text(new_label, label_get_text(copy));
        else label_set_static_text(new_label, label_get_text(copy));

        /*In DOT mode save the text byte-to-byte because a '\0' can be in the middle*/
        if(copy_ext->long_mode == GUI_LABEL_LONG_DOT) {
            ext->text = RTE_BGetr(MEM_RTE,ext->text, RTE_BGet_BufSize(copy_ext->text));
            memcpy(ext->text, copy_ext->text, RTE_BGet_BufSize(copy_ext->text));
        }

        memcpy(ext->dot_tmp, copy_ext->dot_tmp, sizeof(ext->dot_tmp));
        ext->dot_end = copy_ext->dot_end;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_label);
    }
    return new_label;
}

/*=====================
 * Setter functions 
 *====================*/

/**
 * Set a new text for a label. Memory will be allocated to store the text by the label.
 * @param label pointer to a label object
 * @param text '\0' terminated character string. NULL to refresh with the current text.
 */
void label_set_text(obj_t * label, const char * text)
{
    obj_invalidate(label);
    
    label_ext_t * ext = obj_get_ext_attr(label);

    /*If text is NULL then refresh */
    if(text == NULL) {
        label_refr_text(label);
        return;
    }

    if(ext->text == text) {
        /*If set its own text then reallocate it (maybe its size changed)*/
        ext->text = RTE_BGetr(MEM_RTE,ext->text, strlen(ext->text) + 1);
    } else {
        /*Allocate space for the new text*/
        uint32_t len = strlen(text) + 1;
        if(ext->text != NULL && ext->static_txt == 0) {
            RTE_BRel(MEM_RTE,(ext->text));
            ext->text = NULL;
        }

        ext->text = RTE_BGet(MEM_RTE,(len));
        strcpy(ext->text, text);
        ext->static_txt = 0;    /*Now the text is dynamically allocated*/
    }

    label_refr_text(label);
}
/**
 * Set a new text for a label from a character array. The array don't has to be '\0' terminated.
 * Memory will be allocated to store the array by the label.
 * @param label pointer to a label object
 * @param array array of characters or NULL to refresh the label
 * @param size the size of 'array' in bytes
 */
void label_set_array_text(obj_t * label, const char * array, uint16_t size)
{
    obj_invalidate(label);

    label_ext_t * ext = obj_get_ext_attr(label);

    /*If trying to set its own text or the array is NULL then refresh */
    if(array == ext->text || array == NULL) {
        label_refr_text(label);
        return;
    }

    /*Allocate space for the new text*/
    if(ext->text != NULL && ext->static_txt == 0) {
        RTE_BRel(MEM_RTE,(ext->text));
        ext->text = NULL;
    }
    ext->text = RTE_BGet(MEM_RTE,(size + 1));
    memcpy(ext->text, array, size);
    ext->text[size] = '\0';
    ext->static_txt = 0;    /*Now the text is dynamically allocated*/

    label_refr_text(label);
}

/**
 * Set a static text. It will not be saved by the label so the 'text' variable
 * has to be 'alive' while the label exist.
 * @param label pointer to a label object
 * @param text pointer to a text. NULL to refresh with the current text.
 */
void label_set_static_text(obj_t * label, const char * text)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    if(ext->static_txt == 0 && ext->text != NULL) {
        RTE_BRel(MEM_RTE,(ext->text));
        ext->text = NULL;
    }

    if(text != NULL) {
        ext->static_txt = 1;
        ext->text = (char *) text;
    }

    label_refr_text(label);
}

/**
 * Set the behavior of the label with longer text then the object size
 * @param label pointer to a label object
 * @param long_mode the new mode from 'label_long_mode' enum.
 */
void label_set_long_mode(obj_t * label, label_long_mode_t long_mode)
{
    label_ext_t * ext = obj_get_ext_attr(label);

#if GUI_USE_ANIMATION
    /*Delete the old animation (if exists)*/
    anim_del(label, (anim_fp_t) obj_set_x);
    anim_del(label, (anim_fp_t) obj_set_y);
    anim_del(label, (anim_fp_t) label_set_offset_x);
    anim_del(label, (anim_fp_t) label_set_offset_y);
#endif
    ext->offset.x = 0;
    ext->offset.y = 0;

    if(long_mode == GUI_LABEL_LONG_ROLL) ext->expand = 1;
    else ext->expand = 0;

    /*Restore the character under the dots*/
    if(ext->long_mode == GUI_LABEL_LONG_DOT && ext->dot_end != GUI_LABEL_DOT_END_INV) {
        label_revert_dots(label);
    }

    ext->long_mode = long_mode;
    label_refr_text(label);
}

/**
 * Set the align of the label (left or center)
 * @param label pointer to a label object
 * @param align 'GUI_LABEL_ALIGN_LEFT' or 'GUI_LABEL_ALIGN_LEFT'
 */
void label_set_align(obj_t *label, label_align_t align)
{
    label_ext_t *ext = obj_get_ext_attr(label);
    if(ext->align == align) return;

    ext->align = align;

    obj_invalidate(label);       /*Enough to invalidate because alignment is only drawing related (refr_label_text() not required)*/

}

/**
 * Enable the recoloring by in-line commands
 * @param label pointer to a label object
 * @param recolor_en true: enable recoloring, false: disable
 */
void label_set_recolor(obj_t * label, bool recolor_en)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    if(ext->recolor == recolor_en) return;

    ext->recolor = recolor_en == false ? 0 : 1;

    label_refr_text(label);  /*Refresh the text because the potential colo codes in text needs to be hided or revealed*/
}

/**
 * Set the label to ignore (or accept) line breaks on '\n'
 * @param label pointer to a label object
 * @param no_break_en true: ignore line breaks, false: make line breaks on '\n'
 */
void label_set_no_break(obj_t * label, bool no_break_en)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    if(ext->no_break == no_break_en) return;

    ext->no_break = no_break_en == false ? 0 : 1;

    label_refr_text(label);
}

/**
 * Set the label to draw (or not draw) background specified in its style's body
 * @param label pointer to a label object
 * @param body_en true: draw body; false: don't draw body
 */
void label_set_body_draw(obj_t *label, bool body_en)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    if(ext->body_draw == body_en) return;

    ext->body_draw = body_en == false ? 0 : 1;

    obj_refresh_ext_size(label);

    obj_invalidate(label);
}

/**
 * Set the label's animation speed in GUI_LABEL_LONG_ROLL and SCROLL modes
 * @param label pointer to a label object
 * @param anim_speed speed of animation in px/sec unit
 */
void label_set_anim_speed(obj_t *label, uint16_t anim_speed)
{
    label_ext_t *ext = obj_get_ext_attr(label);
    if(ext->anim_speed == anim_speed) return;

    ext->anim_speed = anim_speed;

    if(ext->long_mode == GUI_LABEL_LONG_ROLL || ext->long_mode == GUI_LABEL_LONG_SCROLL) {
        label_refr_text(label);
    }
}

/*=====================
 * Getter functions 
 *====================*/

/**
 * Get the text of a label
 * @param label pointer to a label object
 * @return the text of the label
 */
char * label_get_text(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    
    return ext->text;
}

/**
 * Get the long mode of a label
 * @param label pointer to a label object
 * @return the long mode
 */
label_long_mode_t label_get_long_mode(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    return ext->long_mode;
}

/**
 * Get the align attribute
 * @param label pointer to a label object
 * @return GUI_LABEL_ALIGN_LEFT or GUI_LABEL_ALIGN_CENTER
 */
label_align_t label_get_align(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    return (label_align_t)ext->align;
}

/**
 * Get the recoloring attribute
 * @param label pointer to a label object
 * @return true: recoloring is enabled, false: disable
 */
bool label_get_recolor(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    return ext->recolor == 0 ? false : true;
}

/**
 * Get the no break attribute
 * @param label pointer to a label object
 * @return true: no_break_enabled (ignore '\n' line breaks); false: make line breaks on '\n'
 */
bool label_get_no_break(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    return ext->no_break == 0 ? false : true;
}

/**
 * Get the body draw attribute
 * @param label pointer to a label object
 * @return true: draw body; false: don't draw body
 */
bool label_get_body_draw(obj_t *label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    return ext->body_draw == 0 ? false : true;
}

/**
 * Get the label's animation speed in GUI_LABEL_LONG_ROLL and SCROLL modes
 * @param label pointer to a label object
 * @return speed of animation in px/sec unit
 */
uint16_t label_get_anim_speed(obj_t *label)
{
    label_ext_t *ext = obj_get_ext_attr(label);
    return ext->anim_speed;
}

/**
 * Get the relative x and y coordinates of a letter
 * @param label pointer to a label object
 * @param index index of the letter [0 ... text length]. Expressed in character index, not byte index (different in UTF-8)
 * @param pos store the result here (E.g. index = 0 gives 0;0 coordinates)
 */
void label_get_letter_pos(obj_t * label, uint16_t index, point_t * pos)
{
	const char * txt = label_get_text(label);
    label_ext_t * ext = obj_get_ext_attr(label);
    uint32_t line_start = 0;
    uint32_t new_line_start = 0;
    coord_t max_w = obj_get_width(label);
    style_t * style = obj_get_style(label);
    const font_t * font = style->text.font;
    uint8_t letter_height = font_get_height(font);
    coord_t y = 0;
    txt_flag_t flag = GUI_TXT_FLAG_NONE;

    if(ext->recolor != 0) flag |= GUI_TXT_FLAG_RECOLOR;
    if(ext->expand != 0) flag |= GUI_TXT_FLAG_EXPAND;
    if(ext->no_break != 0) flag |= GUI_TXT_FLAG_NO_BREAK;
    if(ext->align == GUI_LABEL_ALIGN_CENTER) flag |= GUI_TXT_FLAG_CENTER;

    /*If the width will be expanded  the set the max length to very big */
    if(ext->long_mode == GUI_LABEL_LONG_EXPAND || ext->long_mode == GUI_LABEL_LONG_SCROLL) {
        max_w = GUI_COORD_MAX;
    }

    index = txt_utf8_get_byte_id(txt, index);

    /*Search the line of the index letter */;
    while (txt[new_line_start] != '\0') {
        new_line_start += txt_get_next_line(&txt[line_start], font, style->text.letter_space, max_w, flag);
        if(index < new_line_start || txt[new_line_start] == '\0') break; /*The line of 'index' letter begins at 'line_start'*/

        y += letter_height + style->text.line_space;
        line_start = new_line_start;
    }

    /*If the last character is line break then go to the next line*/
    if((txt[index - 1] == '\n' || txt[index - 1] == '\r') && txt[index] == '\0') {
        y += letter_height + style->text.line_space;
        line_start = index;
    }

    /*Calculate the x coordinate*/
    coord_t x = 0;
	uint32_t i = line_start;
    uint32_t cnt = line_start;                      /*Count the letter (in UTF-8 1 letter not 1 byte)*/
	txt_cmd_state_t cmd_state = GUI_TXT_CMD_STATE_WAIT;
	uint32_t letter;
	while(cnt < index) {
        cnt += txt_utf8_size(txt[i]);
	    letter = txt_utf8_next(txt, &i);
        /*Handle the recolor command*/
        if((flag & GUI_TXT_FLAG_RECOLOR) != 0) {
            if(txt_is_cmd(&cmd_state, txt[i]) != false) {
                continue; /*Skip the letter is it is part of a command*/
            }
        }
        x += font_get_width(font, letter) + style->text.letter_space;
	}

	if(ext->align == GUI_LABEL_ALIGN_CENTER) {
		coord_t line_w;
        line_w = txt_get_width(&txt[line_start], new_line_start - line_start,
                               font, style->text.letter_space, flag);
		x += obj_get_width(label) / 2 - line_w / 2;
    }

    pos->x = x;
    pos->y = y;

}

/**
 * Get the index of letter on a relative point of a label
 * @param label pointer to label object
 * @param pos pointer to point with coordinates on a the label
 * @return the index of the letter on the 'pos_p' point (E.g. on 0;0 is the 0. letter)
 * Expressed in character index and not byte index (different in UTF-8)
 */
uint16_t label_get_letter_on(obj_t * label, point_t * pos)
{
	const char * txt = label_get_text(label);
    label_ext_t * ext = obj_get_ext_attr(label);
    uint32_t line_start = 0;
    uint32_t new_line_start = 0;
    coord_t max_w = obj_get_width(label);
    style_t * style = obj_get_style(label);
    const font_t * font = style->text.font;
    uint8_t letter_height = font_get_height(font);
    coord_t y = 0;
    txt_flag_t flag = GUI_TXT_FLAG_NONE;

    if(ext->recolor != 0) flag |= GUI_TXT_FLAG_RECOLOR;
    if(ext->expand != 0) flag |= GUI_TXT_FLAG_EXPAND;
    if(ext->no_break != 0) flag |= GUI_TXT_FLAG_NO_BREAK;
    if(ext->align == GUI_LABEL_ALIGN_CENTER) flag |= GUI_TXT_FLAG_CENTER;

    /*If the width will be expanded set the max length to very big */
    if(ext->long_mode == GUI_LABEL_LONG_EXPAND || ext->long_mode == GUI_LABEL_LONG_SCROLL) {
        max_w = GUI_COORD_MAX;
    }

    /*Search the line of the index letter */;
    while (txt[line_start] != '\0') {
    	new_line_start += txt_get_next_line(&txt[line_start], font, style->text.letter_space, max_w, flag);
    	if(pos->y <= y + letter_height) break; /*The line is found (stored in 'line_start')*/
    	y += letter_height + style->text.line_space;
        line_start = new_line_start;
    }

    /*Calculate the x coordinate*/
    coord_t x = 0;
	if(ext->align == GUI_LABEL_ALIGN_CENTER) {
		coord_t line_w;
        line_w = txt_get_width(&txt[line_start], new_line_start - line_start,
                               font, style->text.letter_space, flag);
		x += obj_get_width(label) / 2 - line_w / 2;
    }

	txt_cmd_state_t cmd_state = GUI_TXT_CMD_STATE_WAIT;
	uint32_t i = line_start;
    uint32_t i_current = i;
	uint32_t letter;
	while(i < new_line_start - 1) {
	    letter = txt_utf8_next(txt, &i);    /*Be careful 'i' already points to the next character*/
	    /*Handle the recolor command*/
	    if((flag & GUI_TXT_FLAG_RECOLOR) != 0) {
            if(txt_is_cmd(&cmd_state, txt[i]) != false) {
                continue; /*Skip the letter is it is part of a command*/
            }
	    }

	    x += font_get_width(font, letter);
		if(pos->x < x) {
		    i = i_current;
		    break;
		}
		x += style->text.letter_space;
		i_current = i;
	}

	return txt_utf8_get_char_id(txt, i);
}


/*=====================
 * Other functions
 *====================*/

/**
 * Insert a text to the label. The label text can not be static.
 * @param label pointer to a label object
 * @param pos character index to insert. Expressed in character index and not byte index (Different in UTF-8)
 *            0: before first char.
 *            GUI_LABEL_POS_LAST: after last char.
 * @param txt pointer to the text to insert
 */
void label_ins_text(obj_t * label, uint32_t pos,  const char * txt)
{
    label_ext_t * ext = obj_get_ext_attr(label);

    /*Can not append to static text*/
    if(ext->static_txt != 0) return;

    obj_invalidate(label);

    /*Allocate space for the new text*/
    uint32_t old_len = strlen(ext->text);
    uint32_t ins_len = strlen(txt);
    uint32_t new_len = ins_len + old_len;
    ext->text = RTE_BGetr(MEM_RTE,ext->text, new_len + 1);

    if(pos == GUI_LABEL_POS_LAST) {
#if GUI_TXT_UTF8 == 0
        pos = old_len;
#else
        pos = txt_get_length(ext->text);
#endif
    }

    txt_ins(ext->text, pos, txt);

    label_refr_text(label);
}

/**
 * Delete characters from a label. The label text can not be static.
 * @param label pointer to a label object
 * @param pos character index to insert. Expressed in character index and not byte index (Different in UTF-8)
 *            0: before first char.
 * @param cnt number of characters to cut
 */
void label_cut_text(obj_t * label, uint32_t pos,  uint32_t cnt)
{
    label_ext_t * ext = obj_get_ext_attr(label);

    /*Can not append to static text*/
    if(ext->static_txt != 0) return;

    obj_invalidate(label);

    char * label_txt = label_get_text(label);
    /*Delete the characters*/
    txt_cut(label_txt, pos, cnt);

    /*Refresh the label*/
    label_refr_text(label);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the labels
 * @param label pointer to a label object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool label_design(obj_t * label, const area_t * mask, design_mode_t mode)
{
    /* A label never covers an area */
    if(mode == GUI_DESIGN_COVER_CHK) return false;
    else if(mode == GUI_DESIGN_DRAW_MAIN) {
        area_t coords;
        style_t * style = obj_get_style(label);
        obj_get_coords(label, &coords);

#if GUI_USE_GROUP
        group_t * g = obj_get_group(label);
        if(group_get_focused(g) == label) {
            draw_rect(&coords, mask, style);
        }
#endif

        label_ext_t * ext = obj_get_ext_attr(label);

        if(ext->body_draw) {
            area_t bg;
            obj_get_coords(label, &bg);
            bg.x1 -= style->body.padding.hor;
            bg.x2 += style->body.padding.hor;
            bg.y1 -= style->body.padding.ver;
            bg.y2 += style->body.padding.ver;

            draw_rect(&bg, mask, style);
        }

        /*TEST: draw a background for the label*/
//		draw_rect(&label->coords, mask, &style_plain_color);

		txt_flag_t flag = GUI_TXT_FLAG_NONE;
		if(ext->recolor != 0) flag |= GUI_TXT_FLAG_RECOLOR;
        if(ext->expand != 0) flag |= GUI_TXT_FLAG_EXPAND;
        if(ext->no_break != 0) flag |= GUI_TXT_FLAG_NO_BREAK;
        if(ext->align == GUI_LABEL_ALIGN_CENTER) flag |= GUI_TXT_FLAG_CENTER;

		draw_label(&coords, mask, style, ext->text, flag, &ext->offset);
    }
    return true;
}



/**
 * Signal function of the label
 * @param label pointer to a label object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t label_signal(obj_t * label, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(label, sign, param);
    if(res != GUI_RES_OK) return res;

    label_ext_t * ext = obj_get_ext_attr(label);
    if(sign ==  GUI_SIGNAL_CLEANUP) {
        if(ext->static_txt == 0) {
            RTE_BRel(MEM_RTE,(ext->text));
            ext->text = NULL;
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
            /*Revert dots for proper refresh*/
            label_revert_dots(label);

            label_refr_text(label);
    }
    else if (sign == GUI_SIGNAL_CORD_CHG) {
        if(area_get_width(&label->coords) != area_get_width(param) ||
           area_get_height(&label->coords) != area_get_height(param))
        {
            label_revert_dots(label);
            label_refr_text(label);
        }
    }
    else if(sign == GUI_SIGNAL_REFR_EXT_SIZE) {
        if(ext->body_draw) {
            style_t * style = label_get_style(label);
            label->ext_size = RTE_MATH_MAX(label->ext_size, style->body.padding.hor);
            label->ext_size = RTE_MATH_MAX(label->ext_size, style->body.padding.ver);
        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "label";
    }

    return res;
}

/**
 * Refresh the label with its text stored in its extended data
 * @param label pointer to a label object
 */
static void label_refr_text(obj_t * label)
{
    label_ext_t * ext = obj_get_ext_attr(label);

    if(ext->text == NULL) return;

    coord_t max_w = obj_get_width(label);
    style_t * style = obj_get_style(label);
    const font_t * font = style->text.font;

    /*If the width will be expanded set the max length to very big */
    if(ext->long_mode == GUI_LABEL_LONG_EXPAND ||
       ext->long_mode == GUI_LABEL_LONG_SCROLL) {
        max_w = GUI_COORD_MAX;
    }

    /*Calc. the height and longest line*/
    point_t size;
    txt_flag_t flag = GUI_TXT_FLAG_NONE;
    if(ext->recolor != 0) flag |= GUI_TXT_FLAG_RECOLOR;
    if(ext->expand != 0) flag |= GUI_TXT_FLAG_EXPAND;
    if(ext->no_break != 0) flag |= GUI_TXT_FLAG_NO_BREAK;
    txt_get_size(&size, ext->text, font, style->text.letter_space, style->text.line_space, max_w, flag);

    /*Set the full size in expand mode*/
    if(ext->long_mode == GUI_LABEL_LONG_EXPAND || ext->long_mode == GUI_LABEL_LONG_SCROLL) {
        obj_set_size(label, size.x, size.y);

        /*Start scrolling if the label is greater then its parent*/
        if(ext->long_mode == GUI_LABEL_LONG_SCROLL) {
#if GUI_USE_ANIMATION
            obj_t * parent = obj_get_parent(label);

            /*Delete the potential previous scroller animations*/
            anim_del(label, (anim_fp_t) obj_set_x);
            anim_del(label, (anim_fp_t) obj_set_y);

            anim_t anim;
            anim.var = label;
            anim.repeat = 1;
            anim.playback = 1;
            anim.start = font_get_width(font, ' ');
            anim.act_time = 0;
            anim.end_cb = NULL;
            anim.path = anim_path_linear;

            anim.playback_pause = (((font_get_width(style->text.font, ' ') +
                                    style->text.letter_space) * 1000) / ext->anim_speed) * ANIM_WAIT_CHAR_COUNT;
            anim.repeat_pause = anim.playback_pause;

            if(obj_get_width(label) > obj_get_width(parent)) {
                anim.end = obj_get_width(parent) - obj_get_width(label) - font_get_width(font, ' ');
                anim.fp = (anim_fp_t) obj_set_x;
                anim.time = anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
                anim_create(&anim);
            } else if(obj_get_height(label) > obj_get_height(parent)) {
                anim.end =  obj_get_height(parent) - obj_get_height(label) - font_get_height(font);
                anim.fp = (anim_fp_t)obj_set_y;
                anim.time = anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
                anim_create(&anim);
            }
#endif
        }
    }
    /*In roll mode keep the size but start offset animations*/
    else if(ext->long_mode == GUI_LABEL_LONG_ROLL) {
#if GUI_USE_ANIMATION
        anim_t anim;
        anim.var = label;
        anim.repeat = 1;
        anim.playback = 1;
        anim.start = font_get_width(font, ' ');
        anim.act_time = 0;
        anim.end_cb = NULL;
        anim.path = anim_path_linear;
        anim.playback_pause =  (((font_get_width(style->text.font, ' ') + style->text.letter_space) * 1000) / ext->anim_speed) * ANIM_WAIT_CHAR_COUNT;;
        anim.repeat_pause =  anim.playback_pause;

        bool hor_anim = false;
        if(size.x > obj_get_width(label)) {
            anim.end = obj_get_width(label) - size.x - font_get_width(font, ' ');
            anim.fp = (anim_fp_t) label_set_offset_x;
            anim.time = anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
            anim_create(&anim);
            hor_anim = true;
        } else {
            /*Delete the offset animation if not required*/
            anim_del(label, (anim_fp_t) label_set_offset_x);
            ext->offset.x = 0;
        }

        if(size.y > obj_get_height(label) && hor_anim == false) {
            anim.end =  obj_get_height(label) - size.y - (font_get_height(font));
            anim.fp = (anim_fp_t)label_set_offset_y;
            anim.time = anim_speed_to_time(ext->anim_speed, anim.start, anim.end);
            anim_create(&anim);
        } else {
            /*Delete the offset animation if not required*/
            anim_del(label, (anim_fp_t) label_set_offset_y);
            ext->offset.y = 0;
        }
#endif
    }
    else if(ext->long_mode == GUI_LABEL_LONG_DOT) {
       if(size.y <= obj_get_height(label)) {                /*No dots are required, the text is short enough*/
           ext->dot_end = GUI_LABEL_DOT_END_INV;
       } else if(txt_get_length(ext->text) <= GUI_LABEL_DOT_NUM) {     /*Don't turn to dots all the characters*/
           ext->dot_end = GUI_LABEL_DOT_END_INV;
       } else {
           point_t p;
           p.x = obj_get_width(label) - (font_get_width(style->text.font, '.') + style->text.letter_space) * GUI_LABEL_DOT_NUM; /*Shrink with dots*/
           p.y = obj_get_height(label);
           p.y -= p.y  % (font_get_height(style->text.font) + style->text.line_space);   /*Round down to the last line*/
           p.y -= style->text.line_space;                                                      /*Trim the last line space*/
           uint32_t letter_id = label_get_letter_on(label, &p);


#if GUI_TXT_UTF8 == 0
           /*Save letters under the dots and replace them with dots*/
           uint8_t i;
           for(i = 0; i < GUI_LABEL_DOT_NUM; i++)  {
               ext->dot_tmp[i] = ext->text[letter_id + i];
               ext->text[letter_id + i] = '.';
           }

           ext->dot_tmp[GUI_LABEL_DOT_NUM] = ext->text[letter_id + GUI_LABEL_DOT_NUM];
           ext->text[letter_id + GUI_LABEL_DOT_NUM] = '\0';

           ext->dot_end = letter_id + GUI_LABEL_DOT_NUM;
#else
           /*Save letters under the dots and replace them with dots*/
           uint32_t i;
           uint32_t byte_id = txt_utf8_get_byte_id(ext->text, letter_id);
           uint32_t byte_id_ori = byte_id;
           uint8_t len = 0;
           for(i = 0; i <= GUI_LABEL_DOT_NUM; i++)  {
               len += txt_utf8_size(ext->text[byte_id]);
               txt_utf8_next(ext->text, &byte_id);
           }

           memcpy(ext->dot_tmp, &ext->text[byte_id_ori], len);
           ext->dot_tmp[len] = '\0';       /*Close with a zero*/

           for(i = 0; i < GUI_LABEL_DOT_NUM; i++) {
               ext->text[byte_id_ori + i] = '.';
           }
           ext->text[byte_id_ori + GUI_LABEL_DOT_NUM] = '\0';

           ext->dot_end = letter_id + GUI_LABEL_DOT_NUM;
#endif

       }
   }
    /*In break mode only the height can change*/
    else if (ext->long_mode == GUI_LABEL_LONG_BREAK) {
        obj_set_height(label, size.y);
    }


    obj_invalidate(label);
}

static void label_revert_dots(obj_t *label)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    if(ext->long_mode != GUI_LABEL_LONG_DOT) return;
    if(ext->dot_end == GUI_LABEL_DOT_END_INV) return;
#if GUI_TXT_UTF8 == 0
    uint32_t i;
    for(i = 0; i <= GUI_LABEL_DOT_NUM; i++) {
        ext->text[ext->dot_end - i] = ext->dot_tmp[GUI_LABEL_DOT_NUM - i];
    }
#else
    uint32_t letter_i = ext->dot_end - GUI_LABEL_DOT_NUM;
    uint32_t byte_i = txt_utf8_get_byte_id(ext->text, letter_i);

    /*Restore the characters*/
    uint8_t i = 0;
    while(ext->dot_tmp[i] != '\0') {
        ext->text[byte_i + i] = ext->dot_tmp[i];
        i++;
    }
#endif

    ext->dot_end = GUI_LABEL_DOT_END_INV;
}

#if GUI_USE_ANIMATION
static void label_set_offset_x(obj_t * label, coord_t x)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    ext->offset.x = x;
    obj_invalidate(label);
}

static void label_set_offset_y(obj_t * label, coord_t y)
{
    label_ext_t * ext = obj_get_ext_attr(label);
    ext->offset.y = y;
    obj_invalidate(label);
}
#endif
#endif
