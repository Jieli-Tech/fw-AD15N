#include "key.h"
#include "msg.h"
#include "typedef.h"

#if KEY_IO_EN
#define IOKEY_MIDI_CTRL_SHORT_UP \
							/*00*/		MSG_VOL_UP,\
							/*01*/		MSG_VOL_DOWN,\
							/*02*/		MSG_MIDICTRL_CHANNAL_NEXT,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_CTRL_LONG \
							/*00*/		MSG_NEXT_MODE,\
							/*01*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*02*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_CTRL_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*02*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_CTRL_LONG_UP \
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

#define IOKEY_MIDI_CTRL_DOUBLE_KICK \
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

#define IOKEY_MIDI_CTRL_SHORT \
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

const u16 iokey_msg_midi_keyboard_table[][IO_KEY_MAX_NUM] = {
    /*短按*/		{IOKEY_MIDI_CTRL_SHORT},
    /*短按抬起*/	{IOKEY_MIDI_CTRL_SHORT_UP},
    /*长按*/		{IOKEY_MIDI_CTRL_LONG},
    /*连按*/		{IOKEY_MIDI_CTRL_HOLD},
    /*长按抬起*/	{IOKEY_MIDI_CTRL_LONG_UP},
    /*双击*/		{IOKEY_MIDI_CTRL_DOUBLE_KICK},
};
#endif

#if KEY_AD_EN
#define ADKEY_MIDI_CTRL_SHORT_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		MSG_MIDICTRL_CHANNAL_NEXT,\
							/*03*/		MSG_MIDICTRL_CHANNAL_PREV,\
							/*04*/		MSG_VOL_UP,\
							/*05*/		MSG_VOL_DOWN,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_LONG \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*03*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*04*/		MSG_A_PLAY,\
							/*05*/		MSG_NEXT_MODE,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*03*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_MIDICTRL_NOTE_OFF_FA,\
							/*07*/		MSG_MIDICTRL_NOTE_OFF_MI,\
							/*08*/		MSG_MIDICTRL_NOTE_OFF_RE,\
							/*09*/		MSG_MIDICTRL_NOTE_OFF_DO,\

#define ADKEY_MIDI_CTRL_DOUBLE_KICK \
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

#define ADKEY_MIDI_CTRL_SHORT \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_MIDICTRL_NOTE_ON_FA,\
							/*07*/		MSG_MIDICTRL_NOTE_ON_MI,\
							/*08*/		MSG_MIDICTRL_NOTE_ON_RE,\
							/*09*/		MSG_MIDICTRL_NOTE_ON_DO,\

const u16 adkey_msg_midi_keyboard_table[][AD_KEY_MAX_NUM] = {
    /*短按*/		{ADKEY_MIDI_CTRL_SHORT},
    /*短按抬起*/	{ADKEY_MIDI_CTRL_SHORT_UP},
    /*长按*/		{ADKEY_MIDI_CTRL_LONG},
    /*连按*/		{ADKEY_MIDI_CTRL_HOLD},
    /*长按抬起*/	{ADKEY_MIDI_CTRL_LONG_UP},
    /*双击*/		{ADKEY_MIDI_CTRL_DOUBLE_KICK},
};
#endif

