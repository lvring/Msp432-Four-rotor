#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H
#ifdef __cplusplus
extern "C" {
#endif
	//¶ÏÑÔÍ·ÎÄ¼þ
	#include <assert.h>
	//
	#include <string.h>
	#include <stdarg.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <stddef.h>
	#include <stdlib.h>
	#include <math.h>
	#include "RTE_Config.h"
	#include "RTE_Vec.h"
	#include "RTE_List.h"
	
	#if RTE_USE_BGET == 1
	#include "RTE_BGet.h"
	#endif
	#include "RTE_Stdlib.h"
	#if RTE_USE_HUMMANINTERFACE == 1
		#if HI_USE_SHELL == 1
		#include "RTE_Shell.h"
		#endif
	#endif
	#if RTE_USE_ROUNDROBIN == 1
		#include "RTE_RoundRobin.h"
		#if RTE_USE_OS == 1
		#include "cmsis_os2.h"
		#endif
	#endif
	#if RTE_USE_STATEMACHINE == 1
		#include "RTE_StateMachine.h"
	#endif
	
	#if RTE_USE_KVDB == 1
	  #include "RTE_KVDB.h"
	#endif
	#if RTE_USE_STDIO == 1
	#include <stdio.h>
	#else
	#include "RTE_Stdio.h"
	#endif
	#if RTE_USE_RINGBUF == 1
	#include "RTE_RingQuene.h"
	#endif
	
	#if RTE_USE_GUI == 1
	#include "GUI_Config.h"

	#include "GUI_Core/GUI_Area.h"
	#include "GUI_Core/GUI_Anim.h"
	#include "GUI_Core/GUI_Color.h"
	#include "GUI_Core/GUI_Font.h"
	#include "GUI_Core/GUI_Style.h"
	#include "GUI_Core/GUI_Refresh.h"
	#include "GUI_Core/GUI_Obj.h"
	#include "GUI_Core/GUI_VDB.h"
	#include "GUI_HAL/GUI_Hal_Indev.h"
	#include "GUI_HAL/GUI_Hal_Disp.h"
	#include "GUI_Core/GUI_Group.h"
	#include "GUI_Core/GUI_Indev.h"
	#include "GUI_Core/GUI_TXT.h"
	#include "GUI_Core/GUI_Trigo.h"
	
	#include "GUI_Draw/GUI_Circ.h"
	#include "GUI_Draw/GUI_Draw.h"
	#include "GUI_Draw/GUI_BufferDraw.h"
	
	#include "GUI_Themes/GUI_Theme.h"
	
	#include "GUI_Data/symbol_def.h"
	
	#include "GUI_Objx/GUI_Bar.h"
	#include "GUI_Objx/GUI_Cont.h"
	#include "GUI_Objx/GUI_Btn.h"
	#include "GUI_Objx/GUI_Btnm.h"
	#include "GUI_Objx/GUI_Cb.h"
	#include "GUI_Objx/GUI_Chart.h"
	#include "GUI_Objx/GUI_Page.h"
	#include "GUI_Objx/GUI_Ddlist.h"
	#include "GUI_Objx/GUI_Lmeter.h"
	#include "GUI_Objx/GUI_Gauge.h"
	#include "GUI_Objx/GUI_Image.h"
	#include "GUI_Objx/GUI_Kb.h"
	#include "GUI_Objx/GUI_Ta.h"
	#include "GUI_Objx/GUI_Lable.h"
	#include "GUI_Objx/GUI_Led.h"
	#include "GUI_Objx/GUI_Line.h"
	#include "GUI_Objx/GUI_List.h"
	#include "GUI_Objx/GUI_Mbox.h"
	#include "GUI_Objx/GUI_Roller.h"
	#include "GUI_Objx/GUI_Slider.h"
	#include "GUI_Objx/GUI_SW.h"
	#include "GUI_Objx/GUI_Tabview.h"
	#include "GUI_Objx/GUI_Win.h"
	
	#endif
	
	#define RTE_MATH_MIN(a,b) (a<b?a:b)
	#define RTE_MATH_MAX(a,b) (a>b?a:b)
	#define RTE_MATH_ABS(x) ((x)>0?(x):(-(x)))
	
	#define RTE_ALIGN_32BYTES(buf) buf __attribute__ ((aligned (32)))
	
	extern void RTE_Init(void);
	extern void RTE_Assert(char *file, uint32_t line);
	#if 1
	#define RTE_AssertParam(expr) {                                     \
																	 if(!(expr))                        \
																	 {                                  \
																			 RTE_Assert(__FILE__, __LINE__);\
																	 }                                  \
																}
	#else
	#define RTE_AssertParam(expr) 
	#endif		
#ifdef __cplusplus
}
#endif
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
