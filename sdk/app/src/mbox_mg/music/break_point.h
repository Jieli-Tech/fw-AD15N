/*************************************************************/
/** @file:break_point.h
    @brief:
    @details:
    @author:Juntham
    @date: 2012-07-25,15:24
    @note:
*/
/*************************************************************/
#ifndef __BREAK_POINT_H__
#define __BREAK_POINT_H__
#include "config.h"

typedef struct _dev_bp_vm_idx_t {
    u8 index;
    u8 bp;
} dev_bp_vm_idx_t;

typedef struct _dev_vm_index_t {
    u32 file_index;
    u32 sclust;
    u8 bp_flag;/*1:had bp info  0:no bp info*/
} dev_vm_index_t;

void clear_music_break_point(u8 dev);
void save_music_break_point(u8 dev, u8 mode);
bool load_music_break_point(u8 dev, void *bp_info, u32 *sclust);
void load_music_last_file(u8 dev);
void save_music_break_point_halfsec_refresh(u8 dev);
void save_rec_break_point(u32 sclust, u32 file_index);

#endif

