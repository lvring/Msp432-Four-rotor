#ifndef __GUI_CONFIG_H
#define __GUI_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<s>  VERSION
//<i> LITTLEV GUI ԭʼ�ļ��汾��
#define GUI_VERSION                 "5.1.1"
// <h> Horizontal and vertical resolution of the library
 //<i> GUI������������
	 //<o> GUI_HOR_RES
	 //<i> GUIˮƽ���ش�С
	#define GUI_HOR_RES          (320)
	 //<o> GUI_VER_RES
	 //<i> GUI��ֱ���ش�С
	#define GUI_VER_RES          (240)
	 //<o> GUI_DPI
	 //<i> GUIDPI��С
	#define GUI_DPI              100
// </h>

// <h> Color settings
// <i> GUIɫ������
	// <o> GUI_COLOR_DEPTH
	// <i> GUIʹ�õ�ɫ��λ����С
	// <i> Ĭ�ϴ�С: 16����λ��bit��
	#define GUI_COLOR_DEPTH     16                     /*Color depth: 1/8/16/24*/
	#define GUI_COLOR_TRANSP    GUI_COLOR_LIME          /*Images pixels with this color will not be drawn (with chroma keying)*/
// </h>

// <h> Graphical settings
// <i> GUIͼ�񻺳����
	// ��СΪ ~1/10 screen�����ڻ����ͼ��������Լ���͸����
	#define GUI_VDB_SIZE         (GUI_VER_RES / 10 * GUI_HOR_RES) 
	/* Use two Virtual Display buffers (VDB) parallelize rendering and flushing (optional)
	 * The flushing should use DMA to write the frame buffer in the background*/
	// <e> GUI_VDB_DOUBLE
	// <i> GUI ˫�����ͼ����ҪDMA���
	#define GUI_VDB_DOUBLE       0       /*1: Enable the use of 2 VDBs*/
	// </e>
// </h>

// <h> Graphics feature usage
// <i> GUI����ֵ����
	// <q> GUI_USE_GROUP
	// <i> GUI Groupģ��
	#define GUI_USE_GROUP            1               /*1: Enable object groups (for keyboards)*/
	// <q> GUI_USE_ANIMATION
	// <i> GUI ����ģ��
	#define GUI_USE_ANIMATION        1            /*1: Enable all animations*/
	// <q> GUI_USE_SHADOW
	// <i> GUI ��Ӱģ��
	#define GUI_USE_SHADOW           1               /*1: Enable shadows*/
	// <q> GUI_USE_GPU
	// <i> GUI ʹ��ͼ�μ���Ӳ��
	#define GUI_USE_GPU              0               /*1: Enable GPU interface*/
	// <o> GUI_REFR_PERIOD
	// <i> GUI ��Ļˢ����
	// <i> Ĭ�ϴ�С: 50����λ��MS��
	#define GUI_REFR_PERIOD          50           /*Screen refresh period in milliseconds*/	
	// <q> GUI_ANTIALIAS
	// <i> GUI �����ʹ��
	#define GUI_ANTIALIAS        1       /*1: Enable anti-aliasing*/
	// <o> GUI_INV_FIFO_SIZE
	// <i> GUI ��ĻĬ�����object��Ŀ
	#define GUI_INV_FIFO_SIZE          32        /*The average count of objects on a screen */
// </h>

// <h> Input device settings
// <i> GUI�����豸����
	// <o> GUI_INDEV_READ_PERIOD
	// <i> GUI �����豸��ȡ���ʱ�䣨��λ��ms��
	#define GUI_INDEV_READ_PERIOD            0                     /*Input device read period in milliseconds*/
	#define GUI_INDEV_DRAG_LIMIT             10                     /*Drag threshold in pixels */
	#define GUI_INDEV_DRAG_THROW             20                     /*Drag throw slow-down in [%]. Greater value means faster slow-down */
	// <o> GUI_INDEV_LONG_PRESS_TIME
	// <i> GUI �����豸����ʱ�䣨��λ��ms��
	#define GUI_INDEV_LONG_PRESS_TIME        400                    /*Long press time in milliseconds*/
	// <o> GUI_INDEV_LONG_PRESS_REP_TIME
	// <i> GUI �����豸�����ط�ʱ�䣨��λ��ms��
	#define GUI_INDEV_LONG_PRESS_REP_TIME    100                    /*Repeated trigger period in long press [ms] */
// </h>

// <h> FONT usage
// <i> GUI ��ʹ�õ�����
#define GUI_FONT_DEFAULT        &font_dejavu_20     /*Always set a default font from the built-in fonts*/
/* More info about fonts: https://littlevgl.com/basics#fonts
 * To enable a built-in font use 1,2,4 or 8 values
 * which will determine the bit-per-pixel */
