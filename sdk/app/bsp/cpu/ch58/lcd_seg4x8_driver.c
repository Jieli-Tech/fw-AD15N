#include "includes.h"
/* #include "key_drv_io.h" */
#include "app_config.h"
#include "lcd_seg4x8_driver.h"
#include "asm/power_interface.h"

#if LCD_4X8_EN

/* #define LOG_TAG_CONST       NORM */
#define LOG_TAG             "[LCDC]"
#include "log.h"

struct ui_lcd_seg4x8_env {
    u8 init;
    LCD_SEG4X8_VAR lcd_seg_var;
    u8 mode;
    const struct lcd_seg4x8_platform_data *user_data;
};

static struct ui_lcd_seg4x8_env _lcd_seg_env = {0};
#define __this      (&_lcd_seg_env)


#define LED_A   BIT(0)
#define LED_B   BIT(1)
#define LED_C   BIT(2)
#define LED_D   BIT(3)
#define LED_E   BIT(4)
#define LED_F   BIT(5)
#define LED_G   BIT(6)
#define LED_H   BIT(7)
//数字'0' ~ '9'显示段码表
static const  u8 LCD_SEG_NUMBER_2_SEG[10] = {
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F),         //'0'
    (u8)(LED_B | LED_C),                                         //'1'
    (u8)(LED_A | LED_B | LED_D | LED_E | LED_G),                 //'2'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_G),                 //'3'
    (u8)(LED_B | LED_C | LED_F | LED_G),                         //'4'
    (u8)(LED_A | LED_C | LED_D | LED_F | LED_G),                 //'5'
    (u8)(LED_A | LED_C | LED_D | LED_E | LED_F | LED_G),         //'6'
    (u8)(LED_A | LED_B | LED_C),                                 //'7'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G), //'8'
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_F | LED_G),         //'9'
};

//字母'A' ~ 'Z'显示段码表
static const  u8 LCD_SEG_LARGE_LETTER_2_SEG[26] = {
    0x77, 0x40, 0x39, 0x3f, 0x79, ///<ABCDE
    0x71, 0x40, 0x76, 0x06, 0x40, ///<FGHIJ
    0x40, 0x38, 0x40, 0x37, 0x3f, ///<KLMNO
    0x73, 0x40, 0x50, 0x6d, 0x78, ///<PQRST
    0x3e, 0x3e, 0x40, 0x76, 0x40, ///<UVWXY
    0x40 ///<Z
};

//字母'a' ~ 'z'显示段码表
static const  u8 LCD_SEG_SMALL_LETTER_2_SEG[26] = {
    0x77, 0x7c, 0x58, 0x5e, 0x79, ///<abcde
    0x71, 0x40, 0x40, 0x40, 0x40, ///<fghij
    0x40, 0x38, 0x40, 0x54, 0x5c, ///<klmno
    0x73, 0x67, 0x50, 0x40, 0x78, ///<pqrst
    0x3e, 0x3e, 0x40, 0x40, 0x40, ///<uvwxy
    0x40 ///<z
};

//=================================================================================//
//                              与IC芯片相关配置                                  //
//=================================================================================//
struct HW_PIN2SEG {
    enum HW_SEG_INDEX seg_index;
    u8 pin_index;
};

struct HW_PIN2COM {
    enum HW_COM_INDEX com_index;
    u8 pin_index;
};

static const struct HW_PIN2SEG hw_pin2seg_mapping[32] = {
    {LCD_SEG0, IO_PORTC_04},
    {LCD_SEG1, IO_PORTC_05},
    {LCD_SEG2, IO_PORTC_06},
    {LCD_SEG3, IO_PORTC_07},
    {LCD_SEG4, IO_PORTC_08},
    {LCD_SEG5, IO_PORTC_09},
    {LCD_SEG6, IO_PORTC_10},
    {LCD_SEG7, IO_PORTC_11},
    {LCD_SEG8, IO_PORTC_12},
    {LCD_SEG9, IO_PORTC_13},
    {LCD_SEG10, IO_PORTC_14},
    {LCD_SEG11, IO_PORTC_15},

    {LCD_SEG12, IO_PORTD_00},
    {LCD_SEG13, IO_PORTD_01},
    {LCD_SEG14, IO_PORTD_02},
    {LCD_SEG15, IO_PORTD_03},
    {LCD_SEG16, IO_PORTD_04},//
    {LCD_SEG17, IO_PORTD_05},//
    {LCD_SEG18, IO_PORTD_06},//
    {LCD_SEG19, IO_PORTD_07},
    {LCD_SEG20, IO_PORTD_08},
    {LCD_SEG21, IO_PORTD_09},
    {LCD_SEG22, IO_PORTD_10},
    {LCD_SEG23, IO_PORTD_11},
    {LCD_SEG24, IO_PORTD_12},
    {LCD_SEG25, IO_PORTD_13},
    {LCD_SEG26, IO_PORTD_14},
};

