#include "key_matrix.h"
#include "key.h"
#include "adc_drv.h"

#define LOG_TAG_CONST       KEYM
#define LOG_TAG             "[keym]"
#include "log.h"

#if KEY_MATRIX_EN
#define ARRAY_MEMS(array)  (sizeof(array)/sizeof(array[0]))
#define IO_X_OUT_Y_AD		0
#define IO_X_AD_Y_OUT		1

static const u8 x_adc_ch[] = X_ADC_CH_INIT;
static const u8 y_port[] = Y_PORT_INIT;
static volatile u16 key_map_table[ARRAY_MEMS(y_port)];

static volatile u16 key_map_x = 0;
static volatile u16 key_map_y = 0;
static volatile u8 scan_ptr = 0;

static bool matrix_key_change_is_over(void)
{
    if (scan_ptr >= ARRAY_MEMS(y_port)) {
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------*/
/**@brief  矩阵按键x方向IO和y方向IO初始化
   @param  初始化类型：x输出1，y为ad输入  或者  x为ad输入，y输出1
   @return NULL
   @note
*/
/*----------------------------------------------------------------------------*/
static void matrix_key_io_set(u8 scan_ptr)
{

    if (scan_ptr >= ARRAY_MEMS(y_port)) {
        return;
    }

    for (int i = 0; i < ARRAY_MEMS(y_port); i++) {
        if (i == scan_ptr) {
            ///输出1
            gpio_set_direction(y_port[i], 0);
            gpio_set_pull_down(y_port[i], 0);
            gpio_set_pull_up(y_port[i], 0);
            gpio_write(y_port[i], 1);
            continue;
        }

        ///输入高阻
        gpio_set_direction(y_port[i], 1);
        gpio_set_pull_down(y_port[i], 0);
        gpio_set_pull_up(y_port[i], 0);
        gpio_write(y_port[i], 0);
    }
}

static void matrix_key_adc_cb(void)
{
    ///save value
    for (int i = 0; i < ARRAY_MEMS(x_adc_ch); i++) {
        u16 adc_value = adc_get_value(x_adc_ch[i]);
        if (adc_value == ADC_VALUE_NONE) {
            continue;
        }

        if (adc_value >= MATRIX_KEY_THRESHOLD) {
            key_map_table[scan_ptr] |= BIT(i);
            //printf("i:%d scan_ptr:%d \n",i,scan_ptr);
        } else {
            key_map_table[scan_ptr] &= ~BIT(i);
        }
    }

    if (++scan_ptr >= ARRAY_MEMS(y_port)) {
        return;
    }

    matrix_key_io_set(scan_ptr);
    adc_kick_start(matrix_key_adc_cb);
}

static void matrix_key_scan_once(void)
{
    scan_ptr = 0;
    matrix_key_io_set(scan_ptr);
    adc_kick_start(matrix_key_adc_cb);
}

/*----------------------------------------------------------------------------*/
/**@brief   按键去抖函数，输出稳定键值
   @param   key：键值
   @return  稳定按键
   @note    u8 key_filter(u8 key)
*/
/*----------------------------------------------------------------------------*/
static u8 key_filter(u8 key)
{
    static u8 used_key = NO_KEY;
    static u8 old_key;
    static u8 key_counter;

    if (old_key != key) {
        key_counter = 0;
        old_key = key;
    } else {
        key_counter++;
        if (key_counter == KEY_BASE_CNT) {
            used_key = key;
        }
    }
    return used_key;
}

/*----------------------------------------------------------------------------*/
/**@brief   matrix key get value
   @param   NULL
   @return  NULL
   @note
*/
/*----------------------------------------------------------------------------*/
u8 get_matrixkey_value(void)
{
    static u8 temp_key_num = NO_KEY;
    u8 key_num = NO_KEY;
    u8 x_value = 0, y_value = 0;

    if (matrix_key_change_is_over()) {
        for (int i = 0; i < ARRAY_MEMS(key_map_table); i++) {
            for (int j = 0; j < sizeof(key_map_table[0]) * 8; j++) {
                if (key_map_table[i] & BIT(j)) {
                    key_num = i * ARRAY_MEMS(x_adc_ch) + j;
                }
            }
        }
        temp_key_num = key_num;
    } else {
        key_num = temp_key_num;
    }

    matrix_key_scan_once();

    //key_printf("matrix key_num:%d \n",key_num);
    return key_filter(key_num);
}

/*----------------------------------------------------------------------------*/
/**@brief   matrix key init
   @param   NULL
   @return  NULL
   @note
*/
/*----------------------------------------------------------------------------*/
void matrix_key_init(void)
{
    key_printf("matrix key init \n");

    memset((u8 *)key_map_table, 0x00, sizeof(key_map_table));
    ///x 轴io init
    for (int i = 0; i < ARRAY_MEMS(x_adc_ch); i++) {
        gpio_set_die(adc_ch2port(x_adc_ch[i]), 0);
        gpio_set_direction(adc_ch2port(x_adc_ch[i]), 1);
        gpio_set_pull_down(adc_ch2port(x_adc_ch[i]), 1);
        gpio_set_pull_up(adc_ch2port(x_adc_ch[i]), 0);

        adc_add_sample_ch(x_adc_ch[i]);
    }

    matrix_key_scan_once();
}

const key_interface_t key_matrix_info = {
    .key_type = KEY_TYPE_MATRIX,
    .key_init = matrix_key_init,
    .key_get_value = get_matrixkey_value,
};

#endif
