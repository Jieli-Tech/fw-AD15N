#ifndef _USB_DEVICE_MODE_H
#define _USB_DEVICE_MODE_H

#include "usb/device/usb_stack.h"
#include "usb/device/msd.h"
#include "usb/device/hid.h"
#include "key.h"

void USB_device_mode(void);
extern const u16 iokey_msg_mbox_usb_table[3][IO_KEY_MAX_NUM];
extern const u16 adkey_msg_mbox_usb_table[3][AD_KEY_MAX_NUM];
extern const u16 irff00_msg_usb_table[3][IR_KEY_MAX_NUM];

#endif
