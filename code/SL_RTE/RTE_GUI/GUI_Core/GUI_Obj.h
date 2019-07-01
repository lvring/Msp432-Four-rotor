#ifndef __GUI_OBJ_H
#define __GUI_OBJ_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*************************************************
*** 预编译时检查GUI_Config文件
*************************************************/
#if GUI_HOR_RES == 0 || GUI_VER_RES == 0
#error "GUI: GUI_HOR_RES and GUI_VER_RES 需要大于0"
#endif
#if GUI_ANTIALIAS > 1
#error "GUI: GUI_ANTIALIAS 抗锯齿使能需要正确配置"
#endif
#if GUI_VDB_SIZE > 0 && GUI_VDB_SIZE < GUI_HOR_RES
#error "GUI: Virtual Display Buffer过小"
#endif
#define GUI_ANIM_IN			    0x00	/*Animation to show an object. 'OR' it with anim_builtin_t*/
#define GUI_ANIM_OUT				0x80  /*Animation to hide an object. 'OR' it with anim_builtin_t*/
#define GUI_ANIM_DIR_MASK		0x80	/*ANIM_IN/ANIM_OUT mask*/
#define GUI_MAX_ANCESTOR_NUM     8
struct _obj_t;
/*************************************************
*** GUI的obj设计模式
*************************************************/
typedef enum
{
	GUI_DESIGN_DRAW_MAIN,
	GUI_DESIGN_DRAW_POST,
	GUI_DESIGN_COVER_CHK,
}design_mode_t;
typedef bool (* design_func_t) (struct _obj_t * obj, const area_t * mask_p, design_mode_t mode);
/*************************************************
*** GUI的obj用户callback返回值类型
*************************************************/
typedef enum
{
	GUI_RES_INV = 0,      /*Typically indicates that the object is deleted (become invalid) in the action function*/
	GUI_RES_OK,           /*The object is valid (no deleted) after the action*/
}res_t;
/*************************************************
*** GUI的信号
*************************************************/
typedef enum
{
  /*General signals*/
	GUI_SIGNAL_CLEANUP,
	GUI_SIGNAL_CHILD_CHG,
	GUI_SIGNAL_CORD_CHG,
	GUI_SIGNAL_STYLE_CHG,
	GUI_SIGNAL_REFR_EXT_SIZE,
	GUI_SIGNAL_GET_TYPE,
	/*Input device related*/
	GUI_SIGNAL_PRESSED,
	GUI_SIGNAL_PRESSING,
	GUI_SIGNAL_PRESS_LOST,
	GUI_SIGNAL_RELEASED,
	GUI_SIGNAL_LONG_PRESS,
	GUI_SIGNAL_LONG_PRESS_REP,
	GUI_SIGNAL_DRAG_BEGIN,
	GUI_SIGNAL_DRAG_END,
	/*Group related*/
	GUI_SIGNAL_FOCUS,
	GUI_SIGNAL_DEFOCUS,
	GUI_SIGNAL_CONTROLL,
}signal_t;
typedef res_t (* signal_func_t) (struct _obj_t * obj, signal_t sign, void * param);
/*************************************************
*** GUI的obj结构体
*************************************************/
typedef struct _obj_t
{
#ifdef GUI_OBJ_FREE_NUM_TYPE
	GUI_OBJ_FREE_NUM_TYPE free_num; 		    /*Application specific identifier (set it freely)*/
#endif
	uint8_t protect;            /*Automatically happening actions can be prevented. 'OR'ed values from obj_prot_t*/
	/*Attributes and states*/
	uint8_t click     :1;    /*1: Can be pressed by an input device*/
	uint8_t drag      :1;    /*1: Enable the dragging*/
	uint8_t drag_throw:1;    /*1: Enable throwing with drag*/
	uint8_t drag_parent  :1;    /*1: Parent will be dragged instead*/
	uint8_t hidden       :1;    /*1: Object is hidden*/
	uint8_t top       :1;    /*1: If the object or its children is clicked it goes to the foreground*/
	uint8_t reserved     :1;
#if GUI_USE_GROUP != 0
  void * group_p;                 /*Pointer to the group of the object*/
#endif
  struct _obj_t * par;    /*Pointer to the parent object*/
	void * ext_attr;                 /*Object type specific extended data*/
	style_t * style_p;       /*Pointer to the object's style*/
#if GUI_OBJ_FREE_PTR != 0
	void * free_ptr;              /*Application specific pointer (set it freely)*/
#endif
	signal_func_t signal_func;     /*Object type specific signal function*/
	design_func_t design_func;     /*Object type specific design function*/
	coord_t ext_size;			/*EXTtend the size of the object in every direction. E.g. for shadow drawing*/
	area_t coords;               /*Coordinates of the object (x1, y1, x2, y2)*/
	ll_t child_ll;          /*Linked list to store the children objects*/
}obj_t;
typedef res_t (*action_t) (struct _obj_t * obj);

