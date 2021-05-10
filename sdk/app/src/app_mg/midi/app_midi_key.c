#include "typedef.h"
#include "key.h"
#include "msg.h"

#if KEY_IO_EN
#define IOKEY_MIDI_SHORT_UP \
							/*00*/		MSG_VOL_UP,\
							/*01*/		MSG_VOL_DOWN,\
							/*02*/		MSG_MIDI_OKON_GOON,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_LONG \
							/*00*/		MSG_NEXT_MODE,\
							/*01*/		NO_MSG,\
							/*02*/		MSG_MIDI_MODE_SWITCH,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_HOLD \
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

#define IOKEY_MIDI_LONG_UP \
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

#define IOKEY_MIDI_DOUBLE_KICK \
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

#define IOKEY_MIDI_SHORT \
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

const u16 iokey_msg_midi_table[][IO_KEY_MAX_NUM] = {
    /*短按*/		{IOKEY_MIDI_SHORT},
    /*短按抬起*/	{IOKEY_MIDI_SHORT_UP},
    /*长按*/		{IOKEY_MIDI_LONG},
    /*连按*/		{IOKEY_MIDI_HOLD},
    /*长按抬起*/	{IOKEY_MIDI_LONG_UP},
    /*双击*/		{IOKEY_MIDI_DOUBLE_KICK},
};
#endif

#if KEY_AD_EN
#define ADKEY_MIDI_SHORT_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		MSG_MIDI_OKON_GOON,\
							/*05*/		MSG_VOL_DOWN,\
							/*06*/		MSG_VOL_UP,\
							/*07*/		MSG_PRIV_FILE,\
							/*08*/		MSG_NEXT_FILE,\
							/*09*/		MSG_PP,\

#define ADKEY_MIDI_LONG \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		MSG_MIDI_MODE_SWITCH,\
							/*05*/		MSG_NEXT_MODE,\
							/*06*/		NO_MSG,\
							/*07*/		MSG_A_PLAY,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_HOLD \
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

#define ADKEY_MIDI_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		MSG_POWER_OFF,\

#define ADKEY_MIDI_DOUBLE_KICK \
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

#define ADKEY_MIDI_SHORT \
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

const u16 adkey_msg_midi_table[][AD_KEY_MAX_NUM] = {
    /*短按*/		{ADKEY_MIDI_SHORT},
    /*短按抬起*/	{ADKEY_MIDI_SHORT_UP},
    /*长按*/		{ADKEY_MIDI_LONG},
    /*连按*/		{ADKEY_MIDI_HOLD},
    /*长按抬起*/	{ADKEY_MIDI_LONG_UP},
    /*双击*/		{ADKEY_MIDI_DOUBLE_KICK},
};
#endif