static const struct HW_PIN2COM hw_pin2com_mapping[6] = {
    {LCD_COM0, IO_PORTC_00},
    {LCD_COM1, IO_PORTC_01},
    {LCD_COM2, IO_PORTC_02},
    {LCD_COM3, IO_PORTC_03},
    {LCD_COM4, IO_PORTC_04},//seg0
    {LCD_COM5, IO_PORTC_05},//seg1
};

static enum HW_COM_INDEX __match_com_index(u8 gpio)
{
    for (u8 i = 0; i < 6; i++) {
        if (hw_pin2com_mapping[i].pin_index == gpio) {
            return hw_pin2com_mapping[i].com_index;
        }
    }

    return 0;
}

static enum HW_SEG_INDEX __match_seg_index(u8 gpio)
{
    for (u8 i = 0; i < 32; i++) {
        if (hw_pin2seg_mapping[i].pin_index == gpio) {
            return hw_pin2seg_mapping[i].seg_index;
        }
    }

    return 0;
}

//LCD COM & SEG
static void __lcd_seg4x8_show_segN(enum HW_COM_INDEX com, enum HW_SEG_INDEX seg, u8 value)
{
    enum HW_COM_INDEX com_index;
    enum HW_SEG_INDEX seg_index;

    com_index = __match_com_index(__this->user_data->pin_cfg.pin_com[com]); //To CHIP connect
    seg_index = __match_seg_index(__this->user_data->pin_cfg.pin_seg[seg]); //To CHIP connect

    if (com_index < 4) {
        if (value) {
            p33_or_1byte(P3_SEG0_DAT0 + com_index * 4 + seg_index / 8, BIT(seg_index % 8));
        } else {
            p33_and_1byte(P3_SEG0_DAT0 + com_index * 4 + seg_index / 8, (u8)(~BIT(seg_index % 8)));
        }
    } else {
        if (value) {
            p33_or_1byte(P3_SEG4_DAT0 + (com_index - 4) * 4 + seg_index / 8, BIT(seg_index % 8));
        } else {
            p33_and_1byte(P3_SEG4_DAT0 + (com_index - 4) * 4 + seg_index / 8, (u8)(~BIT(seg_index % 8)));
        }
    }
}


static void __lcd_seg4x8_reflash_char(u8 index)
{
    //show char
    if (index >= 4) {
        return;
    }

    if (BIT(index) & __this->lcd_seg_var.bFlashChar) {
        return;
    }

    for (u8 i = 0; i < 7; i++) {
        if (__this->lcd_seg_var.bShowBuff[index] & BIT(i)) {
            __lcd_seg4x8_show_segN(lcd_seg4x8_seg2pin[i + index * 7].com_index,
                                   lcd_seg4x8_seg2pin[i + index * 7].seg_index, 1);
        } else {
            __lcd_seg4x8_show_segN(lcd_seg4x8_seg2pin[i + index * 7].com_index,
                                   lcd_seg4x8_seg2pin[i + index * 7].seg_index, 0);
        }
    }
}

static void __lcd_seg4x8_reflash_string()
{
    u8 j = 0;
    for (j = 0; j < 4; j++) {
        __lcd_seg4x8_reflash_char(j);
    }
}

//刷新icon常亮显示
static void __lcd_seg4x8_reflash_icon()
{
    //show icon
    u8 i = 0;
    u8 j = 0;
    for (j = 0; j < 32; j++) {
        if (BIT(j) & __this->lcd_seg_var.bFlashIcon) {
            continue;
        }
        if (BIT(j) & __this->lcd_seg_var.bShowIcon) {
            for (i = 0; i < ARRAY_SIZE(lcd_seg4x8_icon_seg2pin); i++) { //lookup icon exist
                if (BIT(j) == lcd_seg4x8_icon_seg2pin[i].icon) {
                    //look up the seg2pin table, set the pin should be output 0
                    __lcd_seg4x8_show_segN(lcd_seg4x8_icon_seg2pin[i].seg2pin.com_index,
                                           lcd_seg4x8_icon_seg2pin[i].seg2pin.seg_index, 1);
                }
            }
        } else {
            for (i = 0; i < ARRAY_SIZE(lcd_seg4x8_icon_seg2pin); i++) { //lookup icon exist
                if (BIT(j) == lcd_seg4x8_icon_seg2pin[i].icon) {
                    //look up the seg2pin table, set the pin should be output 0
                    __lcd_seg4x8_show_segN(lcd_seg4x8_icon_seg2pin[i].seg2pin.com_index,
                                           lcd_seg4x8_icon_seg2pin[i].seg2pin.seg_index, 0);
                }
            }
        }
    }
}