/*Protect some attributes (max. 8 bit)*/
typedef enum
{
	GUI_PROTECT_NONE      = 0x00,
	GUI_PROTECT_CHILD_CHG = 0x01, /*Disable the child change signal. Used by the library*/
	GUI_PROTECT_PARENT    = 0x02, /*Prevent automatic parent change (e.g. in page)*/
	GUI_PROTECT_POS       = 0x04, /*Prevent automatic positioning (e.g. in cont layout)*/
	GUI_PROTECT_FOLLOW    = 0x08, /*Prevent the object be followed in automatic ordering (e.g. in cont PRETTY layout)*/
	GUI_PROTECT_PRESS_LOST= 0x10, /*TODO */
}protect_t;
/*Used by `obj_get_type()`. The object's and its ancestor types are stored here*/
typedef struct {
	const char * type[GUI_MAX_ANCESTOR_NUM];   /*[0]: the actual type, [1]: ancestor, [2] #1's ancestor ... [x]: "obj" */
}obj_type_t;
/*************************************************
*** GUI的obj布局位置
*************************************************/
typedef enum
{
	GUI_ALIGN_CENTER = 0,
	GUI_ALIGN_IN_TOP_LEFT,
	GUI_ALIGN_IN_TOP_MID,
	GUI_ALIGN_IN_TOP_RIGHT,
	GUI_ALIGN_IN_BOTTOM_LEFT,
	GUI_ALIGN_IN_BOTTOM_MID,
	GUI_ALIGN_IN_BOTTOM_RIGHT,
	GUI_ALIGN_IN_LEFT_MID,
	GUI_ALIGN_IN_RIGHT_MID,
	GUI_ALIGN_OUT_TOP_LEFT,
	GUI_ALIGN_OUT_TOP_MID,
	GUI_ALIGN_OUT_TOP_RIGHT,
	GUI_ALIGN_OUT_BOTTOM_LEFT,
	GUI_ALIGN_OUT_BOTTOM_MID,
	GUI_ALIGN_OUT_BOTTOM_RIGHT,
	GUI_ALIGN_OUT_LEFT_TOP,
	GUI_ALIGN_OUT_LEFT_MID,
	GUI_ALIGN_OUT_LEFT_BOTTOM,
	GUI_ALIGN_OUT_RIGHT_TOP,
	GUI_ALIGN_OUT_RIGHT_MID,
	GUI_ALIGN_OUT_RIGHT_BOTTOM,
}align_t;
/*************************************************
*** GUI的obj内建动画
*************************************************/
typedef enum
{
	GUI_ANIM_NONE = 0,
	GUI_ANIM_FLOAT_TOP, 		/*Float from/to the top*/
	GUI_ANIM_FLOAT_LEFT,		/*Float from/to the left*/
	GUI_ANIM_FLOAT_BOTTOM,	/*Float from/to the bottom*/
	GUI_ANIM_FLOAT_RIGHT,	/*Float from/to the right*/
	GUI_ANIM_GROW_H,			/*Grow/shrink  horizontally*/
	GUI_ANIM_GROW_V,			/*Grow/shrink  vertically*/
}anim_builtin_t;
/**
 * Init. the 'lv' library.
 */
