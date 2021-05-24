#include "app_config.h"
#include "usb/device/usb_stack.h"
void usb_start();
extern volatile int usb_wakeup;
void USB_MassStorage(const struct usb_device_t *usb_device);
void usb_test()
{
    //slave mode
#if 0
    usb_start();
    while (1) {

        while (usb_wakeup == 0);
        usb_wakeup = 0;
        USB_MassStorage(0);
    }
#else
    void usb_host_test();
    usb_host_test();
#endif

}