//刷新icon闪烁显示
static void __lcd_seg4x8_flash_show_icon(u8 is_on)
{
    //show/off icon
    u8 i = 0;
    u8 j = 0;
    if (__this->lcd_seg_var.bFlashIcon) {
        for (j = 0; j < 32; j++) {
            if (BIT(j) & __this->lcd_seg_var.bFlashIcon) {
                for (i = 0; i < ARRAY_SIZE(lcd_seg4x8_icon_seg2pin); i++) { //lookup icon exist
                    if (BIT(j) == lcd_seg4x8_icon_seg2pin[i].icon) {
                        //look up the seg2pin table, set the pin should be output 0
                        if (is_on) {
                            __lcd_seg4x8_show_segN(lcd_seg4x8_icon_seg2pin[i].seg2pin.com_index,
                                                   lcd_seg4x8_icon_seg2pin[i].seg2pin.seg_index, 1);
                        } else {
                            __lcd_seg4x8_show_segN(lcd_seg4x8_icon_seg2pin[i].seg2pin.com_index,
                                                   lcd_seg4x8_icon_seg2pin[i].seg2pin.seg_index, 0);
                        }
                    }
                }
            }
        }
    }
}


static void __lcd_seg4x8_flash_show_char(u8 is_on)
{
    //show/off icon
    u8 i = 0;
    u8 j = 0;
    for (j = 0; j < 4; j++) {
        if (__this->lcd_seg_var.bFlashChar & BIT(j)) {
            if (is_on) {
                for (u8 i = 0; i < 7; i++) {
                    if (__this->lcd_seg_var.bShowBuff[j] & BIT(i)) {
                        __lcd_seg4x8_show_segN(lcd_seg4x8_seg2pin[i + j * 7].com_index,
                                               lcd_seg4x8_seg2pin[i + j * 7].seg_index, 1);
                    }
                }
            } else {
                for (u8 i = 0; i < 7; i++) {
                    __lcd_seg4x8_show_segN(lcd_seg4x8_seg2pin[i + j * 7].com_index,
                                           lcd_seg4x8_seg2pin[i + j * 7].seg_index, 0);
                }
            }
        }
    }
}

void __lcd_seg4x8_flash_show_timer_handle()
{
    static u8 flash_on = 0;
    if (__this->lcd_seg_var.bFlashIcon) {
        __lcd_seg4x8_flash_show_icon(flash_on);
    }
    if (__this->lcd_seg_var.bFlashChar) {
        __lcd_seg4x8_flash_show_char(flash_on);
    }
    flash_on = !flash_on;
}



void __lcd_seg4x8_reflash_screen()
{
    __lcd_seg4x8_reflash_string();
    __lcd_seg4x8_reflash_icon();
}


