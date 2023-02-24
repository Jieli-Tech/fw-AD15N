#include "usb/device/usb_stack.h"
#include "usb_config.h"
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb/device/hid.h"
#include "usb/device/uac_audio.h"
#include "irq.h"
#include "gpio.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

#if TCFG_PC_ENABLE
static const u8 user_stirng[] = {
    24,
    0x03,
    'U', 0x00,
    'S', 0x00,
    'B', 0x00,
    'A', 0x00,
    'u', 0x00,
    'd', 0x00,
    'i', 0x00,
    'o', 0x00,
    '2', 0x00,
    '.', 0x00,
    '0', 0x00,
};
static u8 root2_testing;
u32 usb_root2_testing()
{
#if USB_ROOT2
    return root2_testing;
#else
    return 0;
#endif
}
u32 check_ep_vaild(u32 ep)
{
    u32 en = 0;
    switch (ep) {
    case 0:
        en = 1;
        break;
#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    case 1:
        en = 1;
        break;
#endif
#if USB_DEVICE_CLASS_CONFIG & HID_CLASS
    case 2:
        en = 1;
        break;
#endif
#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    case 3:
        en = 1;
        break;
#endif
    case 4:
        break;
    }
    return en;
}
static u32 setup_endpoint(struct usb_device_t *usb_device, struct usb_ctrlrequest *req)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len = 0;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);

#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    u32 uac_setup_endpoint(struct usb_device_t *usb_device, struct usb_ctrlrequest * req);
    if (uac_setup_endpoint(usb_device, req)) {
        return 1;
    }
#endif
    u32 ep = LOBYTE(req->wIndex) & 0x0f;
    if (check_ep_vaild(ep) == 0) {
        usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        return 1;// not zero user handle this request
    }


    return 0;
}
static u32 setup_device(struct usb_device_t *usb_device, struct usb_ctrlrequest *req)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ret = 0;
    switch (req->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
        switch (HIBYTE(req->wValue)) {
        case USB_DT_STRING:
            switch (LOBYTE(req->wValue)) {
#if 0//USB_SLAVE_SUPPORT_AUDIO
            case SPEAKER_STR_INDEX:
            case MIC_STR_INDEX:
                if (usb_device->bDeviceStates == USB_DEFAULT) {
                    ret = 0;
                } else {
                    usb_set_data_payload(usb_device, req, user_stirng, sizeof(user_stirng));
                    ret = 1;
                }
                break;
#endif
            default:
                break;
            }
            break;
        case USB_DT_DEVICE:
#if USB_ROOT2
            if (req->wLength == 0xffff) {
                root2_testing = 1;
            }
#endif
            break;
        }
        break;
    case USB_REQ_SET_CONFIGURATION:
        break;
    case USB_REQ_SET_ADDRESS:
        /* if(req->wLength || req->wIndex){                        */
        /*     ret = 1;                                            */
        /*     usb_set_setup_phase(usb_device, USB_EP0_SET_STALL); */
        /*     dump_setup_request(req);                            */
        /*     log_debug_hexdump((u8 *)req, 8);                    */
        /* }                                                       */
        break;
    }
    return ret;
}
static u32 user_setup_filter(struct usb_device_t *usb_device, struct usb_ctrlrequest *request)
{
    // dump_setup_request(request);
    // log_debug_hexdump((u8 *)request, 8);
    u32 ret = 0;
    u32 recip = request->bRequestType & USB_RECIP_MASK;
    switch (recip) {
    case USB_RECIP_DEVICE:
        ret = setup_device(usb_device, request);
        break;
    case USB_RECIP_INTERFACE:
        break;
    case USB_RECIP_ENDPOINT:
        ret = setup_endpoint(usb_device, request);
        break;
    case USB_RECIP_OTHER:
        break;
    }
#if 0
    const char *statas[] = {"USB_ATTACHED", "USB_POWERED", "USB_DEFAULT",
                            "USB_ADDRESS", "USB_CONFIGURED", "USB_SUSPENDED"
                           };

    const char *phases[] = {"SETUP", "IN", "OUT",
                            "STALL",
                           };

    usb_log_d("state:%s phase: %s", statas[usb_device->bDeviceStates],
              phases[usb_device->bsetup_phase]);
#endif
    return ret;// not zero user handle this request
}
void user_setup_filter_install(struct usb_device_t *usb_device)
{
    usb_set_setup_hook(usb_device, user_setup_filter);
}
#endif
