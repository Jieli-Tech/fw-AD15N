#ifndef __TOY_USB_SLAVE_H__
#define __TOY_USB_SLAVE_H__
#include "key.h"
#if HAS_USB_EN
#include "device/usb/otg.h"

u16 usb_slave_key_msg_filter(u8 key_status, u8 key_num, u8 key_type);
void toy_usb_slave_app(void);
#endif
#endif