static void __lcd_seg4x8_clear_screen()
{
    //clear all
    u8 p33_addr = P3_SEG0_DAT0; //com0~3
    for (; p33_addr <= P3_SEG3_DAT3; p33_addr++) {
        p33_tx_1byte(p33_addr, 0);
    }
    p33_addr = P3_SEG4_DAT0; //com4~5
    for (; p33_addr <= P3_SEG5_DAT3; p33_addr++) {
        p33_tx_1byte(p33_addr, 0);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   lcd_seg 单个字符显示函数
  @param   chardata：显示字符
  @return  void
  @note    void __lcd_seg4x8_show_char(u8 chardata)
  @ display:
       ___    ___    ___    ___
      |___|  |___|  |___|  |___|
      |___|  |___|  |___|  |___|
     ---0------1------2------3----> X
 */
/*----------------------------------------------------------------------------*/
static void __lcd_seg4x8_show_char(u8 chardata)
{
    if (__this->lcd_seg_var.bCoordinateX >= 4) {
        __this->lcd_seg_var.bCoordinateX = 0; //or return
        //return ;
    }
    if ((chardata >= '0') && (chardata <= '9')) {
        __this->lcd_seg_var.bShowBuff[__this->lcd_seg_var.bCoordinateX++] = LCD_SEG_NUMBER_2_SEG[chardata - '0'];
    } else if ((chardata >= 'a') && (chardata <= 'z')) {
        __this->lcd_seg_var.bShowBuff[__this->lcd_seg_var.bCoordinateX++] = LCD_SEG_SMALL_LETTER_2_SEG[chardata - 'a'];
    } else if ((chardata >= 'A') && (chardata <= 'Z')) {
        __this->lcd_seg_var.bShowBuff[__this->lcd_seg_var.bCoordinateX++] = LCD_SEG_LARGE_LETTER_2_SEG[chardata - 'A'];
    } else if (chardata == ':') {
        __this->lcd_seg_var.bShowIcon |= LCD_SEG4X8_1POINT;
        __this->lcd_seg_var.bFlashIcon &= (~LCD_SEG4X8_1POINT);
    } else if (chardata == '.') {
        __this->lcd_seg_var.bShowIcon |= LCD_SEG4X8_2POINT;
        __this->lcd_seg_var.bFlashIcon &= (~LCD_SEG4X8_2POINT);
    } else if (chardata == ' ') {
        __this->lcd_seg_var.bShowBuff[__this->lcd_seg_var.bCoordinateX++] = 0;
    } else {
        __this->lcd_seg_var.bShowBuff[__this->lcd_seg_var.bCoordinateX++] = LED_G; //显示'-'
    }

    /* __lcd_seg4x8_reflash_char(__this->lcd_seg_var.bCoordinateX - 1); */
}

///////////////// API:
void lcd_seg4x8_setX(u8 X)
{
    __this->lcd_seg_var.bCoordinateX = X;
}

void lcd_seg4x8_clear_string(void)
{
    memset(__this->lcd_seg_var.bShowBuff, 0x00, 4);
    __this->lcd_seg_var.bFlashChar = 0;

    lcd_seg4x8_setX(0);
    __lcd_seg4x8_reflash_string();
}

void lcd_seg4x8_show_char(u8 chardata)
{
    __lcd_seg4x8_show_char(chardata);
}

void lcd_seg4x8_show_string(u8 *str)
{
    while (*str != '\0') {
        __lcd_seg4x8_show_char(*str++);
    }
}

void lcd_seg4x8_show_icon(UI_LCD_SEG4X8_ICON icon)
{
    __this->lcd_seg_var.bShowIcon |= icon;
    __this->lcd_seg_var.bFlashIcon &= (~icon); //stop display
    __lcd_seg4x8_reflash_icon();
}

void lcd_seg4x8_flash_icon(UI_LCD_SEG4X8_ICON icon)
{
    __this->lcd_seg_var.bFlashIcon |= icon;
    __this->lcd_seg_var.bShowIcon &= (~icon); //stop display
    __lcd_seg4x8_reflash_icon();
}

void lcd_seg4x8_flash_char_start(u8 index)
{
    if (index < 4) {
        __this->lcd_seg_var.bFlashChar |= BIT(index);
    }
}

void lcd_seg4x8_flash_char_stop(u8 index)
{
    if (index < 4) {
        __this->lcd_seg_var.bFlashChar &= ~BIT(index);
    }
}

void lcd_seg4x8_clear_icon(UI_LCD_SEG4X8_ICON icon)
{
    __this->lcd_seg_var.bShowIcon &= (~icon);
    __this->lcd_seg_var.bFlashIcon &= (~icon);
    __lcd_seg4x8_reflash_icon();
}

void lcd_seg4x8_clear_all_icon(void)
{
    __this->lcd_seg_var.bFlashIcon = 0;
    __this->lcd_seg_var.bShowIcon = 0;
    __lcd_seg4x8_reflash_icon();
}

void lcd_seg4x8_show_null(void)
{
    lcd_seg4x8_clear_string();
    lcd_seg4x8_clear_all_icon();
}


/*----------------------------------------------------------------------------*/
/**@brief   字符串显示函数, 默认左对齐, 从x = 0开始显示
  @param   *str：字符串的指针
  @return  void
  @note    void lcd_seg_show_string_left(u8 *str)
 */
/*----------------------------------------------------------------------------*/
void lcd_seg4x8_show_string_reset_x(u8 *str)
{
    lcd_seg4x8_clear_string();
    while (*str != '\0') {
        __lcd_seg4x8_show_char(*str++);
    }
    __lcd_seg4x8_reflash_screen();
}

void lcd_seg4x8_show_string_align_right(u8 *str)
{
    u8 cnt = 0;
    u8 *_str = str;
    while (*_str++ != '\0') {
        cnt++;
    }
    if (cnt > 4) {
        return;
    }
    lcd_seg4x8_clear_string();
    lcd_seg4x8_setX(4 - cnt);//right
    /* lcd_seg4x8_setX(0);//left */
    while (*str != '\0') {
        lcd_seg4x8_show_char(*str++);
    }
    __lcd_seg4x8_reflash_screen();
}

void lcd_seg4x8_show_string_align_left(u8 *str)
{
    lcd_seg4x8_show_string_reset_x(str);
}

static const u8 asc_number[] = "0123456789";    ///<0~9的ASCII码表

/*----------------------------------------------------------------------------*/
/**@brief  获取一个4位十进制的数的各个位
   @param  i:输入的一个4位十进制的数
   @return 无
   @note   void itoa4(u8 i, u8 *buf)
*/
/*----------------------------------------------------------------------------*/
static void itoa4(u16 i, u8 *buf)
{
    buf[0] = asc_number[(i % 10000) / 1000]; //千
    buf[1] = asc_number[(i % 1000) / 100]; //百
    buf[2] = asc_number[(i % 100) / 10]; //十
    buf[3] = asc_number[i % 10]; //个
}

void lcd_seg4x8_show_number(u16 val)
{
    u8 tmp_buf[5] = {0};

    if (val > 9999) {
        lcd_seg4x8_show_icon(LCD_SEG4X8_1POINT);
    } else {
        lcd_seg4x8_clear_icon(LCD_SEG4X8_1POINT);
    }
    itoa4(val, tmp_buf);
    lcd_seg4x8_show_string_reset_x(&tmp_buf[0]);
}

//数字显示函数, 高位不显示0
void lcd_seg4x8_show_number2(u16 val)
{
    u8 i;
    u8 tmp_buf[5] = {0};

    if (val > 9999) {
        lcd_seg4x8_show_icon(LCD_SEG4X8_1POINT);
    } else {
        lcd_seg4x8_clear_icon(LCD_SEG4X8_1POINT);
    }

    itoa4(val, tmp_buf);
    for (i = 0; i < 3; i++) {
        if (tmp_buf[i] != '0') {
            break;
        }
    }
    lcd_seg4x8_show_string_align_right((u8 *) & (tmp_buf[i + 0]));
}

//数字显示函数(追加方式)
void lcd_seg4x8_show_number_add(u16 val)
{
    u8 i;
    u8 tmp_buf[5] = {0};

    if (__this->lcd_seg_var.bCoordinateX == 0) {
        if (val > 9999) {
            lcd_seg4x8_show_icon(LCD_SEG4X8_1POINT);
        } else {
            lcd_seg4x8_clear_icon(LCD_SEG4X8_1POINT);
        }
    }

    itoa4(val, tmp_buf);
    for (i = 0; i < 4; i++) {
        if (tmp_buf[i] != '0') {
            break;
        }
    }
    lcd_seg4x8_show_string((u8 *)&tmp_buf[i]);
}

static void lcd_con_dump()
{
    log_info("LCDC->CON0 = 0x%x", p33_rx_1byte(P3_LCDC_CON0));
    log_info("LCDC->CON1 = 0x%x", p33_rx_1byte(P3_LCDC_CON1));
    log_info("LCDC->CON2 = 0x%x", p33_rx_1byte(P3_LCDC_CON2));
    log_info("LCDC->CON3 = 0x%x", p33_rx_1byte(P3_LCDC_CON3));
    /* log_info("LCDC->CON4 = 0x%x", p33_rx_1byte(P3_LCDC_CON4)); */
    log_info("LCDC->CON5 = 0x%x", p33_rx_1byte(P3_LCDC_CON5));
    log_info("LCDC->CON6 = 0x%x", p33_rx_1byte(P3_LCDC_CON6));
    log_info("LCDC->CON7 = 0x%x", p33_rx_1byte(P3_LCDC_CON7));
    log_info("LCDC->SEG_IO_EN0 = 0x%x", p33_rx_1byte(P3_SEG_IO_EN0));
    log_info("LCDC->SEG_IO_EN1 = 0x%x", p33_rx_1byte(P3_SEG_IO_EN1));
    log_info("LCDC->SEG_IO_EN2 = 0x%x", p33_rx_1byte(P3_SEG_IO_EN2));
    log_info("LCDC->SEG_IO_EN3 = 0x%x", p33_rx_1byte(P3_SEG_IO_EN3));
    log_info("LCDC->COM_IO_EN = 0x%x", p33_rx_1byte(P3_LCDC_CON4));
    log_info("LCDC->CLK_CON   = 0x%x", p33_rx_1byte(P3_CLK_CON0));

    u32 p33_lcd_segx_dat = 0;
    for (u8 i = P3_SEG0_DAT0; i < P3_SEG3_DAT3; i += 4) {
        p33_lcd_segx_dat = p33_rx_1byte(i + 3) << 24;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 2) << 16;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 1) << 8;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 0) << 0;
        log_info("P33 LCDC-SEG%d_DAT = 0x%x", (i & 0x0f) / 4, p33_lcd_segx_dat);
        p33_lcd_segx_dat = 0;
    }
    for (u8 i = P3_SEG4_DAT0; i < P3_SEG5_DAT3; i += 4) {
        p33_lcd_segx_dat = p33_rx_1byte(i + 3) << 24;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 2) << 16;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 1) << 8;
        p33_lcd_segx_dat |= p33_rx_1byte(i + 0) << 0;
        log_info("P33 LCDC-SEG%d_DAT = 0x%x", (i & 0x0f) / 4 + 4, p33_lcd_segx_dat);
        p33_lcd_segx_dat = 0;
    }
}

