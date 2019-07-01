#include "RTE_Include.h"
#if RTE_USE_GUI == 1&& GUI_USE_ANIMATION == 1
/*********************
 *      DEFINES
 *********************/
#define GUI_ANIM_RESOLUTION      1024
#define GUI_ANIM_RES_SHIFT       10

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void anim_task (void * param);
static bool anim_ready_handler(anim_t * a);

/**********************
 *  STATIC VARIABLES
 **********************/
static ll_t anim_ll = {0};
static uint32_t last_task_run = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Init. the animation module
 */
void anim_init(void)
{
    ll_init(&anim_ll, sizeof(anim_t));
    last_task_run = RTE_RoundRobin_GetTick();
    RTE_RoundRobin_CreateTimer(1,"AnimTimer",GUI_REFR_PERIOD,1,1, anim_task, (void *)0);
}

/**
 * Create an animation
 * @param anim_p an initialized 'anim_t' variable. Not required after call.
 */
void anim_create(anim_t * anim_p)
{
    /* Do not let two animations for the  same 'var' with the same 'fp'*/
    if(anim_p->fp != NULL) anim_del(anim_p->var, anim_p->fp);       /*fp == NULL would delete all animations of var*/

    /*Add the new animation to the animation linked list*/
    anim_t * new_anim = ll_ins_head(&anim_ll);
    RTE_AssertParam(new_anim);

    /*Initialize the animation descriptor*/
    anim_p->playback_now = 0;
    memcpy(new_anim, anim_p, sizeof(anim_t));

    /*Set the start value*/
    if(new_anim->fp != NULL) new_anim->fp(new_anim->var, new_anim->start);
}

/**
 * Delete an animation for a variable with a given animator function
 * @param var pointer to variable
 * @param fp a function pointer which is animating 'var',
 *           or NULL to delete all animations of 'var'
 * @return true: at least 1 animation is deleted, false: no animation is deleted
 */
bool anim_del(void * var, anim_fp_t fp)
{
    bool del = false;
    anim_t * a;
    anim_t * a_next;
    a = ll_get_head(&anim_ll);
    while(a != NULL) {
        /*'a' might be deleted, so get the next object while 'a' is valid*/
        a_next = ll_get_next(&anim_ll, a);

        if(a->var == var && (a->fp == fp || fp == NULL)) {
            ll_rem(&anim_ll, a);
            RTE_BRel(MEM_RTE , a);
            del = true;
        }

        a = a_next;
    }

    return del;
}

/**
 * Calculate the time of an animation with a given speed and the start and end values
 * @param speed speed of animation in unit/sec
 * @param start start value of the animation
 * @param end end value of the animation
 * @return the required time [ms] for the animation with the given parameters
 */
uint16_t anim_speed_to_time(uint16_t speed, int32_t start, int32_t end)
{
    int32_t d = RTE_MATH_ABS((int32_t) start - end);
    uint32_t time = (int32_t)((int32_t)(d * 1000) / speed);

    if(time > UINT16_MAX) time = UINT16_MAX;

    if(time == 0) {
        time++;
    }

    return time;
}

/**
 * Calculate the current value of an animation applying linear characteristic
 * @param a pointer to an animation
 * @return the current value to set
 */
int32_t anim_path_linear(const anim_t *a)
{
    /*Calculate the current step*/

    uint16_t step;
    if(a->time == a->act_time) step = GUI_ANIM_RESOLUTION; /*Use the last value id the time fully elapsed*/
    else step = (a->act_time * GUI_ANIM_RESOLUTION) / a->time;


    /* Get the new value which will be proportional to the current element of 'path_p'
     * and the 'start' and 'end' values*/
    int32_t new_value;
    new_value =  (int32_t) step * (a->end - a->start);
    new_value = new_value >> GUI_ANIM_RES_SHIFT;
    new_value += a->start;

    return new_value;
}

/**
 * Calculate the current value of an animation applying step characteristic.
 * (Set end value on the end of the animation)
 * @param a pointer to an animation
 * @return the current value to set
 */
int32_t anim_path_step(const anim_t *a)
{
    if(a->act_time >= a->time) return a->end;
    else return a->start;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Periodically handle the animations.
 * @param param unused
 */
static void anim_task (void * param)
{
    (void)param;

    volatile uint32_t elaps;
    elaps = RTE_RoundRobin_TickElaps(last_task_run);

    anim_t * a;
    anim_t * a_next;
    a = ll_get_head(&anim_ll);
    while(a != NULL) {
        /*'a' might be deleted, so get the next object while 'a' is valid*/
        a_next = ll_get_next(&anim_ll, a);

        a->act_time += elaps;
        if(a->act_time >= 0) {
            if(a->act_time > a->time) a->act_time = a->time;

            int32_t new_value;
            new_value = a->path(a);

            if(a->fp != NULL) a->fp(a->var, new_value); /*Apply the calculated value*/

            /*If the time is elapsed the animation is ready*/
            if(a->act_time >= a->time) {
                bool invalid;
                invalid = anim_ready_handler(a);
                if(invalid != false) {
                    a_next = ll_get_head(&anim_ll);  /*a_next might be invalid if animation delete occurred*/
                }
            }
        }

        a = a_next;
    }

    last_task_run = RTE_RoundRobin_GetTick();
}

/**
 * Called when an animation is ready to do the necessary thinks
 * e.g. repeat, play back, delete etc.
 * @param a pointer to an animation descriptor
 * @return true: animation delete occurred
 * */
static bool anim_ready_handler(anim_t * a)
{
    bool invalid = false;

    /*Delete the animation if
     * - no repeat and no play back (simple one shot animation)
     * - no repeat, play back is enabled and play back is ready */
    if((a->repeat == 0 && a->playback == 0) ||
            (a->repeat == 0 && a->playback == 1 && a->playback_now == 1)) {
        void (*cb) (void *) = a->end_cb;
        void * p = a->var;
        ll_rem(&anim_ll, a);
        RTE_BRel(MEM_RTE , a);

        /*Call the callback function at the end*/
        /* Check if an animation is deleted in the cb function
         * if yes then the caller function has to know this*/
        if(cb != NULL) cb(p);
        invalid = true;
    }
    /*If the animation is not deleted then restart it*/
    else {
        a->act_time = - a->repeat_pause;    /*Restart the animation*/
        /*Swap the start and end values in play back mode*/
        if(a->playback != 0) {
            /*If now turning back use the 'playback_pause*/
            if(a->playback_now == 0) a->act_time = - a->playback_pause;

            /*Toggle the play back state*/
            a->playback_now = a->playback_now == 0 ? 1: 0;
            /*Swap the start and end values*/
            int32_t tmp;
            tmp = a->start;
            a->start = a->end;
            a->end = tmp;
        }
    }

    return invalid;
}
#endif
