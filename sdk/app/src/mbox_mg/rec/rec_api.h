#ifndef _REC_API_H
#define _REC_API_H
typedef struct __enc_file_info {
    void *pfs;
    void *pfile;
    void *device_hdl;
} enc_file_info_t;

void *enc_file_open(const char *folder, const char *filename);
void enc_file_close(void *enc_hdl, u8 enc_bp_save_flg);
#endif