void lcd_seg4x8_seg_init()
{
    for (int j = 0; j < 8; j++) {
        /* gpio_set_mode(IO_PORT_SPILT(__this->user_data->pin_cfg.pin_seg[j]), PORT_HIGHZ); */
        gpio_set_direction(__this->user_data->pin_cfg.pin_seg[j], 1);
        gpio_set_die(__this->user_data->pin_cfg.pin_seg[j], 0);
        gpio_set_dieh(__this->user_data->pin_cfg.pin_seg[j], 0);
        gpio_set_pull_up(__this->user_data->pin_cfg.pin_seg[j], 0);
        gpio_set_pull_down(__this->user_data->pin_cfg.pin_seg[j], 0);
    }
}

void lcd_seg4x8_com_init()
{
    for (int i = 0; i < 4; i++) {
        /* gpio_set_mode(IO_PORT_SPILT(__this->user_data->pin_cfg.pin_com[i]), PORT_HIGHZ); */
        gpio_set_direction(__this->user_data->pin_cfg.pin_com[i], 1);
        gpio_set_die(__this->user_data->pin_cfg.pin_com[i], 0);
        gpio_set_dieh(__this->user_data->pin_cfg.pin_com[i], 0);
        gpio_set_pull_up(__this->user_data->pin_cfg.pin_com[i], 0);
        gpio_set_pull_down(__this->user_data->pin_cfg.pin_com[i], 0);
    }
}
#define LCD_CHG_CON0_INIT \
        /* EN             1bit  */ (0<<0) | \
        /* LDOB_EN        1bit  */ (0<<1) | \
        /* HFREQ_EN       1bit  */ (1<<2) | \
        /* COMP_EN        1bit  */ (1<<3) | \
        /* PUMPCLK_EN     1bit  */ (1<<4) | \
        /* ADC_DET_EN     1bit  */ (0<<5) | \
        /* STANDBY_SETB   1bit  */ (1<<6) | \
        /* X2MODE_SETB    1bit  */ (1<<7)

