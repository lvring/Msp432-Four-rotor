/**
 * @file img.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_IMG != 0

/*Testing of dependencies*/
#if GUI_USE_LABEL == 0
#error "img: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool img_design(obj_t * img, const area_t * mask, design_mode_t mode);
static res_t img_signal(obj_t * img, signal_t sign, void * param);

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
 * Create an image objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a image object, if not NULL then the new object will be copied from it
 * @return pointer to the created image
 */
obj_t * img_create(obj_t * par, obj_t * copy)
{
    obj_t * new_img = NULL;
    
    /*Create a basic object*/
    new_img = obj_create(par, copy);
    RTE_AssertParam(new_img);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_img);
    
    /*Extend the basic object to image object*/
    img_ext_t * ext = obj_allocate_ext_attr(new_img, sizeof(img_ext_t));
    RTE_AssertParam(ext);
    ext->src = NULL;
    ext->src_type = GUI_IMG_SRC_UNKNOWN;
    ext->w = obj_get_width(new_img);
    ext->h = obj_get_height(new_img);
    ext->chroma_keyed = 0;
    ext->alpha_byte = 0;
    ext->auto_size = 1;

    /*Init the new object*/    
    obj_set_signal_func(new_img, img_signal);
    obj_set_design_func(new_img, img_design);
    
    if(copy == NULL) {
		/* Enable auto size for non screens
		 * because image screens are wallpapers
		 * and must be screen sized*/
		if(par != NULL) ext->auto_size = 1;
		else ext->auto_size = 0;
		if(par != NULL) obj_set_style(new_img, NULL);            /*Inherit the style  by default*/
		else obj_set_style(new_img, &style_plain);            /*Set a style for screens*/
    } else {
        img_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->auto_size = copy_ext->auto_size;
    	img_set_src(new_img, copy_ext->src);

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_img);
    }

    return new_img;
}


/*=====================
 * Setter functions 
 *====================*/


/**
 * Set the pixel map to display by the image
 * @param img pointer to an image object
 * @param data the image data
 */
void img_set_src(obj_t * img, const void * src_img)
{
    img_src_t src_type = img_get_src_type(src_img);
    img_ext_t * ext = obj_get_ext_attr(img);


    if(src_type == GUI_IMG_SRC_UNKNOWN) {
        if(ext->src_type == GUI_IMG_SRC_SYMBOL || ext->src_type == GUI_IMG_SRC_FILE) {
            RTE_BRel(MEM_RTE,(void *)ext->src);
        }
        ext->src = NULL;
        ext->src_type = GUI_IMG_SRC_UNKNOWN;
        return;
    }

    ext->src_type = src_type;

    if(src_type == GUI_IMG_SRC_VARIABLE) {
        ext->src = src_img;
        ext->w = ((img_t*)src_img)->header.w;
        ext->h = ((img_t*)src_img)->header.h;
        ext->chroma_keyed = ((img_t*)src_img)->header.chroma_keyed;
        ext->alpha_byte = ((img_t*)src_img)->header.alpha_byte;
        obj_set_size(img, ext->w, ext->h);
    }
    else if(src_type == GUI_IMG_SRC_SYMBOL) {
        style_t * style = obj_get_style(img);
        point_t size;
        txt_get_size(&size, src_img, style->text.font, style->text.letter_space, style->text.line_space, GUI_COORD_MAX, GUI_TXT_FLAG_NONE);
        ext->w = size.x;
        ext->h = size.y;
        ext->chroma_keyed = 1;    /*Symbols always have transparent parts, Important because of cover check in the design function*/

        /* If the new and the old src are the same then it was only a refresh.*/
        if(ext->src != src_img) {
            RTE_BRel(MEM_RTE,(void *)ext->src);
            char * new_txt = RTE_BGet(MEM_RTE,strlen(src_img) + 1);
            strcpy(new_txt, src_img);
            ext->src = new_txt;
        }
    }

    if(img_get_auto_size(img) != false) {
        obj_set_size(img, ext->w, ext->h);
    }

    obj_invalidate(img);
}

/**
 * Enable the auto size feature.
 * If enabled the object size will be same as the picture size.
 * @param img pointer to an image
 * @param autosize_en true: auto size enable, false: auto size disable
 */
void img_set_auto_size(obj_t * img, bool autosize_en)
{
    img_ext_t * ext = obj_get_ext_attr(img);

    ext->auto_size = (autosize_en == false ? 0 : 1);
}


/*=====================
 * Getter functions 
 *====================*/

