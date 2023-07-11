#include "config.h"
#include "gpio.h"
#include "irflt.h"
#include "log.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[irflt]"


#define ir_log 		log_info//log_d
extern const int IRQ_IRTMR_IP;

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

JL_TIMER_TypeDef   *IRTMR = JL_TIMER1;
u8 IRQ_IRTMR = IRQ_TIME1_IDX;



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

    IRTMR->CON |= BIT(14);

    bCap1 = IRTMR->PRD;
    IRTMR->CNT = 0;
    cap = bCap1 / irtmr_prd;

    /* ir_log("ir isr verify"); */
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
#define timer_Hz  		16000000L

void set_ir_clk(void)
{
    u32 clk;
    u32 prd_cnt;

    IRTMR->CON = BIT(14);
    IRTMR->PRD = 0;
    IRTMR->CNT = 0;
    SFR(IRTMR->CON, 10, 4, 3); //rc16m (lsb/2以下)
    SFR(IRTMR->CON, 4, 4, 8); //pset=2
    clk = timer_Hz;//clock_get_lsb_freq();
    clk /= (1000 * 256);//1ms for cnt
    prd_cnt = clk;
    irtmr_prd = prd_cnt;

    SFR(IRTMR->CON, 16, 1, 0); //2 edge dis
    SFR(IRTMR->CON, 2, 1, 1); //irflt en
    request_irq(IRQ_IRTMR, IRQ_IRTMR_IP, irtmr_ir_isr, 0);
    SFR(IRTMR->CON, 0, 2, 3); //mode falling edge
}


static void ir_input_io_sel(u8 port)
{
    //选择irflt input channel1输入
    gpio_set_direction(port, 1);
    gpio_set_die(port, 1);
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 0);
    gpio_ich_sel_input_signal(port, INPUT_CH_SIGNAL_IRFLT, INPUT_CH_TYPE_GP_ICH);
}

static void irflt_config()
{

    JL_IR->RFLT_CON = 0;
    JL_IR->RFLT_CON |= 0x90;		//512 div
    JL_IR->RFLT_CON |= 0x0c;		//std24m  512/24=21.3us

    JL_IR->RFLT_CON |= BIT(0);		//irflt enable

    set_ir_clk();
}

static void log_irflt_info()
{
    ir_log("RFLT_CON = 0x%x\n", JL_IR->RFLT_CON);
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
static u8 ir_input_io;
int irflt_init(void *node, void *arg)
{
    //timer1
    ir_log("ir key init >>>\n");
    struct irflt_platform_data *user_data = (struct irflt_platform_data *)arg;
    ir_input_io = user_data->irflt_io;
    if (user_data->timer == SEL_TIMER0) {
        IRTMR = JL_TIMER0;
        IRQ_IRTMR = IRQ_TIME0_IDX;
    } else if (user_data->timer == SEL_TIMER1) {
        IRTMR = JL_TIMER1;
        IRQ_IRTMR = IRQ_TIME1_IDX;
    } else {
        IRTMR = JL_TIMER2;
        IRQ_IRTMR = IRQ_TIME2_IDX;
    }

    ir_input_io_sel(user_data->irflt_io);

    irflt_config();

    log_irflt_info();
    return 0;
}

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
        tkey = (u8)ir_code.wData;
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

#define CONFIG_IRFLT_TEST
#ifdef CONFIG_IRFLT_TEST
IRFLT_PLATFORM_DATA_BEGIN(irflt_data)
.irflt_io 		= IO_PORTA_09,
     .timer 			= SEL_TIMER1,
            IRFLT_PLATFORM_DATA_END()
#endif

            int irflt_test(void)
{
    u8 ir_key = 0xff;
    irflt_init(0, (void *)&irflt_data);
    while (1) {
        delay(100);
        ir_key = get_irkey_value();
        if (ir_key != 0xff) {
            log_info("ir_key:0x%x", ir_key);
            ir_key = 0xff;
            ir_code.bState = 0;
            ir_busy = 0;
            ir_code.wData = 0;
        }
        wdt_clear();
    }
    return 0;
}
#endif
