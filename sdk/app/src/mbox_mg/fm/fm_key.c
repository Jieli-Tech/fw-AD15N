#include "typedef.h"
#include "key.h"
#include "msg.h"
#include "common/hot_msg.h"


#if KEY_IR_EN
#define IRFF00_FM_SHORT_UP			\
								/*00*/    NO_MSG,\
							    /*01*/    MSG_NEXT_WORKMODE,\
								/*02*/    MSG_MUTE,\
								/*03*/    MSG_MUSIC_PP,\
								/*04*/    MSG_FM_PREV_STATION,\
								/*05*/    MSG_FM_NEXT_STATION,\
								/*06*/    NO_MSG,\
								/*07*/    MSG_VOL_DOWN,\
								/*08*/    MSG_VOL_UP,\
								/*09*/    MSG_0,\
                                /*10*/    MSG_FM_PREV_STEP,\
								/*11*/    MSG_FM_NEXT_STEP,\
								/*12*/    MSG_1,\
								/*13*/    MSG_2,\
								/*14*/    MSG_3,\
								/*15*/    MSG_4,\
								/*16*/    MSG_5,\
								/*17*/    MSG_6,\
								/*18*/    MSG_7,\
								/*19*/    MSG_8,\
								/*20*/    MSG_9

#define IRFF00_FM_SHORT			\
                                /*00*/   NO_MSG,\
							    /*01*/   NO_MSG,\
								/*02*/   NO_MSG,\
								/*03*/   NO_MSG,\
								/*04*/   NO_MSG,\
								/*05*/   NO_MSG,\
								/*06*/   NO_MSG,\
								/*07*/   NO_MSG,\
								/*08*/   NO_MSG,\
								/*09*/   NO_MSG,\
                                /*10*/   NO_MSG,\
								/*11*/   NO_MSG,\
								/*12*/   NO_MSG,\
								/*13*/   NO_MSG,\
								/*14*/   NO_MSG,\
								/*15*/   NO_MSG,\
								/*16*/   NO_MSG,\
								/*17*/   NO_MSG,\
								/*18*/   NO_MSG,\
								/*19*/   NO_MSG,\
								/*20*/   NO_MSG

#define IRFF00_FM_LONG			\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    MSG_CH_SAVE,\
								/*03*/    MSG_FM_SCAN_ALL_INIT,\
								/*04*/    MSG_FM_SCAN_ALL_DOWN,\
								/*05*/    MSG_FM_SCAN_ALL_UP,\
								/*06*/    NO_MSG,\
								/*07*/    MSG_VOL_DOWN,\
								/*08*/    MSG_VOL_UP,\
								/*09*/    NO_MSG,\
                                /*10*/    MSG_FM_PREV_STEP,\
								/*11*/    MSG_FM_NEXT_STEP,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
								/*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG

#define IRFF00_FM_HOLD			\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    MSG_VOL_DOWN,\
								/*08*/    MSG_VOL_UP,\
								/*09*/    NO_MSG,\
                                /*10*/    MSG_FM_PREV_STEP,\
								/*11*/    MSG_FM_NEXT_STEP,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
								/*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG


#define IRFF00_FM_LONG_UP		\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    NO_MSG,\
								/*08*/    NO_MSG,\
								/*09*/    NO_MSG,\
								/*10*/    NO_MSG,\
								/*11*/    NO_MSG,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
                                /*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG


#define IRFF00_FM_DOUBLE_KICK		\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    NO_MSG,\
								/*08*/    NO_MSG,\
								/*09*/    NO_MSG,\
								/*10*/    NO_MSG,\
								/*11*/    NO_MSG,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
                                /*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG

const u16 irff00_msg_fm_table[][IR_KEY_MAX_NUM] = {			//Music模式下的遥控转换表
    /*短按*/	    {IRFF00_FM_SHORT},
    /*短按*/	    {IRFF00_FM_SHORT_UP},
    /*长按*/		{IRFF00_FM_LONG},
    /*连按*/		{IRFF00_FM_HOLD},
    /*长按抬起*/	{IRFF00_FM_LONG_UP},
    /*双击*/		{IRFF00_FM_DOUBLE_KICK},
};
#endif

#if KEY_IO_EN
#define IOKEY_FM_SHORT_UP \
							/*00*/		MSG_VOL_UP,\
							/*01*/		MSG_VOL_DOWN,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_FM_LONG \
							/*00*/		MSG_NEXT_WORKMODE,\
							/*01*/		MSG_FM_SCAN_ALL_INIT,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_FM_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_FM_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_FM_DOUBLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_FM_SHORT \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 iokey_msg_mbox_fm_table[][IO_KEY_MAX_NUM] = {
    /*短按*/		{IOKEY_FM_SHORT},
    /*短按抬起*/	{IOKEY_FM_SHORT_UP},
    /*长按*/		{IOKEY_FM_LONG},
    /*连按*/		{IOKEY_FM_HOLD},
    /*长按抬起*/	{IOKEY_FM_LONG_UP},
    /*双击*/		{IOKEY_FM_DOUBLE_KICK},
};
#endif

#if KEY_AD_EN
#define ADKEY_FM_SHORT_UP \
							/*00*/		MSG_MUTE,\
							/*01*/		MSG_FM_NEXT_STATION,\
							/*02*/		MSG_FM_PREV_STATION,\
							/*03*/		NO_MSG,\
							/*04*/		MSG_VOL_UP,\
							/*05*/		MSG_VOL_DOWN,\
							/*06*/		MSG_FM_NEXT_STEP,\
							/*07*/		MSG_FM_PREV_STEP,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_FM_LONG \
							/*00*/		NO_MSG,\
							/*01*/		MSG_FM_SCAN_ALL_UP,\
							/*02*/		MSG_FM_SCAN_ALL_DOWN,\
							/*03*/		MSG_FM_SCAN_ALL_INIT,\
							/*04*/		MSG_NEXT_WORKMODE,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_FM_NEXT_STEP,\
							/*07*/		MSG_FM_PREV_STEP,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_FM_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_FM_NEXT_STEP,\
							/*07*/		MSG_FM_PREV_STEP,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_FM_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_FM_DOUBLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_FM_SHORT \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 adkey_msg_mbox_fm_table[][AD_KEY_MAX_NUM] = {
    /*短按*/		{ADKEY_FM_SHORT},
    /*短按抬起*/	{ADKEY_FM_SHORT_UP},
    /*长按*/		{ADKEY_FM_LONG},
    /*连按*/		{ADKEY_FM_HOLD},
    /*长按抬起*/	{ADKEY_FM_LONG_UP},
    /*双击*/		{ADKEY_FM_DOUBLE_KICK},
};
#endif