void GUI_Init(void);
/**
 * Create a basic object
 * @param parent pointer to a parent object.
 *                  If NULL then a screen will be created
 * @param copy pointer to a base object, if not NULL then the new object will be copied from it
 * @return pointer to the new object
 */
obj_t * obj_create(obj_t * parent, obj_t * copy);
/**
 * Delete 'obj' and all of its children
 * @param obj pointer to an object to delete
 * @return GUI_RES_INV because the object is deleted
 */
res_t obj_del(obj_t * obj);
/**
 * Delete all children of an object
 * @param obj pointer to an object
 */
void obj_clean(obj_t *obj);
/**
 * Mark the object as invalid therefore its current position will be redrawn by 'refr_task'
 * @param obj pointer to an object
 */
void obj_invalidate(obj_t * obj);
/**
 * Load a new screen
 * @param scr pointer to a screen
 */
void scr_load(obj_t * scr);
/**
 * Set a new parent for an object. Its relative position will be the same.
 * @param obj pointer to an object
 * @param parent pointer to the new parent object
 */
void obj_set_parent(obj_t * obj, obj_t * parent);
/**
 * Set relative the position of an object (relative to the parent)
 * @param obj pointer to an object
 * @param x new distance from the left side of the parent
 * @param y new distance from the top of the parent
 */
void obj_set_pos(obj_t * obj, coord_t x, coord_t y);
/**
 * Set the x coordinate of a object
 * @param obj pointer to an object
 * @param x new distance from the left side from the parent
 */
void obj_set_x(obj_t * obj, coord_t x);
/**
 * Set the y coordinate of a object
 * @param obj pointer to an object
 * @param y new distance from the top of the parent
 */
void obj_set_y(obj_t * obj, coord_t y);
/**
 * Set the size of an object
 * @param obj pointer to an object
 * @param w new width
 * @param h new height
 */
void obj_set_size(obj_t * obj, coord_t w, coord_t h);
/**
 * Set the width of an object
 * @param obj pointer to an object
 * @param w new width
 */
void obj_set_width(obj_t * obj, coord_t w);
/**
 * Set the height of an object
 * @param obj pointer to an object
 * @param h new height
 */
void obj_set_height(obj_t * obj, coord_t h);
/**
 * Align an object to an other object.
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'align_t' enum)
 * @param x_mod x coordinate shift after alignment
 * @param y_mod y coordinate shift after alignment
 */
void obj_align(obj_t * obj,obj_t * base, align_t align, coord_t x_mod, coord_t y_mod);
/**
 * Set a new style for an object
 * @param obj pointer to an object
 * @param style_p pointer to the new style
 */
void obj_set_style(obj_t * obj, style_t * style);
/**
 * Notify an object about its style is modified
 * @param obj pointer to an object
 */
void obj_refresh_style(obj_t * obj);
/**
 * Notify all object if a style is modified
 * @param style pointer to a style. Only the objects with this style will be notified
 *               (NULL to notify all objects)
 */
void obj_report_style_mod(style_t * style);
/**
 * Hide an object. It won't be visible and clickable.
 * @param obj pointer to an object
 * @param en true: hide the object
 */
void obj_set_hidden(obj_t * obj, bool en);
/**
 * Enable or disable the clicking of an object
 * @param obj pointer to an object
 * @param en true: make the object clickable
 */
void obj_set_click(obj_t * obj, bool en);
/**
 * Enable to bring this object to the foreground if it
 * or any of its children is clicked
 * @param obj pointer to an object
 * @param en true: enable the auto top feature
 */
void obj_set_top(obj_t * obj, bool en);
/**
 * Enable the dragging of an object
 * @param obj pointer to an object
 * @param en true: make the object dragable
 */
void obj_set_drag(obj_t * obj, bool en);
/**
 * Enable the throwing of an object after is is dragged
 * @param obj pointer to an object
 * @param en true: enable the drag throw
 */
