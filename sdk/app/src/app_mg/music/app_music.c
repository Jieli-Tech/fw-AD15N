#include "config.h"
#include "msg.h"
#include "app_mg/app_mg.h"
#include "app_mg/music/app_music.h"
#include "music_api.h"
#include "vm.h"
#include "asm/power/p33.h"
#include "app_dev_mg.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "debug.h"

#if MUSIC_BREAK_POINT_EN
dp_buff dbuff;
#endif

#define MAX_DIR_NUM				5
#define DIR_MEMS(ppdir) (sizeof(ppdir)/sizeof(ppdir[0]))
static const char dir_inner_flash_table[MAX_DIR_NUM][16] = {
    "dir_song",
    "dir_eng",
    "dir_poetry",
    "dir_story",
    "dir_bin_f1x",
    /* "dir_midi", */
};
static const char *const dir_a = {
    "dir_a",
};
const u8 vm_index_inner_flash_table[MAX_DIR_NUM] = {
    VM_INDEX_SONG,
    VM_INDEX_ENG,
    VM_INDEX_POETRY,
    VM_INDEX_STORY,
    VM_INDEX_F1X,
};

#if (EXT_FLASH_EN)
static const char dir_ext_flash_table[][16] = {
    "/",
};
const u8 vm_index_ext_flash_table[] = {
    VM_INDEX_EXT_SONG,
};
#endif

///dev info
static const char (*dir_table)[16] = dir_inner_flash_table;
static const u8 *vm_index_table = vm_index_inner_flash_table;
static u8 dir_mems = DIR_MEMS(dir_inner_flash_table);
static void *device, *fs_type;

static u8 dir_index = 0;
///music play hdl
static music_play_obj *music_obj = NULL;
static music_play_obj *music_a_obj = NULL;

void music_device_info_switch(u8 dev)
{
    switch (dev) {
    case APP_DEV_INNER_FLASH:
        log_info("device inner flash \n");
        dir_index = 0;
        dir_table = dir_inner_flash_table;
        vm_index_table = vm_index_inner_flash_table;
        dir_mems = DIR_MEMS(dir_inner_flash_table);
        break;
#if (EXT_FLASH_EN)
    case APP_DEV_EXT_FLASH:
        log_info("device extern flash \n");
        dir_index = 0;
        dir_table = dir_ext_flash_table;
        vm_index_table = vm_index_ext_flash_table;
        dir_mems = DIR_MEMS(dir_ext_flash_table);
        break;
#endif
    default:
        break;
    }
}

static u16 music_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_music_table[key_status][key_num];
        break;
#endif
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_music_table[key_status][key_num];
        break;
#endif

    default:
        break;
    }

    return msg;
}

u32 music_bp_to_file_index(void *dbuff)
{
    if (dbuff == NULL) {
        return 1;
    }

    return ((dp_buff *)dbuff)->findex;
}

void music_decode_succ_func(music_play_obj *hdl)
{
    int file_index = 0;
    int total_file_num = 0;
    music_play_io_ctl(hdl, MUSIC_CMD_GET_CUR_FILE_INDEX, (int)&file_index);
    music_play_io_ctl(hdl, MUSIC_CMD_GET_TOTAL_NUM, (int)&total_file_num);

    log_info(">>>>>cur play index:%d  total num:%d <<<<<<<<<<<<<\n", file_index, total_file_num);

#if MUSIC_BREAK_POINT_EN
    ///save break point,下一曲不需要获取断点信息，只要存文件号就可以
    memset((u8 *)&dbuff, 0x00, sizeof(dbuff));
    dbuff.findex = file_index;
    vm_write(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff));
#endif
}

static int app_music_init(void *param)
{
    void *bp_info = NULL;
    u32 file_index = 1;
    dir_index = 0;
    key_table_sel((void *)music_msg_filter);
    decoder_init();
    app_device_ioctl(APP_DEV_CMD_SET, APP_DEV_INNER_FLASH);
    device = (void *)app_device_ioctl(APP_DEV_CMD_GET_HDL, 0);
    fs_type = (void *)app_device_ioctl(APP_DEV_CMD_GET_FS_TYPE, 0);
    music_device_info_switch(app_device_ioctl(APP_DEV_CMD_GET, 0));
    music_decode_succ_cb_regist(music_decode_succ_func);
#if MUSIC_BREAK_POINT_EN
    ///read break point
    if (vm_read(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff)) == sizeof(dbuff)) {
        bp_info = (void *)&dbuff;
        file_index = music_bp_to_file_index(bp_info);
    }
#endif
    if (music_play(&music_obj, dir_table[dir_index], file_index, BIT_UMP3 | BIT_F1A1 | BIT_MIDI, device, MUSIC_MODE_AUTO_NEXT, fs_type, bp_info) == -1) {
        log_info("music_play failed !!! \n");
        return -1;
    } else {
        log_info("music_play succeed !!! \n");
    }
    return 0;
}

__attribute__((noinline))
static int app_music_close(void)
{
    log_info("app_music_close \n");
#if MUSIC_BREAK_POINT_EN
    ///save break point
    if (music_play_get_bp(music_obj, &dbuff) == 0) {
        vm_write(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff));
    }
#endif

    music_play_destroy(&music_obj);
    music_play_destroy(&music_a_obj);
    music_decode_succ_cb_regist(NULL);
    return 0;
}