/**
 * Get the type of an image source
 * @param src pointer to an image source:
 *  - pointer to an 'img_t' variable (image stored internally and compiled into the code)
 *  - a path to an file (e.g. "S:/folder/image.bin")
 *  - or a symbol (e.g. SYMBOL_CLOSE)
 * @return type of the image source GUI_IMG_SRC_VARIABLE/FILE/SYMBOL/UNKOWN
 */
img_src_t img_get_src_type(const void * src)
{
    if(src == NULL) return GUI_IMG_SRC_UNKNOWN;
    const uint8_t * uint8_t_p = src;

    /*The first byte shows the type of the image source*/
    if(uint8_t_p[0] >= 'A' && uint8_t_p[0] <= 'Z') return GUI_IMG_SRC_FILE;    /*It's a driver letter*/
    else if(((uint8_t_p[0] & 0xFC) >> 2) == GUI_IMG_FORMAT_INTERNAL_RAW) return GUI_IMG_SRC_VARIABLE;      /*Mask the file format part og of img_t header. IT should be 0 which means C array */
    else if(uint8_t_p[0] >= ' ') return GUI_IMG_SRC_SYMBOL;               /*Other printable characters are considered symbols*/

    else return GUI_IMG_SRC_UNKNOWN;
}

/**
 * Get the name of the file set for an image
 * @param img pointer to an image
 * @return file name
 */
const char * img_get_file_name(obj_t * img)
{
    img_ext_t * ext = obj_get_ext_attr(img);

    if(ext->src_type == GUI_IMG_SRC_FILE) return ext->src;
    else return "";
}


/**
 * Get the auto size enable attribute
 * @param img pointer to an image
 * @return true: auto size is enabled, false: auto size is disabled
 */
bool img_get_auto_size(obj_t * img)
{
    img_ext_t * ext = obj_get_ext_attr(img);

    return ext->auto_size == 0 ? false : true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the images
 * @param img pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'        
 */
static bool img_design(obj_t * img, const area_t * mask, design_mode_t mode)
{
    style_t * style = obj_get_style(img);
    img_ext_t * ext = obj_get_ext_attr(img);

    if(mode == GUI_DESIGN_COVER_CHK) {
        bool cover = false;
        if(ext->src_type == GUI_IMG_SRC_UNKNOWN || ext->src_type == GUI_IMG_SRC_SYMBOL) return false;

        if(ext->chroma_keyed == 0 && ext->alpha_byte == 0) cover = area_is_in(mask, &img->coords);
        return cover;

    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
        if(ext->h == 0 || ext->w == 0) return true;
		area_t coords;

		obj_get_coords(img, &coords);

		if(ext->src_type == GUI_IMG_SRC_FILE || ext->src_type == GUI_IMG_SRC_VARIABLE) {
		    area_t cords_tmp;
            cords_tmp.y1 = coords.y1;
            cords_tmp.y2 = coords.y1 + ext->h - 1;

            for(; cords_tmp.y1 < coords.y2; cords_tmp.y1 += ext->h, cords_tmp.y2 += ext->h) {
                cords_tmp.x1 = coords.x1;
                cords_tmp.x2 = coords.x1 + ext->w - 1;
                for(; cords_tmp.x1 < coords.x2; cords_tmp.x1 += ext->w, cords_tmp.x2 += ext->w) {
                    draw_img(&cords_tmp, mask, style, ext->src);
                }
            }
		} else if(ext->src_type == GUI_IMG_SRC_SYMBOL) {
            draw_label(&coords, mask, style, ext->src, GUI_TXT_FLAG_NONE, NULL);

		} else {

		    /*Trigger the error handler of image drawer*/
            draw_img(&img->coords, mask, style, NULL);

		}
    }
    
    return true;
}


/**
 * Signal function of the image
 * @param img pointer to an image object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t img_signal(obj_t * img, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(img, sign, param);
    if(res != GUI_RES_OK) return res;

    img_ext_t * ext = obj_get_ext_attr(img);
    if(sign == GUI_SIGNAL_CLEANUP) {
        if(ext->src_type == GUI_IMG_SRC_FILE || ext->src_type == GUI_IMG_SRC_SYMBOL) {
            RTE_BRel(MEM_RTE,(void *)ext->src);
            ext->src = NULL;
            ext->src_type = GUI_IMG_SRC_UNKNOWN;
        }
    }
    else if(sign == GUI_SIGNAL_STYLE_CHG) {
        /*Refresh the file name to refresh the symbol text size*/
        if(ext->src_type == GUI_IMG_SRC_SYMBOL) {
            img_set_src(img, ext->src);

        }
    }
    else if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "img";
    }

    return res;
}

#endif