void obj_set_drag_throw(obj_t * obj, bool en);
/**
 * Enable to use parent for drag related operations.
 * If trying to drag the object the parent will be moved instead
 * @param obj pointer to an object
 * @param en true: enable the 'drag parent' for the object
 */
void obj_set_drag_parent(obj_t * obj, bool en);
/**
 * Set a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from obj_prot_t
 */
void obj_set_protect(obj_t * obj, uint8_t prot);
/**
 * Clear a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from obj_prot_t
 */
void obj_clear_protect(obj_t * obj, uint8_t prot);
/**
 * Set the signal function of an object.
 * Always call the previous signal function in the new.
 * @param obj pointer to an object
 * @param fp the new signal function
 */
void obj_set_signal_func(obj_t * obj, signal_func_t fp);
/**
 * Set a new design function for an object
 * @param obj pointer to an object
 * @param fp the new design function
 */
void obj_set_design_func(obj_t * obj, design_func_t fp);
/**
 * Allocate a new ext. data for an object
 * @param obj pointer to an object
 * @param ext_size the size of the new ext. data
 * @return pointer to the allocated ext
 */
void * obj_allocate_ext_attr(obj_t * obj, uint16_t ext_size);
/**
 * Send a 'GUI_SIGNAL_REFR_EXT_SIZE' signal to the object
 * @param obj pointer to an object
 */
void obj_refresh_ext_size(obj_t * obj);
#ifdef GUI_OBJ_FREE_NUM_TYPE
/**
 * Set an application specific number for an object.
 * It can help to identify objects in the application.
 * @param obj pointer to an object
 * @param free_num the new free number
 */
void obj_set_free_num(obj_t * obj, GUI_OBJ_FREE_NUM_TYPE free_num);
#endif
#if GUI_OBJ_FREE_PTR != 0
/**
 * Set an application specific  pointer for an object.
 * It can help to identify objects in the application.
 * @param obj pointer to an object
 * @param free_p the new free pinter
 */
void obj_set_free_ptr(obj_t * obj, void * free_p);
#endif
#if GUI_USE_ANIMATION
/**
 * Animate an object
 * @param obj pointer to an object to animate
 * @param type type of animation from 'anim_builtin_t'. 'OR' it with ANIM_IN or ANIM_OUT
 * @param time time of animation in milliseconds
 * @param delay delay before the animation in milliseconds
 * @param cb a function to call when the animation is ready
 */
void obj_animate(obj_t * obj, anim_builtin_t type, uint16_t time, uint16_t delay, void (*cb) (obj_t *));
#endif
/**
 * Return with a pointer to the active screen
 * @return pointer to the active screen object (loaded by 'scr_load()')
 */
obj_t * scr_act(void);
/**
 * Return with the top layer. (Same on every screen and it is above the normal screen layer)
 * @return pointer to the top layer object  (transparent screen sized obj)
 */
obj_t * layer_top(void);
/**
 * Return with the system layer. (Same on every screen and it is above the all other layers)
 * It is used for example by the cursor
 * @return pointer to the system layer object (transparent screen sized obj)
 */
obj_t * layer_sys(void);
/**
 * Return with the screen of an object
 * @param obj pointer to an object
 * @return pointer to a screen
 */
obj_t * obj_get_screen(obj_t * obj);
/**
 * Returns with the parent of an object
 * @param obj pointer to an object
 * @return pointer to the parent of  'obj'
 */
obj_t * obj_get_parent(obj_t * obj);
/**
 * Iterate through the children of an object (start from the "youngest, lastly created")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
obj_t * obj_get_child(obj_t * obj, obj_t * child);
/**
 * Iterate through the children of an object (start from the "oldest", firstly created)
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
obj_t * obj_get_child_back(obj_t * obj, obj_t * child);
/**
 * Count the children of an object (only children directly on 'obj')
 * @param obj pointer to an object
 * @return children number of 'obj'
 */
uint16_t obj_count_children(obj_t * obj);
/**
 * Copy the coordinates of an object to an area
 * @param obj pointer to an object
 * @param cords_p pointer to an area to store the coordinates
 */
