/* #include "generic/gpio.h" */
#include "sfr.h"
#include "cpu.h"
#include "config.h"
#include "irflt.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

#define ir_log 		log_info//log_d

IR_CODE  ir_code;       ///<红外遥控信息
u16 irtmr_prd;
volatile u8 ir_busy = 0;

static const u16 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};


static const u8 IRTabFF00[] = {
    NKEY_00, NKEY_01, NKEY_02, NKEY_03, NKEY_04, NKEY_05, NKEY_06, IR_06, IR_15, IR_08, NKEY_0A, NKEY_0B, IR_12, IR_11, NKEY_0E, NKEY_0F,
    NKEY_10, NKEY_11, NKEY_12, NKEY_13, NKEY_14, IR_07, IR_09, NKEY_17, IR_13, IR_10, NKEY_1A, NKEY_1B, IR_16, NKEY_1D, NKEY_1E, NKEY_1F,
    NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
    NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,
    IR_04, NKEY_41, IR_18, IR_05, IR_03, IR_00, IR_01, IR_02, NKEY_48, NKEY_49, IR_20, NKEY_4B, NKEY_4C, NKEY_4D, NKEY_4E, NKEY_4F,
    NKEY_50, NKEY_51, IR_19, NKEY_53, NKEY_54, NKEY_55, NKEY_56, NKEY_57, NKEY_58, NKEY_59, IR_17, NKEY_5B, NKEY_5C, NKEY_5D, IR_14, NKEY_5F,
};


/* #define INPUT_CHANNLE0_SRC_SEL(x)		SFR(JL_IOMC->IOMC2, 5, 5, x) */
#define INPUT_CHANNLE0_SRC_SEL(x)		SFR(JL_IOMC->IOMC2, 6, 6, x)
#define IRFLT_OUTPUT_TIMER_SEL(x)		SFR(JL_IOMC->IOMC1, 3, 1, x)
#define CAP_TIMER_SEL(x)	        	SFR(JL_IOMC->IOMC1, 4, 2, x)

#if 1

#define IRTMR       JL_TMR2
#define IRQ_IRTMR   IRQ_TIME2_IDX
#define IOMC_CAP    (1<<4)
#define SEL_IRTMR  SEL_TIMER2

#else

#define IRTMR       JL_TMR1
#define IRQ_IRTMR   IRQ_TIME1_IDX
#define IOMC_CAP    (2<<4)
#define SEL_IRTMR  SEL_TIMER1

#endif


