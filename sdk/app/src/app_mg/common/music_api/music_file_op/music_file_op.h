#ifndef _MUSIC_FILE_OP_H
#define _MUSIC_FILE_OP_H

u32 musci_file_open_bypath(void **ppvfs, void **ppvfile, const char *path, void *device, void *type);
u32 musci_file_open_bydirindex(void **ppvfs, void **ppvfile, const char *path, u32 *index, void *device, void *type);
u32 music_file_reopen_byindex(void **ppvfs, void **ppvfile, u32 *index, u8 dir);
u32 music_file_reopen_bypath(void **ppvfs, void **ppvfile, const char *path);
void music_file_close(void **ppvfs, void **ppvfile);
u32 music_file_get_total(void *pvfile);
u32 music_file_name(void *pvfile, void *name, u32 len);

#endif