int app_music(void *param)
{
    int msg[2];
    void *bp_info = NULL;
    u32 file_index = 1;

    log_info("app_music \n");
    if (app_music_init(param) != 0) {
        log_error("app_music init error !!! \n");
        app_switch_next();
        return 0;
    }

    while (1) {
        get_msg(2, &msg[0]);
        bsp_loop();

        if (common_msg_deal(msg) != (-1)) {
            continue;
        }

        switch (msg[0]) {
        case MSG_NEXT_DEV:
            log_info("next device \n");
#if MUSIC_BREAK_POINT_EN
            ///save break point
            if (music_play_get_bp(music_obj, &dbuff) == 0) {
                vm_write(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff));
            }
#endif

            //switch to next device
            app_device_ioctl(APP_DEV_CMD_NEXT, 0);
            device = (void *)app_device_ioctl(APP_DEV_CMD_GET_HDL, 0);
            fs_type = (void *)app_device_ioctl(APP_DEV_CMD_GET_FS_TYPE, 0);
            music_device_info_switch(app_device_ioctl(APP_DEV_CMD_GET, 0));

#if MUSIC_BREAK_POINT_EN
            ///read break point
            if (vm_read(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff)) == sizeof(dbuff)) {
                bp_info = (void *)&dbuff;
                file_index = music_bp_to_file_index(bp_info);
            }
#endif
            ///如果不要断点播放，bp_info 传NULL
            if (music_play(&music_obj, dir_table[dir_index], file_index, BIT_UMP3 | BIT_F1A1 | BIT_MIDI, device, MUSIC_MODE_AUTO_NEXT, fs_type, bp_info) == -1) {
                log_error("music_play failed !!! \n");
            }

            break;
        case MSG_NEXT_FILE:
            log_info("music next file\n");
            music_play_next_file(&music_obj);
            break;
        case MSG_PRIV_FILE:
            log_info("music priv file\n");
            music_play_priv_file(&music_obj);
            break;
        case MSG_PP:
            log_info("music pp\n");
            music_play_pp(music_obj);
            break;
        case MSG_NEXT_DIR:
#if MUSIC_BREAK_POINT_EN
            ///save break point
            if (music_play_get_bp(music_obj, &dbuff) == 0) {
                vm_write(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff));
            }
#endif

            ///music play
            dir_index++;
            (dir_index >= dir_mems) ? (dir_index = 0) : 0;
            log_info("next dir:%s \n", dir_table[dir_index]);
#if MUSIC_BREAK_POINT_EN
            ///read break point
            if (vm_read(vm_index_table[dir_index], (u8 *)&dbuff, sizeof(dbuff)) == sizeof(dbuff)) {
                bp_info = (void *)&dbuff;
                file_index = music_bp_to_file_index(bp_info);
            }
#endif
            ///如果不要断点播放，bp_info 传NULL
            if (music_play(&music_obj, dir_table[dir_index], file_index, BIT_UMP3 | BIT_F1A1 | BIT_MIDI, device, MUSIC_MODE_AUTO_NEXT, fs_type, bp_info) == -1) {
                log_error("music_play failed !!! \n");
            }
            break;
        case MSG_A_PLAY:
            log_info("a play \n");
            if (music_play(&music_a_obj, dir_a, 1, BIT_A, NULL, MUSIC_MODE_PLAY_ONE, FS_TYPE_SYD, NULL) == -1) {
                log_error("music_play failed !!! \n");
            }
            break;

        case MSG_WAV_LOOP:
        case MSG_F1A1_LOOP:
        case MSG_F1A2_LOOP:
        case MSG_MP3_LOOP:
        case MSG_A_LOOP:
            log_info("loop one \n");
            break;

        case MSG_500MS:
            if (music_play_get_status(music_obj) == MUSIC_PLAY || music_play_get_status(music_a_obj) == MUSIC_PLAY) {
                //busy
                idle_check_deal(1);
            } else {
                idle_check_deal(0);
            }
            break;

        case MSG_WAV_FILE_END:
        case MSG_MP3_FILE_END:
        case MSG_F1A1_FILE_END:
        case MSG_F1A2_FILE_END:
        case MSG_MIDI_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_obj));
            music_play_end_operation(&music_obj);
            break;
        case MSG_A_FILE_END:
            log_info("decode end :%d \n", music_play_get_decoder_type(music_a_obj));
            music_play_end_operation(&music_a_obj);
            break;
        case MSG_WAV_FILE_ERR:
        case MSG_F1A1_FILE_ERR:
        case MSG_F1A2_FILE_ERR:
        case MSG_MP3_FILE_ERR:
        case MSG_MIDI_FILE_ERR:
            /* music_play_destroy(&music_obj); */
            music_play_end_operation(&music_obj);
            break;
        case MSG_A_FILE_ERR:
            /* music_play_destroy(&music_a_obj); */
            music_play_end_operation(&music_a_obj);
            break;

        case MSG_NEXT_MODE:
            app_switch_next();
            break;
        case MSG_APP_SWITCH_ACTIVE:
            log_info("MSG_APP_SWITCH_ACTIVE \n");
            if (app_switch_en()) {
                goto _app_switch;
            }
            break;
#ifdef USB_DEVICE_EN
        case MSG_USB_PC_IN:
        case MSG_PC_IN:
            app_switch_to_usb();
            break;

#endif


        default:
            break;
        }
    }

_app_switch:
    app_music_close();
    return 0;
}