static void lcd_pump_init(void)
{
    gpio_write(IO_PORTD_15, 1);//vlcd io
    gpio_set_direction(IO_PORTD_15, 0);
    udelay(500);

    LCDCP_WVBG_EN(1);//LCDCP_WVBG_EN
    /* p33_or_1byte(P3_CHG_PUMP, BIT(1));//1:lcdp wvbg en */
    udelay(500);

    p33_tx_1byte(P3_LCD_CHG_CON0, LCD_CHG_CON0_INIT);
    LCDCP_OSCCAP_SEL(LCDCP_OSCCAP_SEL_1P2M);
    LCDCP_PUMPVOL_SEL(LCDCP_PUMPVOL_SEL_3P8V);//__this->usr_data->chgpump_vol);
    LCDCP_LDOB_SEL(LCDCP_LDOB_SEL_3P3V);//__this->usr_data->ldob_vol);
    udelay(500);

    gpio_set_direction(IO_PORTD_15, 1);
    gpio_set_die(IO_PORTD_15, 0);
    gpio_set_dieh(IO_PORTD_15, 0);
    gpio_set_pull_up(IO_PORTD_15, 0);
    gpio_set_pull_down(IO_PORTD_15, 0);
    LCDCP_PUMP_EN(1);
    udelay(1500);

    LCDCP_LDOB_EN(1);
}

static u8 lcd_reuse_flag = 0;
//32k-->15ms,30
___interrupt
void lcdc_isr(void)
{
    if (p33_rx_1byte(P3_LCDC_CON3) & BIT(7)) {
        p33_or_1byte(P3_LCDC_CON3, BIT(6));    //
        lcd_reuse_flag = 1;
    }
    p33_or_1byte(P3_LCDC_CON3, BIT(4));    //sw_kst
}
//cnt=f/1040(s),t=515000/f(ms)
/*----------------------------------------------------------------------------*/
/**@brief   LCD段码屏初始化
  @param   void
  @return  void
  @note    void lcd_seg4x8_init(const struct lcd_SEG4X8_platform_data *user_data)
 */