#define USE_FONT_DEJAVU_10              4
#define USE_FONT_DEJAVU_10_LATIN_SUP    0
#define USE_FONT_DEJAVU_10_CYRILLIC     0
#define USE_FONT_SYMBOL_10              4

#define USE_FONT_DEJAVU_20              4
#define USE_FONT_DEJAVU_20_LATIN_SUP    0
#define USE_FONT_DEJAVU_20_CYRILLIC     0
#define USE_FONT_SYMBOL_20              4

#define USE_FONT_DEJAVU_30              4
#define USE_FONT_DEJAVU_30_LATIN_SUP    0
#define USE_FONT_DEJAVU_30_CYRILLIC     0
#define USE_FONT_SYMBOL_30              4

#define USE_FONT_DEJAVU_40              4
#define USE_FONT_DEJAVU_40_LATIN_SUP    0
#define USE_FONT_DEJAVU_40_CYRILLIC     0
#define USE_FONT_SYMBOL_40              4
// </h>

// <h> Text settings
// <i> GUI �ı�����
	// <q> GUI_TXT_UTF8
	// <i> GUI ʹ��UTF8����
	#define GUI_TXT_UTF8             1                /*Enable UTF-8 coded Unicode character usage */
	//<s>  GUI_TXT_BREAK_CHARS
	//<i> GUI TXTʹ���ض��ַ���
	#define GUI_TXT_BREAK_CHARS     " ,.;:-_"         /*Can break texts on these chars*/
// </h>

// <h> Compiler settings
// <i> GUI ����������
	// <q> GUI_COMPILER_VLA_SUPPORTED
	// <i> GUI ������֧�ֱ䳤����
	#define GUI_COMPILER_VLA_SUPPORTED    1        /* 1: Variable length array is supported*/
// </h>

/*===================
 *  GUI_OBJ SETTINGS
 *==================*/
#define GUI_OBJ_FREE_NUM_TYPE    uint32_t    /*Type of free number attribute (comment out disable free number)*/
#define GUI_OBJ_FREE_PTR         1           /*Enable the free pointer attribute*/
#endif
/*================
 *  THEME USAGE
 *================*/
#define GUI_USE_THEME_DEFAULT    0       /*Built mainly from the built-in styles. Consumes very few RAM*/
#define GUI_USE_THEME_ALIEN      0       /*Dark futuristic theme*/
#define GUI_USE_THEME_NIGHT      1       /*Dark elegant theme*/
#define GUI_USE_THEME_MONO       0       /*Mono color theme for monochrome displays*/
#define GUI_USE_THEME_MATERIAL   0       /*Flat theme with bold colors and light shadows*/
#define GUI_USE_THEME_ZEN        0       /*Peaceful, mainly light theme */

