#include "usb_config.h"
#include "usb/scsi.h"
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



#if TCFG_PC_ENABLE || TCFG_UDISK_ENABLE
#define EP0_DMA_SIZE    (64+4)
#define HID_DMA_SIZE    (64+4)
#define AUDIO_DMA_SIZE  (256+4)
#define MSD_DMA_SIZE    ((64+4)*2)
#define     MAX_EP_TX   5
#define     MAX_EP_RX   5
static usb_interrupt usb_interrupt_tx[USB_MAX_HW_NUM][MAX_EP_TX];// SEC(.usb_g_bss);
static usb_interrupt usb_interrupt_rx[USB_MAX_HW_NUM][MAX_EP_RX];// SEC(.usb_h_bss);

static u8 ep0_dma_buffer[EP0_DMA_SIZE]     __attribute__((aligned(4))) SEC(.usb_h_dma)    ;
static u8 ep1_msd_dma_buffer[2][MSD_DMA_SIZE]  __attribute__((aligned(4)))SEC(.usb_msd_dma);
static u8 ep2_hid_dma_buffer[HID_DMA_SIZE]  __attribute__((aligned(4)))SEC(.usb_hid_dma);
static u8 ep2_spk_dma_buffer[AUDIO_DMA_SIZE]   __attribute__((aligned(4)))SEC(.usb_iso_dma);
static u8 ep3_mic_dma_buffer[AUDIO_DMA_SIZE]   __attribute__((aligned(4)))SEC(.usb_iso_dma);

struct usb_config_var_t {
    u8 usb_setup_buffer[USB_SETUP_SIZE];
    struct usb_ep_addr_t usb_ep_addr;
    struct usb_setup_t usb_setup;
};
static struct usb_config_var_t *usb_config_var[USB_MAX_HW_NUM] = {NULL};

#if USB_MALLOC_ENABLE
#else
static struct usb_config_var_t _usb_config_var[USB_MAX_HW_NUM] SEC(.usb_config_var);
#endif

__attribute__((always_inline_when_const_args))
void *usb_get_ep_buffer(const usb_dev usb_id, u32 ep)
{
    u8 *ep_buffer = NULL;
    u32 _ep = ep & 0xf;
    if (ep & USB_DIR_IN) {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        case 1:
            ep_buffer = ep1_msd_dma_buffer[0];
            break;
        case 2:
            ep_buffer = ep2_hid_dma_buffer;
            break;
        case 3:
            ep_buffer = ep3_mic_dma_buffer;
            break;
        case 4:
            ep_buffer = NULL;
            break;
        }
    } else {
        switch (_ep) {
        case 0:
            ep_buffer = ep0_dma_buffer;
            break;
        case 1:
            ep_buffer = ep1_msd_dma_buffer[0];
            break;
        case 2:
            ep_buffer = ep2_spk_dma_buffer;
            break;
        case 4:
            ep_buffer = NULL;
            break;
        }
    }
    return ep_buffer;
}



void usb_isr(const usb_dev usb_id)
{
    u32 intr_usb, intr_usbe;
    u32 intr_tx, intr_txe;
    u32 intr_rx, intr_rxe;

    __asm__ volatile("ssync");
    usb_read_intr(usb_id, &intr_usb, &intr_tx, &intr_rx);
    usb_read_intre(usb_id, &intr_usbe, &intr_txe, &intr_rxe);
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    intr_usb &= intr_usbe;
    intr_tx &= intr_txe;
    intr_rx &= intr_rxe;

    if (intr_usb & INTRUSB_SUSPEND) {
        log_error("usb suspend");
        usb_sie_close(usb_id);
    }
    if (intr_usb & INTRUSB_RESET_BABBLE) {
        log_error("usb reset");
        usb_reset_interface(usb_device);
    }
    if (intr_usb & INTRUSB_RESUME) {
        log_error("usb resume");
    }

    if (intr_tx & BIT(0)) {
        if (usb_interrupt_rx[usb_id][0]) {
            usb_interrupt_rx[usb_id][0](usb_device, 0);
        } else {
#if TCFG_PC_ENABLE
            usb_control_transfer(usb_device);
#endif
        }
    }

    for (int i = 1; i < MAX_EP_TX; i++) {
        if (intr_tx & BIT(i)) {
            if (usb_interrupt_tx[usb_id][i]) {
                usb_interrupt_tx[usb_id][i](usb_device, i);
            }
        }
    }

    for (int i = 1; i < MAX_EP_RX; i++) {
        if (intr_rx & BIT(i)) {
            if (usb_interrupt_rx[usb_id][i]) {
                usb_interrupt_rx[usb_id][i](usb_device, i);
            }
        }
    }
    __asm__ volatile("csync");
}
SET_INTERRUPT
void usb0_g_isr()
{
    usb_isr(0);
}
SET_INTERRUPT
void usb0_sof_isr()
{
    const usb_dev usb_id = 0;
    usb_sof_clr_pnd(usb_id);
    static u32 sof_count = 0;
    if ((sof_count++ % 1000) == 0) {
        log_info("sof 1s isr frame:%d", usb_read_sofframe(usb_id));
    }
}

__attribute__((always_inline_when_const_args))
u32 usb_g_set_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander)
{
    if (ep & USB_DIR_IN) {
        usb_interrupt_tx[usb_id][ep & 0xf] = hander;
    } else {
        usb_interrupt_rx[usb_id][ep] = hander;
    }
    return 0;
}
void usb_g_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    request_irq(IRQ_USB_CTRL_IDX, priority, usb0_g_isr, cpu_id);
}
void usb_sof_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    request_irq(IRQ_USB_SOF_IDX, priority, usb0_sof_isr, cpu_id);
}
u32 usb_config(const usb_dev usb_id)
{
    memset(usb_interrupt_rx[usb_id], 0, sizeof(usb_interrupt_rx[usb_id]));
    memset(usb_interrupt_tx[usb_id], 0, sizeof(usb_interrupt_tx[usb_id]));

    log_info("zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);
    if (!usb_config_var[usb_id]) {
#if USB_MALLOC_ENABLE
        usb_config_var[usb_id] = (struct usb_config_var_t *)zalloc(sizeof(struct usb_config_var_t));
        if (!usb_config_var[usb_id]) {
            return -1;
        }
#else
        memset(&_usb_config_var, 0, sizeof(_usb_config_var));
        usb_config_var[usb_id] = &_usb_config_var[usb_id];
#endif
    }
    log_info("zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);

    usb_var_init(usb_id, &(usb_config_var[usb_id]->usb_ep_addr));
    usb_setup_init(usb_id, &(usb_config_var[usb_id]->usb_setup), usb_config_var[usb_id]->usb_setup_buffer);
    return 0;
}

u32 usb_release(const usb_dev usb_id)
{
    log_info("free zalloc: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);
    usb_var_init(usb_id, NULL);
    usb_setup_init(usb_id, NULL, NULL);
#if USB_MALLOC_ENABLE
    if (usb_config_var[usb_id]) {
        log_info("free: usb_config_var[%d] = %x\n", usb_id, usb_config_var[usb_id]);
        free(usb_config_var[usb_id]);
    }
#endif

    usb_config_var[usb_id] = NULL;

    return 0;
}
#endif