/*----------------------------------------------------------------------------*/
void lcd_seg4x8_init(const struct lcd_seg4x8_platform_data *user_data)
{
    u8 temp = 0;
    if ((__this->init == true) || (user_data == NULL)) {
        return;
    }

    memset(__this, 0x00, sizeof(struct ui_lcd_seg4x8_env));
    __this->user_data = user_data;
    for (temp = P3_LCDC_CON0; temp <= P3_LCDC_CON7; temp++) {
        p33_tx_1byte(temp, 0);    //
    }
    for (temp = P3_SEG_IO_EN0; temp <= P3_SEG_IO_EN3; temp++) {
        p33_tx_1byte(temp, 0);    //
    }

    lcd_pump_init();

    //P3_CLK_CON0[7:6]
    //00b: osl_clk(rtc32k);
    //01b: wclk;
    //10b: lrc_clk;
    //11b: pat_clk;
    p33_and_1byte(P3_CLK_CON0, 0x1f);     //LCD_clk
    p33_or_1byte(P3_CLK_CON0, BIT(6));     //LCD_clk sel:wclk(32k)
    /* p33_or_1byte(P3_CLK_CON0, BIT(7));     //LCD_clk sel:lrc200k(162) */
    u8 p3_clk = p33_rx_1byte(P3_CLK_CON0) & 0xc0;
    if (p3_clk == 0) {//osc
        p33_or_1byte(P3_OSL_CON, BIT(0));//X32k en
        p33_or_1byte(P3_OSL_CON, BIT(1));//X32K HD
    } else if (p3_clk == 0x80) { //lrc200k
        p33_or_1byte(P3_LCDC_CON2, 0b0100); //clk div5
    }
    p33_or_1byte(P3_CLK_CON0, BIT(5));     //LCD_clk_EN

    lcd_seg4x8_seg_init();              //seg set
    lcd_seg4x8_com_init();              //com set
    //板级配置参数
    p33_or_1byte(P3_LCDC_CON0, user_data->bias << 2); //P3_LCDC_CON0[3:2]:0:close, 0x01:1/2, 0x02:1/3, 0x03:1/4
    /* JL_LCDC->CON0 |= (0b00 << 28);    //[28:29]: bias current selection 0~3:200na~500na*/
    p33_or_1byte(P3_LCDC_CON0, user_data->vlcd << 4); //P3_LCDC_CON0[6:4]: VLCDS

    //固定配置参数
    /* JL_LCDC->CON0 |= (1 << 7);              //[7]: 帧频率控制, 默认使用 32KHz / 64 */
    p33_or_1byte(P3_LCDC_CON1, 0b01);          //P3_LCDC_CON1[1:0]: CHGMOD一直用强充电模式
    /* JL_LCDC->CON0 |= (1 << 30);             //high drive EN0 */
    /* JL_LCDC->CON0 |= (1 << 31);          //high drive EN1 */
    /* p33_or_1byte(P3_LCDC_CON1, 0b0000<< 4);//P3_LCDC_CON1[7:4]: CHGMOD交替充电模式下cycle */
    /* p33_or_1byte(P3_LCDC_CON2, BIT(5));    //P3_LCDC_CON2.5:死区控制使能 */

    //SEG使能
    u8 i = 0;
    u8 j = 0;
    for (j = 0; j < 8; j++) {
        for (i = 0; i < ARRAY_SIZE(hw_pin2seg_mapping); i++) {
            if (user_data->pin_cfg.pin_seg[j] == hw_pin2seg_mapping[i].pin_index) {
                p33_or_1byte(P3_SEG_IO_EN0 + i / 8, BIT(i % 8));
                break;
            }
        }
    }
    //COM使能
    for (j = 0; j < 4; j++) {
        for (i = 0; i < ARRAY_SIZE(hw_pin2com_mapping); i++) {
            if (user_data->pin_cfg.pin_com[j] == hw_pin2com_mapping[i].pin_index) {
                p33_or_1byte(P3_LCDC_CON4, BIT(i));
                break;
            }
        }
    }

    __this->mode = user_data->ctu_en;                       //0:断续推屏 1:连续推屏

    p33_or_1byte(P3_LCDC_CON3, BIT(6));    //clr pnd
    if (__this->mode) {
        p33_and_1byte(P3_LCDC_CON3, (u8)(~BIT(1)));    //ie
        p33_or_1byte(P3_LCDC_CON3, BIT(0));    //
    } else {
        request_irq(IRQ_LCD_IDX, 3, lcdc_isr, 0);
        p33_or_1byte(P3_LCDC_CON3, BIT(1));    //ie
        p33_and_1byte(P3_LCDC_CON3, (u8)(~BIT(0)));
    }

    //跳秒配置 -- 使用OSL_CLK时跳秒才有效
    if (user_data->dot_en) {
        p33_tx_1byte(P3_LCDC_CON5, user_data->dot_seg_com_sel);//
        p33_or_1byte(P3_LCDC_CON0, BIT(1));    //DOT_EN
        p33_or_1byte(P3_OSL_CON, BIT(0));
    }

    p33_or_1byte(P3_LCDC_CON0, BIT(0));    //LCD_EN
    p33_or_1byte(P3_LCDC_CON3, BIT(6));    //clr

    p33_or_1byte(P3_LCDC_CON3, BIT(4));    //sw_kst

    log_info("lcd_seg4x8_init\n");

    __this->init = true;
}


