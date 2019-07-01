#ifndef __RTE_CONFIG_H
#define __RTE_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<s>  RTE_VERSION
//<i> RTE�汾�ţ����嶨�������ĵ���
#define RTE_VERSION                 "3.0.1002"
// <e> RTE_USE_BGET
// <i> RTE_BGet������Ķ�̬�ڴ棬���ɶ����ɣ�ÿ����䲻ͬ��С��
// <i> ��SL_RTE�У�RTE_BGet����ؽṹ��Ϊ��̬����ʽ������ģ��������RTE_BGetʵ�ֶ�̬����
#ifndef RTE_USE_BGET
#define RTE_USE_BGET                1 
#endif
	// <o> RTE_MEM_SIZE
	// <i> RTEʹ�õö�̬�ڴ��С
	// <i> Ĭ�ϴ�С: 32����λ��K��
	#define RTE_MEM_SIZE    		(10U)            /*Size memory used by `mem_alloc` in bytes (>= 2kB)*/
	// <q> RTE_MEM_AUTO_DEFRAG
	// <i> RTE�ڴ���Ƭ����
	#define GUI_MEM_AUTO_DEFRAG  1               /*Automatically defrag on free*/
// </e>
// <e> RTE_USE_STDIO
// <i> c���Ա�׼stdio
// <i> ע�⣺��Դ���޵�����¿��Կ��ǲ�ʹ��
#ifndef RTE_USE_STDIO
#define RTE_USE_STDIO              0
#endif
// </e>
// <e> RTE_USE_RINGBUF
// <i> ���ζ���
#ifndef RTE_USE_RINGBUF
#define RTE_USE_RINGBUF          1
#endif
// </e>
// <e> RTE_USE_HUMMANINTERFACE
// <i> �˻������ӿ�
#ifndef RTE_USE_HUMMANINTERFACE
#define RTE_USE_HUMMANINTERFACE   1 
#endif
	// <e> HI_USE_SHELL
	// <i> ��shell����
	#ifndef HI_USE_SHELL
	#define HI_USE_SHELL          0
	#endif
		// <o> HI_SHELL_MAX_NUM
		// <i> ����shell���֧�ֵ�ָ����Ŀ
		// <i> Ĭ�ϴ�С: 16
		#define HI_SHELL_MAX_NUM    		16
		// <o> HI_SHELL_MAX_ARGS
		// <i> ����shell���Խ�������������ָ�������
		// <i> Ĭ�ϴ�С: 8
		#define HI_SHELL_MAX_ARGS      8
		// <o> HI_SHELL_MAX_BUFSIZE
		// <i> ����shellʹ�õ����ݻ����С
		// <i> Ĭ�ϴ�С: 32 [bytes] 
		#define HI_SHELL_MAX_BUFSIZE    		32
	// </e>
// </e>
// <e> RTE_USE_ROUNDROBIN
// <i> ��̬ʱ��Ƭ��ת����
#ifndef RTE_USE_ROUNDROBIN
#define RTE_USE_ROUNDROBIN   1 
#endif
	// <e> RTE_USE_OS
	// <i> ʵʱ����ϵͳ
	#ifndef RTE_USE_OS
	#define RTE_USE_OS          1
	#endif
	// </e>
	#if RTE_USE_OS == 1
	
	#endif
	// <o> HI_ROUNDROBIN_MAX_NUM
	// <i> ��ת�������֧�ֵ�Timer��Ŀ
	// <i> Ĭ�ϴ�С: 16
	#define HI_ROUNDROBIN_MAX_NUM    		16
	// <o> HI_ROUNDROBIN_MAX_GROUP
	// <i> ��ת�������֧�ֵ�TimerGroup��Ŀ
	// <i> Ĭ�ϴ�С: 4
	#define HI_ROUNDROBIN_MAX_GROUP_NUM   4
// </e>
// <e> RTE_USE_STATEMACHINE
// <i> ״̬��ģ��
// <i> ע��: һ��StateMachine��ռ�õ�ram�ռ�Ϊ24�ֽڣ��˴�С��ͳ��const char���������ģ�
#ifndef RTE_USE_STATEMACHINE
#define RTE_USE_STATEMACHINE   1 
#endif
// </e>
// <e> RTE_USE_KVDB
// <i> KV���ݿ�
#ifndef RTE_USE_KVDB
#define RTE_USE_KVDB   0 
	// <o> KVDB_ERASE_MIN_SIZE
	// <i> KVDB��С������λ��С����FLASH�ṹ�й� ��λ��K��
	#ifndef KVDB_ERASE_MIN_SIZE
	#define KVDB_ERASE_MIN_SIZE         (128 * 1024)              /* it is 128K for compatibility */
	#endif
	// <e> KVDB_USE_PFS
	// <i> ���籣��ģʽ
	#ifndef KVDB_USE_PFS
	#define KVDB_USE_PFS          0
	#endif
	// </e>
	// <e> KVDB_USE_AUTO_UPDATE
	// <i> �Զ����£��������£�
	#ifndef KVDB_USE_AUTO_UPDATE
	#define KVDB_USE_AUTO_UPDATE  0
	#endif
	// </e>
	// <o> KVDB_USER_SETTING_SIZE
	// <i> �û����û���������С
	#ifndef KVDB_USER_SETTING_SIZE
	#define KVDB_USER_SETTING_SIZE             2048
	#endif
	// <o> KVDB_ADDR_OFFSET
	// <i> KVDB��ַƫ�ƣ�����FLASH�׵�ַ��Ϊʵ�ʵ�ַ����λ��K��
	#ifndef KVDB_ADDR_OFFSET
	#define KVDB_ADDR_OFFSET             			 512 * 1024 
	#endif
	// <o> FLASH_BASE
	// <i> FLASH�׵�ַ
	#ifndef KVDB_FLASH_BASE
	#define KVDB_FLASH_BASE                    0x08000000
	#endif
	// <o> KVDB_FM_VER_NUM
	// <i> �̼��汾�������⵽��Ʒ�洢�İ汾�����趨�汾�Ų�һ�£����Զ�׷��Ĭ�ϻ������������������Ļ���������
	#ifndef KVDB_FM_VER_NUM
	#define KVDB_FM_VER_NUM                 0
	#endif
#endif
// </e>
// <e> RTE_USE_GUI
// <i> ����GUI
#ifndef RTE_USE_GUI
#define RTE_USE_GUI   0 
#endif
// </e>
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