void obj_get_coords(obj_t * obj, area_t * cords_p);
/**
 * Get the x coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the left side of its parent
 */
coord_t obj_get_x(obj_t * obj);
/**
 * Get the y coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the top of its parent
 */
coord_t obj_get_y(obj_t * obj);
/**
 * Get the width of an object
 * @param obj pointer to an object
 * @return the width
 */
coord_t obj_get_width(obj_t * obj);
/**
 * Get the height of an object
 * @param obj pointer to an object
 * @return the height
 */
coord_t obj_get_height(obj_t * obj);
/**
 * Get the extended size attribute of an object
 * @param obj pointer to an object
 * @return the extended size attribute
 */
coord_t obj_get_ext_size(obj_t * obj);
/**
 * Get the style pointer of an object (if NULL get style of the parent)
 * @param obj pointer to an object
 * @return pointer to a style
 */
style_t * obj_get_style(obj_t * obj);
/**
 * Get the hidden attribute of an object
 * @param obj pointer to an object
 * @return true: the object is hidden
 */
bool obj_get_hidden(obj_t * obj);
/**
 * Get the click enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is clickable
 */
bool obj_get_click(obj_t * obj);
/**
 * Get the top enable attribute of an object
 * @param obj pointer to an object
 * @return true: the auto top feture is enabled
 */
bool obj_get_top(obj_t * obj);
/**
 * Get the drag enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is dragable
 */
bool obj_get_drag(obj_t * obj);
/**
 * Get the drag thow enable attribute of an object
 * @param obj pointer to an object
 * @return true: drag throw is enabled
 */
bool obj_get_drag_throw(obj_t * obj);
/**
 * Get the drag parent attribute of an object
 * @param obj pointer to an object
 * @return true: drag parent is enabled
 */
bool obj_get_drag_parent(obj_t * obj);
/**
 * Get the protect field of an object
 * @param obj pointer to an object
 * @return protect field ('OR'ed values of obj_prot_t)
 */
uint8_t obj_get_protect(obj_t * obj);
/**
 * Check at least one bit of a given protect bitfield is set
 * @param obj pointer to an object
 * @param prot protect bits to test ('OR'ed values of obj_prot_t)
 * @return false: none of the given bits are set, true: at least one bit is set
 */
bool obj_is_protected(obj_t * obj, uint8_t prot);
/**
 * Get the signal function of an object
 * @param obj pointer to an object
 * @return the signal function
 */
signal_func_t obj_get_signal_func(obj_t * obj);
/**
 * Get the design function of an object
 * @param obj pointer to an object
 * @return the design function
 */
design_func_t obj_get_design_func(obj_t * obj);
/**
 * Get the ext pointer
 * @param obj pointer to an object
 * @return the ext pointer but not the dynamic version
 *         Use it as ext->data1, and NOT da(ext)->data1
 */
void * obj_get_ext_attr(obj_t * obj);
/**
 * Get object's and its ancestors type. Put their name in `type_buf` starting with the current type.
 * E.g. buf.type[0]="btn", buf.type[1]="cont", buf.type[2]="obj"
 * @param obj pointer to an object which type should be get
 * @param buf pointer to an `obj_type_t` buffer to store the types
 */
void obj_get_type(obj_t * obj, obj_type_t * buf);
#ifdef GUI_OBJ_FREE_NUM_TYPE
/**
 * Get the free number
 * @param obj pointer to an object
 * @return the free number
 */
GUI_OBJ_FREE_NUM_TYPE obj_get_free_num(obj_t * obj);
#endif
#if GUI_OBJ_FREE_PTR != 0
/**
 * Get the free pointer
 * @param obj pointer to an object
 * @return the free pointer
 */
void * obj_get_free_ptr(obj_t * obj);
#endif
#if GUI_USE_GROUP
/**
 * Get the group of the object
 * @param obj pointer to an object
 * @return the pointer to group of the object
 */
void * obj_get_group(obj_t * obj);
#endif
#define GUI_SCALE(x) (x << GUI_ANTIALIAS)
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
