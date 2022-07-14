#include "mio_api.h"
#include "vfs.h"
#include "gpio.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#define MIO_EN				0
/* MIO最多支持4路PWM通道 */
#define MIO_API_PWM_PORT	IO_PORTA_15
#define PWM_FRE				3000

/* MIO最多支持(16 - 已使用PWM通道数)路IO通道 */
#define MIO_API_IO_PORT		JL_PORTA
#define MIO_API_IO_OFFSET	1

u32 mio_a_read(void *pfile, u8 *buff, u32 len)
{
    return vfs_read(pfile, buff, len);
}
void mio_a_pwm_init(u32 chl)
{
#if MIO_EN
    log_info("mio pwm init -> chl : %d\n", chl);
    gpio_set_pull_up(MIO_API_PWM_PORT, 1);
    gpio_set_pull_down(MIO_API_PWM_PORT, 1);
    SFR(JL_IOMC->IOMC3, 0, 3, 0);
    gpio_set_direction(MIO_API_PWM_PORT, 0);
    gpio_set_die(MIO_API_PWM_PORT, 0);
    JL_PWM->TMR0_PR = sys_clock_get() / PWM_FRE;
    JL_PWM->CH0_CMP = 0;
    SFR(JL_PWM->PWMCON1, (1 + chl * 4), 3, chl);
    JL_PWM->TMR0_CON = BIT(5);
    JL_PWM->PWMCON0 |= BIT(8 + chl);
    JL_PWM->PWMCON0 |= BIT(chl);
#endif
}
void mio_a_pwm_run(u32 chl, u32 pwm_var)
{
#if MIO_EN
    local_irq_disable();
    JL_PWM->CH0_CMP = (sys_clock_get() / PWM_FRE + 1) * pwm_var / 255;	//pwm_var范围在0-255
    local_irq_enable();
#endif
}
void mio_a_io_init(u32 mask)
{
#if MIO_EN
    log_info("mio io init -> mask : 0x%x\n", mask);
    MIO_API_IO_PORT->PU &= ~(mask << MIO_API_IO_OFFSET);
    MIO_API_IO_PORT->PD |= (mask << MIO_API_IO_OFFSET);
    MIO_API_IO_PORT->DIR &= ~(mask << MIO_API_IO_OFFSET);
    MIO_API_IO_PORT->OUT &= ~(mask << MIO_API_IO_OFFSET);
#endif
}
void mio_a_io_run(u32 mask, u32 io_ver)
{
#if MIO_EN
    MIO_API_IO_PORT->OUT &= ~(mask << MIO_API_IO_OFFSET);
    MIO_API_IO_PORT->OUT |= (io_ver << MIO_API_IO_OFFSET);
#endif
}

void mio_a_hook_init(sound_mio_obj *obj)
{
    obj->read = mio_a_read;
    obj->pwm_init = mio_a_pwm_init;
    obj->pwm_run = mio_a_pwm_run;
    obj->io_init = mio_a_io_init;
    obj->io_run = mio_a_io_run;
}