#if 0
//=================================================================================//
//                              FOR TEST CODE                                      //
//=================================================================================//
// *INDENT-OFF*
LCD_SEG4X8_PLATFORM_DATA_BEGIN(lcd_seg4x8_test_data)
    .vlcd = LCD_VOL_SEL_1P000XVLCDCP,
    .bias = LCD_SEG4X8_BIAS_1_3,
    .ctu_en = 1, //0:断续推屏(en ie) 1:连续推屏
    .dot_en = 0, //0:关闭跳秒 1:打开跳秒
    .dot_seg_com_sel = LCD_SEG11 << 3 | LCD_COM3, //seg:7~3; com:2~0
    .pin_cfg.pin_com[0] = IO_PORTC_00,
    .pin_cfg.pin_com[1] = IO_PORTC_01,
    .pin_cfg.pin_com[2] = IO_PORTC_02,
    .pin_cfg.pin_com[3] = IO_PORTC_03,

    .pin_cfg.pin_seg[0] = IO_PORTC_04,
    .pin_cfg.pin_seg[1] = IO_PORTC_05,
    .pin_cfg.pin_seg[2] = IO_PORTC_06,
    .pin_cfg.pin_seg[3] = IO_PORTC_07,
    .pin_cfg.pin_seg[4] = IO_PORTC_08,
    .pin_cfg.pin_seg[5] = IO_PORTC_09,
    .pin_cfg.pin_seg[6] = IO_PORTC_10,
    .pin_cfg.pin_seg[7] = IO_PORTC_11,
LCD_SEG4X8_PLATFORM_DATA_END()

static u32 cnt = 0;
void lcd_seg4x8_test_show()
{
    if (cnt > 9999) {
        cnt = 0;
    }
    /* log_info("cnt = %d\n", cnt); */
    lcd_seg4x8_show_number(cnt);
    cnt += 10;
}

void lcd_seg4x8_test(u16 num)
{
    lcd_seg4x8_init(&lcd_seg4x8_test_data);
    __lcd_seg4x8_clear_screen();
    lcd_seg4x8_show_number(num);
    lcd_seg4x8_show_icon(LCD_SEG4X8_1POINT);
    lcd_seg4x8_show_icon(LCD_SEG4X8_2POINT);
    lcd_seg4x8_show_icon(LCD_SEG4X8_3POINT);
    lcd_con_dump();
    while(1){
        wdt_clear();
        mdelay(500);
        /* lcd_seg4x8_test_show(); */
    }
}
#endif
#if 0
/*                powerdown保持lcd显示                  */
/* 使用该接口进入powerdown时，需要保证推屏时钟源选择LRC */
/*                                                      */
void lcd_powerdown()
{
    OS_ENTER_CRITICAL();
    u8 lcd_ctu_en = 0;
	u8 lcdc_con0 = p33_rx_1byte(P3_LCDC_CON0);
	u8 lcdc_con3 = p33_rx_1byte(P3_LCDC_CON3);
    if (1) {
    /* if (powerdown_lcd_on) { */
        if (lcdc_con0 & BIT(0)) {         //lcd_en
            /* LCD推屏模式断续变成连续 */
            if (!(lcdc_con3 & BIT(0))) {  //ctu_en
                lcd_ctu_en = 1;
				/* log_info("lcdc_con3 0x%x\n", lcdc_con3); */
				p33_and_1byte(P3_LCDC_CON3, (u8)(~BIT(1)));  //lcd_ie
				lcdc_con3 = p33_rx_1byte(P3_LCDC_CON3);
				/* log_info("lcdc_con3 0x%x\n", lcdc_con3); */
        		p33_or_1byte(P3_LCDC_CON3, BIT(0));          //lcd_ie
				/* lcdc_con3 = p33_rx_1byte(P3_LCDC_CON3); */
				/* log_info("lcdc_con3 0x%x\n", lcdc_con3); */
            }
        }
    }
    sys_power_down(-2);//进入powerdown
    if (1) {
    /* if (powerdown_lcd_on) { */
        if (lcd_ctu_en) {
			p33_or_1byte(P3_LCDC_CON3, BIT(1));	  //lcd_ie
			p33_and_1byte(P3_LCDC_CON3, (u8)(~BIT(0)));	  //ctu en
            lcd_ctu_en = 0;
        }
    }
    OS_EXIT_CRITICAL();
}
#endif
#endif