/*----------------------------------------------------------------------------*/
/**@brief   time1红外中断服务函数
   @param   void
   @param   void
   @return  void
   @note    void timer1_ir_isr(void)
*/
/*----------------------------------------------------------------------------*/
___interrupt
static void irtmr_ir_isr(void)
{
    u16 bCap1;
    u8 cap = 0;

    static u8 cnt = 0;

    IRTMR->CON |= BIT(6);

    bCap1 = IRTMR->PRD;
    IRTMR->CNT = 0;
    cap = bCap1 / irtmr_prd;
    /* ir_log("cnt = %d, cap = 0x%x", cnt++, cap); */
    if (cap <= 1) {
        ir_code.wData >>= 1;
        ir_code.bState++;
        ir_code.boverflow = 0;
    } else if (cap == 2) {
        ir_code.wData >>= 1;
        ir_code.bState++;
        ir_code.wData |= 0x8000;
        ir_code.boverflow = 0;
    }
    /*13ms-Sync*/
    /*
    else if ((cap == 13) || (cap < 8) || (cap > 110))
    {
        ir_code.bState = 0;
    }
    else
    {
        ir_code.boverflow = 0;
    }
    */
    else if ((cap == 13) && (ir_code.boverflow < 8)) {
        ir_code.bState = 0;
        ir_busy = 1;
    } else if ((cap < 8) && (ir_code.boverflow < 5)) {
        ir_code.bState = 0;
    } else if ((cap > 110) && (ir_code.boverflow > 53)) {
        ir_code.bState = 0;
    } else if ((cap > 20) && (ir_code.boverflow > 53)) { //溢出情况下 （12M 48M）
        ir_code.bState = 0;
    } else {
        ir_code.boverflow = 0;
    }
    if (ir_code.bState == 16) {
        ir_code.wUserCode = ir_code.wData;
    }
    if (ir_code.bState == 32) {
        log_info("[0x%X]\n", ir_code.wData);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void set_ir_clk(void)

   ((cnt - 1)* 分频数)/lsb_clk = 1ms
*/
/*----------------------------------------------------------------------------*/
#define OSC_Hz  		12000000L

/* #define MAX_TIME_CNT 0x07ff //分频准确范围，更具实际情况调整 */
/* #define MIN_TIME_CNT 0x0030 */
void set_ir_clk(void)
{
    u32 clk;
    u32 prd_cnt;
    /* u8 index; */
    clk = OSC_Hz;//clock_get_lsb_freq();

    clk /= (1000 * 64);
    clk *= 1; //1ms for cnt
    prd_cnt = clk;
    /* for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) { */
    /* prd_cnt = (clk + timer_div[index]) / timer_div[index]; */
    /* if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) { */
    /* break; */
    /* } */
    /* } */
    irtmr_prd = prd_cnt;

    /* JL_IOMC->IOMC1 &= ~(3 << 4); */
    /* JL_IOMC->IOMC1 |= (2<<4); */
    /* JL_IOMC->IOMC1 |= IOMC_CAP; */
    /* JL_IOMC->IOMC2 &= ~0x1f; */
    /* JL_IOMC->IOMC2 |= 0x7; */
    /* JL_PORTA->DIR |= BIT(7); */
    IRTMR->CON = BIT(6);
    IRTMR->PRD = 0;
    IRTMR->CNT = 0;
    IRTMR->CON = ((3 << 4) | (2 << 2) | (3 << 0));  //osc + falling edge

}


static void ir_input_io_sel(u8 port)
{
    //选择input channel1输入
    // IOMC2[13 : 8]
    // 0 ~ 15: 		PA0 ~ PA15
    //16 ~ 31: 		PB0 ~ PB15
    //32 ~ 47: 		PC0 ~ PC15
    //48 ~ 55: 		PD0 ~ PD7
    INPUT_CHANNLE0_SRC_SEL(port);

    gpio_set_direction(port, 1);
    gpio_set_die(port, 1);
    /* JL_PORTA->DIR |= BIT(8); */
}

static inline void ir_output_timer_sel(u8 timer)
{
    log_info("timer%d too irflt!!!\n", timer + 1);
    IRFLT_OUTPUT_TIMER_SEL(timer);
    /* CAP_TIMER_SEL(timer+1); */
}

static void irflt_config()
{

    JL_IRFLT->CON = 0;
    JL_IRFLT->CON |= BIT(6) | BIT(5);		//256 div
    JL_IRFLT->CON |= BIT(2);		//osc
    JL_IRFLT->CON |= BIT(0);		//irflt enable

    set_ir_clk();
}

static void log_irflt_info()
{
    ir_log("IOMC1 = 0x%x\n", JL_IOMC->IOMC1);
    ir_log("IOMC2 = 0x%x\n", JL_IOMC->IOMC2);
    ir_log("RFLT_CON = 0x%x\n", JL_IRFLT->CON);
    ir_log("IR TIMER_CON = 0x%x\n", IRTMR->CON);
}

void ir_timeout(void)
{
    ir_code.boverflow++;
    if (ir_code.boverflow > 10) { //120ms
        ir_code.boverflow = 10;
        ir_code.bState = 0;
        ir_busy = 0;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ir_key_init(void)
*/
/*----------------------------------------------------------------------------*/
#if 0
static int irflt_init(const struct dev_node *node, void *arg)
{
    //timer1
    ir_log("ir key init >>>\n");
    struct irflt_platform_data *user_data = (struct irflt_platform_data *)arg;

    //PORT->IRFLT->TIMER

    ir_input_io_sel(user_data->irflt_io);

    ir_output_timer_sel(user_data->timer);

    irflt_config();

    log_irflt_info();

    request_irq(IRQ_TIME0_IDX, IRQ_IRTMR_IP, irtmr_ir_isr, 0);

    return 0;
}
#else

static u8 ir_input_io;
int irflt_init(void *node, void *arg)
{
    //timer1
    ir_log("ir key init >>>\n");

    struct irflt_platform_data *user_data = (struct irflt_platform_data *)arg;

    request_irq(IRQ_IRTMR, IRQ_IRTMR_IP, irtmr_ir_isr, 0);
    //PORT->IRFLT->TIMER
    /* set_ir_clk(); */
    /* return 0; */
    ir_input_io = user_data->irflt_io;
    ir_input_io_sel(user_data->irflt_io);

    ir_output_timer_sel(user_data->timer);

    irflt_config();

    log_irflt_info();


    return 0;
}

#endif
/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_irkey_value(void)
{
    u8 tkey = 0xff;
    if (ir_code.bState != 32) {
        return tkey;
    }
    if ((((u8 *)&ir_code.wData)[0] ^ ((u8 *)&ir_code.wData)[1]) == 0xff) {
        /* if (ir_code.wUserCode == 0xFF00) */
        {
            /* log_info("<%d>",(u8)ir_code.wData); */
#if FPGA
            tkey = IRTabFF00[(u8)ir_code.wData];
#else
            tkey = (u8)ir_code.wData;
#endif
        }
    } else {
        ir_code.bState = 0;
    }
    return tkey;
}

void get_irflt_value(void)
{
    ir_log("irkey = 0x%x", get_irkey_value());
}

u8 ir_busy_check(void)
{
    return ir_busy;
}



/**********ir_key 测试函数***********/
#if 0
const struct device_operations irflt_dev_ops = {
    .init 	= irflt_init,
};
#endif


#define CONFIG_IRFLT_TEST
#ifdef CONFIG_IRFLT_TEST
IRFLT_PLATFORM_DATA_BEGIN(irflt_data)
.irflt_io 		= IO_PORTA_08,
     /* .irflt_io 		= IO_PORTA_09, */
     .timer 			= SEL_IRTMR,
            IRFLT_PLATFORM_DATA_END()
            /* REGISTER_DEVICE(irflt_dev) = { */
            /* .name 			= "irflt", */
            /* .ops 			= &irflt_dev_ops, */
            /* .priv_data 		= (void *) &irflt_data, */
            /* }; */
#endif
// *INDENT-ON*


            int irflt_test(void)
{
    /* JL_ANA->HTC_CON |= BIT(6); */
    /* JL_ANA->HTC_CON |= BIT(7); */
    irflt_init(0, (void *)&irflt_data);
    while (1) {
        delay(100);
        /* if(IRTMR->CON & BIT(7)) */
        /* { */
        /* IRTMR->CON |= BIT(6); */
        /* ir_log("ir pnd\n");     */

        /* } */
        /* delay(1000000); */
        /* __asm__ volatile("mov %0,icfg" : "=r"(tmp)); */
        /* log_info("%02x\n",tmp); */
        wdt_clear();
    }
    return 0;
}