// <h> Objectx settings
// <i> GUI �ؼ�����
	// <e> GUI_USE_LABEL
	// <i> GUI ��ǩ�ؼ� �����ؼ�����
	#define GUI_USE_LABEL    1
	#if GUI_USE_LABEL != 0
	// <o> GUI_LABEL_SCROLL_SPEED
	// <i> GUI ��ǩ�ؼ�����ʱ�䣨��λ��ms��
	#define GUI_LABEL_SCROLL_SPEED       25     /*Hor, or ver. scroll speed [px/sec] in 'GUI_LABEL_LONG_SCROLL/ROLL' mode*/
	#endif
	// </e>
	// <e> GUI_USE_IMG
	// <i> GUI ͼƬ�ؼ� �����ؼ���label
	#define GUI_USE_IMG      1
	// </e>
	// <e> GUI_USE_LINE
	// <i> GUI ֱ�߿ؼ� �����ؼ�����
	#define GUI_USE_LINE     1
	// </e>
	// <e> GUI_USE_CONT
	// <i> GUI �����ؼ� �����ؼ�����
	#define GUI_USE_CONT     1
	// </e>
	// <e> GUI_USE_PAGE
	// <i> GUI ҳ�ؼ� �����ؼ���cont
	#define GUI_USE_PAGE     1
	// </e>
	// <e> GUI_USE_WIN
	// <i> GUI ���ڿؼ� �����ؼ���cont, btn, label, img, page
	#define GUI_USE_WIN      1
	// </e>
	// <e> GUI_USE_TABVIEW
	// <i> GUI ��ǩҳ�ؼ� �����ؼ���page, btnm
	#define GUI_USE_TABVIEW      1
	#if GUI_USE_TABVIEW != 0
	// <o> GUI_LABEL_SCROLL_SPEED
	// <i> GUI ��ǩҳ�ؼ��л�ʱ�䣨��λ��ms��
	#define GUI_TABVIEW_ANIM_TIME    300     /*Time of slide animation [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> GUI_USE_BAR
	// <i> GUI �������ؼ� �����ؼ�����
	#define GUI_USE_BAR      1
	// </e>
	// <e> GUI_USE_LMETER
	// <i> GUI ���ȱ�ؼ� �����ؼ�����
	#define GUI_USE_LMETER   1
	// </e>
	// <e> GUI_USE_GAUGE
	// <i> GUI ���ȱ�ؼ�2 �����ؼ���bar, lmeter
	#define GUI_USE_GAUGE    1
	// </e>
	// <e> GUI_USE_CHART
	// <i> GUI ��άͼ�ؼ� �����ؼ�����
	#define GUI_USE_CHART    1
	// </e>
	// <e> GUI_USE_LED
	// <i> GUI LED�ؼ� �����ؼ�����
	#define GUI_USE_LED      1
	// </e>
	// <e> GUI_USE_MBOX
	// <i> GUI ��Ϣ���ڿؼ� �����ؼ���rect, btnm, label
	#define GUI_USE_MBOX     1
	// </e>
	// <e> GUI_USE_TA
	// <i> GUI �ı���ؼ� �����ؼ���label, page
	#define GUI_USE_TA       1
	#if GUI_USE_TA != 0
	// <o> GUI_TA_CURSOR_BLINK_TIME
	// <i> GUI �ı���ؼ�������˸�������λ��ms��
	#define GUI_TA_CURSOR_BLINK_TIME 400     /*ms*/
	// <o> GUI_TA_PWD_SHOW_TIME
	// <i> GUI �ı���ؼ�PWD��ʾʱ�䣨��λ��ms��
	#define GUI_TA_PWD_SHOW_TIME     1500    /*ms*/
	#endif
	// </e>
	// <e> GUI_USE_BTN
	// <i> GUI ��ť�ؼ� �����ؼ���cont
	#define GUI_USE_BTN      1
	// </e>
	// <e> GUI_USE_BTNM
	// <i> GUI ����ť�ؼ� �����ؼ�����
	#define GUI_USE_BTNM     1
	// </e>
	// <e> GUI_USE_KB
	// <i> GUI ���̿ؼ� �����ؼ���btnm
	#define GUI_USE_KB       1
	// </e>
	// <e> GUI_USE_CB
	// <i> GUI ��ѡ��ؼ� �����ؼ���btn, label
	#define GUI_USE_CB       1
	// </e>
	// <e> GUI_USE_LIST
	// <i> GUI �б�ؼ� �����ؼ���page, btn, label, (img optionally for icons )
	#define GUI_USE_LIST     1
	#if GUI_USE_LIST != 0
	// <o> GUI_LIST_FOCUS_TIME
	// <i> GUI �б�ؼ����㶯��ʱ�䣨��λ��ms��
	#define GUI_LIST_FOCUS_TIME  100 /*Default animation time of focusing to a list element [ms] (0: no animation)  */
	#endif
	// </e>
	// <e> GUI_USE_DDLIST
	// <i> GUI �����б�ؼ� �����ؼ���page, label
	#define GUI_USE_DDLIST    1
	#if GUI_USE_DDLIST != 0
	// <o> GUI_DDLIST_ANIM_TIME
	// <i> GUI �����б�ؼ�����ʱ�䣨��λ��ms��
	#define GUI_DDLIST_ANIM_TIME     200     /*Open and close default animation time [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> GUI_USE_ROLLER
	// <i> GUI �����б�ؼ� �����ؼ���ddlist
	#define GUI_USE_ROLLER    1
	#if GUI_USE_ROLLER != 0
	// <o> GUI_ROLLER_ANIM_TIME
	// <i> GUI �����б�ؼ�����ʱ�䣨��λ��ms��
	#define GUI_ROLLER_ANIM_TIME     200     /*Focus animation time [ms] (0: no animation)*/
	#endif
	// </e>
	// <e> GUI_USE_SLIDER
	// <i> GUI �϶��������ؼ� �����ؼ���bar
	#define GUI_USE_SLIDER    1
	// </e>
	// <e> GUI_USE_SW
	// <i> GUI ���ؿؼ� �����ؼ���slider
	#define GUI_USE_SW       1
	// </e>
	/*Experimental use for 3D modeling*/
	#define GUI_USE_TRIANGLE 0
// </h>
/****************** (C) COPYRIGHT SuChow University Shannon********************/
