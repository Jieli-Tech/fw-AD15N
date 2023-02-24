#include "key.h"
#include "msg.h"
#include "decoder/sine_play.h"
#if KEY_IO_EN
#include "key_drv_io.h"
#endif
#if KEY_AD_EN
#include "key_drv_ad.h"
#endif
#if KEY_MATRIX_EN
#include "key_matrix.h"
#endif
#if KEY_IR_EN
#include "key_ir.h"
#endif
#if KEY_TOUCH_EN
#include "key_touch.h"
#endif


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

static u16(*key_msg_filter)(u8, u8, u8) = NULL;

static const key_interface_t *key_list[] = {
#if KEY_IO_EN
    &key_io_info,
#endif
#if KEY_AD_EN
    &key_ad_info,
#endif
#if KEY_MATRIX_EN
    &key_matrix_info,
#endif
#if KEY_IR_EN
    &key_ir_info,
#endif
#if KEY_TOUCH_EN
    &key_touch_info,
#endif
};

void key_table_sel(void *msg_filter)
{
    local_irq_disable();
    key_msg_filter = msg_filter;
    local_irq_enable();
}

void key_init(void)
{
    key_puts("key init\n");

#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN || KEY_IR_EN || KEY_TOUCH_EN)
    for (int i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++) {
        if (key_list[i]->key_init) {
            key_list[i]->key_init();
        }
    }
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   按键-消息转换函数
   @param   key_status：按键状态
   @param   key_num：最后按键值
   @param   key_type：按键值类型
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
static void key2msg_emit(u8 key_status, u8 key_num, u8 key_type)
{
    key_printf("key_status:%d key_num:%d key_type:%d \n", key_status, key_num, key_type);

    if (key_msg_filter) {
        u16 msg = key_msg_filter(key_status, key_num, key_type);
        if (msg != NO_MSG) {
#if (defined(KEY_VOICE_EN) && KEY_VOICE_EN)
            d_key_voice_kick();
#endif
            int ret = post_msg(1, msg);
            if (ret != 0) {
                key_printf("error !!! msg pool full \n");
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   按键-消息转换函数,按键产生顺序：短按抬起/长按-->连按
   @param
   @param
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
key_io_t get_key_value(void)
{
    u8 i;
    key_io_t key;
    key.key_type = NO_KEY;

#if (KEY_IO_EN || KEY_AD_EN || KEY_MATRIX_EN || KEY_IR_EN || KEY_TOUCH_EN)
    for (i = 0; i < (sizeof(key_list) / sizeof(key_list[0])); i++) {
        if (key_list[i]->key_get_value) {
            key.key_num = key_list[i]->key_get_value();

            if (NO_KEY != key.key_num) {
                key.key_type = key_list[i]->key_type;
                /* log_printf("get_key_num = %d\n", key_num); */
                return key;
            }
        }
    }
#endif

    key.key_num = NO_KEY;
    return key;
}

/*----------------------------------------------------------------------------*/
/**@brief   按键-消息转换函数,按键产生顺序：短按抬起/长按-->连按
   @param
   @param
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
void key_scan()
{
    static u8 key_type = NO_KEY;        ///<按键类型
    static u8 last_key = NO_KEY;
    static u8 key_press_counter = 0;
    u8 cur_key = 0, key_status = 0, back_last_key = 0;
    key_io_t key;

#if (KEY_DOUBLE_CLICK_EN)
    static u8 key_press_flag = 0;
    static u8 double_last_key = 0;
    if (key_press_flag) {
        key_press_flag++;
        if (key_press_flag > KEY_DOUBLE_CLICK_CNT) {
            key_puts(" One_Click ");
            key_press_flag = 0;
            key2msg_emit(KEY_SHORT_UP, double_last_key, key_type);
        }
    }
#endif

    cur_key = NO_KEY;
    back_last_key = last_key;
    key = get_key_value();
    cur_key = key.key_num;
    if (key.key_type != NO_KEY) {
        key_type = key.key_type;
    }

    if (cur_key == last_key) {                          //长时间按键
        if (cur_key == NO_KEY) {
            return;
        }
        key_press_counter++;
        if (key_press_counter == KEY_SHORT_CNT) {
            key_status = KEY_SHORT;
        } else if (key_press_counter == KEY_LONG_CNT) {      //长按
            key_status = KEY_LONG;
        } else if (key_press_counter == (KEY_LONG_CNT + KEY_HOLD_CNT)) {    //连按
            key_status = KEY_HOLD;
            key_press_counter = KEY_LONG_CNT;
        } else {
            return;
        }
    } else { //cur_key = NO_KEY, 抬键
        last_key = cur_key;
        if ((key_press_counter > KEY_SHORT_CNT) && (key_press_counter < KEY_LONG_CNT) && (cur_key == NO_KEY)) {   //短按抬起
            key_press_counter = 0;
            key_status = KEY_SHORT_UP;
        } else if ((cur_key == NO_KEY) && (key_press_counter >= KEY_LONG_CNT)) { //长按抬起
            key_press_counter = 0;
            key_status = KEY_LONG_UP;
            //puts("[UP]");
        } else {
            key_press_counter = 0;
            return;
        }
    }

#if (KEY_DOUBLE_CLICK_EN)
    if (key_status == KEY_SHORT_UP) {
        if (key_press_flag == 0) {
            key_press_flag = 1;
            double_last_key = back_last_key;
        }
        if ((key_press_flag > 15) && (key_press_flag <= KEY_DOUBLE_CLICK_CNT)) {
            //key_puts(" Double_Click ");
            key_press_flag = 0;
            key2msg_emit(KEY_DOUBLE, double_last_key, key_type);
        }
    } else
#endif
    {
        key2msg_emit(key_status, back_last_key, key_type);
    }
}

