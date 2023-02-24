/**
 * @file task_pc.c
 * @brief 从机模式
 * @author chenrixin@zh-jieli.com
 * @version 1.0.0
 * @date 2020-02-29
 */

#include "app_config.h"
#include "msg.h"
/* #include "system/app_core.h" */
/* #include "system/includes.h" */
/* #include "server/server_core.h" */
/* #include "app_config.h" */
/* #include "app_action.h" */
/* #include "app_task.h" */
/* #include "os/os_api.h" */
/* #include "device/sdmmc.h" */

#if TCFG_PC_ENABLE
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"

#if USB_DEVICE_CLASS_CONFIG & HID_CLASS
#include "usb/device/hid.h"
#endif

#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
#include "usb/device/msd.h"
#endif

#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS

#endif

#if (TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0)
#include "dev_multiplex_api.h"
#endif


#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

#define     USB_TASK_NAME   "usb_msd"

#define USBSTACK_EVENT		    0x80
#define USBSTACK_MSD_RUN		0x81
#define USBSTACK_MSD_RELASE		0x82
#define USBSTACK_HID		    0x83
#define USBSTACK_MSD_RESET      0x84

extern int usb_audio_demo_init(void);
extern void usb_audio_demo_exit(void);
extern void charge_event_to_user(u8 event);

static usb_dev usbfd ;//SEC(.usb_g_bss);
static OS_MUTEX msd_mutex ;//SEC(.usb_g_bss);
volatile int usb_wakeup;


static void usb_task(void *p)
{
#if 0
    int ret = 0;
    int msg[16];
    while (1) {
        ret = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }
        switch (msg[1]) {
#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
        case USBSTACK_MSD_RUN:
            os_mutex_pend(&msd_mutex, 0);
            USB_MassStorage((void *)msg[2]);
            os_mutex_post(&msd_mutex);
            break;
        case USBSTACK_MSD_RELASE:
            os_sem_post((OS_SEM *)msg[2]);
            while (1) {
                os_time_dly(10000);
            }
            break;
        case USBSTACK_MSD_RESET:
            os_mutex_pend(&msd_mutex, 0);
            msd_reset((struct usb_device_t *)msg[2], (u32)msg[3]);
            os_mutex_post(&msd_mutex);
            break;
#endif
        default:
            break;
        }
    }
#else

#endif
}

static void usb_msd_wakeup(struct usb_device_t *usb_device)
{
    /* int err = os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RUN, usb_device); */
    usb_wakeup = 1;
}
static void usb_msd_reset_wakeup(struct usb_device_t *usb_device, u32 itf_num)
{
    /* os_taskq_post_msg(USB_TASK_NAME, 3, USBSTACK_MSD_RESET, usb_device, itf_num); */
    usb_wakeup = 0xfe;
}
static void usb_msd_init()
{
    r_printf("%s()", __func__);
    /* int err;                                           */
    /* os_mutex_create(&msd_mutex);                       */
    /* err = task_create(usb_task, NULL, USB_TASK_NAME);  */
    /* if (err != OS_NO_ERR) {                            */
    /*     r_printf("usb_msd task creat fail %x\n", err); */
    /* }                                                  */
}
static void usb_msd_free()
{
    r_printf("%s()", __func__);

    /* os_mutex_del(&msd_mutex, 0); */

    int err;
    /* OS_SEM *sem = malloc(sizeof(OS_SEM));; */
    /* os_sem_create(sem, 0);                                              */
    /* os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RELASE, (int)sem); */
    /* os_sem_pend(sem, 0);                                                */
    /* free(sem);                                                          */


    /* err = task_kill(USB_TASK_NAME);          */
    /* if (!err) {                              */
    /*     r_printf("usb_msd_uninit succ!!\n"); */
    /* } else {                                 */
    /*     r_printf("usb_msd_uninit fail!!\n"); */
    /* }                                        */
}

void usb_start()
{

#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    /* usb_audio_demo_init(); */
#endif

    usb_device_mode(usbfd, USB_DEVICE_CLASS_CONFIG);

#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    //没有复用时候判断 sd开关
    //复用时候判断是否参与复用
#ifdef SDMMCA_EN
    msd_register_disk("sd0", NULL);
#endif

#ifdef SDMMCB_EN
    msd_register_disk("sd1", NULL);
#endif

#if TCFG_USB_EXFLASH_UDISK_ENABLE
    msd_register_disk(__EXT_FLASH_NANE, NULL);
#endif

#if TCFG_USB_MSD_CDROM_ENABLE
    msd_register_disk("sfc", NULL); //默认介质为sfc
#endif

    msd_set_wakeup_handle(usb_msd_wakeup);
    msd_set_reset_wakeup_handle(usb_msd_reset_wakeup);
    usb_msd_init();
#endif
}
static void usb_remove_disk()
{
#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    /* os_mutex_pend(&msd_mutex, 0); */
    /* msd_unregister_all();         */
    /* os_mutex_post(&msd_mutex);    */
#endif
}
void usb_pause()
{
    log_info("usb pause");

    usb_sie_disable(usbfd);

#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    if (msd_set_wakeup_handle(NULL)) {
        usb_remove_disk();
        usb_msd_free();
    }
#endif


#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    /* usb_audio_demo_exit(); */
#endif

    usb_device_mode(usbfd, 0);
}

void usb_stop()
{
    log_info("App Stop - usb");

    usb_pause();

    if (usb_otg_online(0) == DISCONN_MODE) {
        usb_sie_close(usbfd);
    }
}


#if 0
int pc_device_event_handler(struct sys_event *event)
{
    int switch_app_case = false;
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            const char *usb_msg = (const char *)event->u.dev.value;
            log_debug("usb event : %d DEVICE_EVENT_FROM_OTG %s", event->u.dev.event, usb_msg);

            if (usb_msg[0] == 's') {
                if (event->u.dev.event == DEVICE_EVENT_IN) {
                    log_info("usb %c online", usb_msg[2]);
                    usbfd = usb_msg[2] - '0';
#if   USB_PC_NO_APP_MODE
                    usb_start();
#elif TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                    usb_otg_suspend(0, OTG_KEEP_STATE);
                    mult_sdio_suspend();
                    usb_pause();
                    mult_sdio_resume();
#else
                    usb_pause();
#endif
                    switch_app_case = 1;
                } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                    log_info("usb %c offline", usb_msg[2]);
                    switch_app_case = 2;
#ifdef  USB_PC_NO_APP_MODE
                    usb_stop();
#else

#ifdef CONFIG_SOUNDBOX
                    if (!app_check_curr_task(APP_PC_TASK)) {
#else
                    if (!app_cur_task_check(APP_NAME_PC)) {
#endif
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                        mult_sdio_suspend();
#endif
                        usb_stop();
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                        mult_sdio_resume();
#endif
                    }

#endif
                }
            }
        }
        break;
    default:
        log_d("%x\n", event->type);
        break;
    }
    return switch_app_case;
}

#ifdef  CONFIG_DONGLE_CASE
static void usb_event_handler(struct sys_event *event, void *priv)
{
    pc_device_event_handler(event);
}

static u16 sys_event_id;
void usbstack_init()
{
    sys_event_id = register_sys_event_handler(SYS_ALL_EVENT, 2, NULL, usb_event_handler);
}
void usbstack_exit()
{
    if (sys_event_id) {
        unregister_sys_event_handler(sys_event_id);
        sys_event_id = 0;
    }
}
#endif
#endif
#endif
