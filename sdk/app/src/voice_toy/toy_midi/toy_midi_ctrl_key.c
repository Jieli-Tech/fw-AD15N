#include "key.h"
#include "msg.h"
#include "typedef.h"

#if KEY_IR_EN
#define IRKEY_MIDI_CTRL_SHORT_UP \
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

#define IRKEY_MIDI_CTRL_LONG \
							/*00*/		MSG_NEXT_WORKMODE,\
							/*01*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*02*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IRKEY_MIDI_CTRL_HOLD \
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

#define IRKEY_MIDI_CTRL_LONG_UP \
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

#define IRKEY_MIDI_CTRL_DOUBLE_KICK \
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

#define IRKEY_MIDI_CTRL_SHORT \
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

const u16 irkey_msg_midi_keyboard_table[][IR_KEY_MAX_NUM] = {
    /*短按*/		{IRKEY_MIDI_CTRL_SHORT},
    /*短按抬起*/	{IRKEY_MIDI_CTRL_SHORT_UP},
    /*长按*/		{IRKEY_MIDI_CTRL_LONG},
    /*连按*/		{IRKEY_MIDI_CTRL_HOLD},
    /*长按抬起*/	{IRKEY_MIDI_CTRL_LONG_UP},
    /*双击*/		{IRKEY_MIDI_CTRL_DOUBLE_KICK},
};
#endif

#if KEY_IO_EN
#define IOKEY_MIDI_CTRL_SHORT_UP \
							/*00*/		MSG_VOL_DOWN,\
							/*01*/		MSG_VOL_UP,\
							/*02*/		MSG_MIDICTRL_CHANNAL_NEXT,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_MIDI_CTRL_LONG \
							/*00*/		MSG_NEXT_WORKMODE,\
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
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		MSG_VOL_DOWN,\
							/*05*/		MSG_VOL_UP,\
							/*06*/		MSG_MIDICTRL_CHANNAL_PREV,\
							/*07*/		MSG_MIDICTRL_CHANNAL_NEXT,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_LONG \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		MSG_NEXT_WORKMODE,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*07*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		MSG_MIDICTRL_PITCH_BEND_DOWN,\
							/*07*/		MSG_MIDICTRL_PITCH_BEND_UP,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_MIDI_CTRL_LONG_UP \
							/*00*/		MSG_MIDICTRL_NOTE_OFF_DO,\
							/*01*/		MSG_MIDICTRL_NOTE_OFF_RE,\
							/*02*/		MSG_MIDICTRL_NOTE_OFF_MI,\
							/*03*/		MSG_MIDICTRL_NOTE_OFF_FA,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

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
							/*00*/		MSG_MIDICTRL_NOTE_ON_DO,\
							/*01*/		MSG_MIDICTRL_NOTE_ON_RE,\
							/*02*/		MSG_MIDICTRL_NOTE_ON_MI,\
							/*03*/		MSG_MIDICTRL_NOTE_ON_FA,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 adkey_msg_midi_keyboard_table[][AD_KEY_MAX_NUM] = {
    /*短按*/		{ADKEY_MIDI_CTRL_SHORT},
    /*短按抬起*/	{ADKEY_MIDI_CTRL_SHORT_UP},
    /*长按*/		{ADKEY_MIDI_CTRL_LONG},
    /*连按*/		{ADKEY_MIDI_CTRL_HOLD},
    /*长按抬起*/	{ADKEY_MIDI_CTRL_LONG_UP},
    /*双击*/		{ADKEY_MIDI_CTRL_DOUBLE_KICK},
};
#endif

u16 midi_keyboard_key_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_midi_keyboard_table[key_status][key_num];
        break;
#endif
#if KEY_IR_EN
    case KEY_TYPE_IR:
        msg = irkey_msg_midi_keyboard_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_midi_keyboard_table[key_status][key_num];
        break;
#endif
    default:
        break;
    }
    return msg;
}


