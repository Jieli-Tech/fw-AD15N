
#ifndef _FAT_H_
#define _FAT_H_

#include "typedef.h"
// #include "common.h"

// #define LOG_TAG_CONST       NORM
// #define LOG_TAG             "[normal]"
#include "log.h"

#define FS_DEBUG    1

#if FS_DEBUG
// #define fs_puts				puts
// #define fs_printf			printf
// #define fs_puts				printf_buf
#define fs_printf			log_info
#else
#define fs_puts(a)
#define fs_printf(...)
#endif

#define FAT_ONLY_SUPPORT_FAT32  1
#if (FAT_ONLY_SUPPORT_FAT32 == 0)
#define FAT_EXFAT_ENABLE 0
#endif


#define D_MAX_DEEPTH        1
#define D_LFN_MAX_SIZE 		512
#define LFN_MAX_SIZE        D_LFN_MAX_SIZE

#define MAX_DEEPTH          D_MAX_DEEPTH		/* 0~9 deepth of system */

#define SZDIRE 			    (32)
#define DIR_BUFF_LEN   	    (SZDIRE*(2+4))

#define  UL1_SHIFT(sh)      (1L << (sh))
#define  UL1_SHIFT_R(sh)    (1L << (sh)) - 1

/* File access control and file status flags (FIL.flag) */
#define FA_OPEN_EXISTING	0x00

#define FA__ERROR           0x80            //文件错误
#define FDISK__ERROR		0x40            //设备错误

#define DDE					0xE5	/* Deleted directory entry mark in DIR_Name[0] */
#define NDDE				0x05	/* Replacement of the character collides with DDE */

#define SZ_DIR              32

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#define _DF1S               1
#if _DF1S		/* DBCS configuration */
#ifdef _DF2S	/* Two 1st byte areas */
#define IsDBCS1(c)	(((u8)(c) >= _DF1S && (u8)(c) <= _DF1E) || ((u8)(c) >= _DF2S && (u8)(c) <= _DF2E))
#else			/* One 1st byte area */
//#define IsDBCS1(c)	((u8)(c) >= _DF1S && (u8)(c) <= _DF1E)
#define IsDBCS1(c)	((u8)(c) < ' ' && (u8)(c) > '~')
#endif
#ifdef _DS3S	/* Three 2nd byte areas */
#define IsDBCS2(c)	(((u8)(c) >= _DS1S && (u8)(c) <= _DS1E) || ((u8)(c) >= _DS2S && (u8)(c) <= _DS2E) || ((u8)(c) >= _DS3S && (u8)(c) <= _DS3E))
#else			/* Two 2nd byte areas */
#define IsDBCS2(c)	(((u8)(c) >= _DS1S && (u8)(c) <= _DS1E) || ((u8)(c) >= _DS2S && (u8)(c) <= _DS2E))
#endif
#else			/* SBCS configuration */
#define IsDBCS1(c)	0
#define IsDBCS2(c)	0
#endif /* _DF1S */

/* FAT sub-type boundaries */
/* Note that the FAT spec by Microsoft says 4085 but Windows works with 4087! */
#define	MIN_FAT32	65526	/* Minimum number of clusters for FAT32 */

#define ROOT_DIR    0

/* FAT sub type */
#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3
#define FS_EXFAT	4

/* File attribute bits for directory entry */
#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_FCH	0x80    /* exFAT下,文件簇连续标志 */
#define AM_FRG	0x40    /* exFAT下,文件簇BITMAP 有碎片 */
#define AM_LFN0_START	0x1
#define AM_LFN0_LEN	    10
#define AM_LFN1_START	0xE
#define AM_LFN1_LEN	    12
#define AM_LFN2_START	0x1C
#define AM_LFN2_LEN	    4
#define AM_LFN_LEN	    26
#define AM_EX_DIR_STRETCHED 0x08

/* Offset of FAT structure members */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec_l	11
#define BPB_BytsPerSec_h	12
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82
#define BS_FileSysTypeexFAT				5
#define BPB_FatOffset					80
#define BPB_FatLength					84
#define BPB_ClusterHeapOffset			88
#define BPB_ClusterCount				92
#define BPB_FirstClusterOfRootDirectory	96
#define BPB_VolumeFlags					106
#define BPB_BytesPerSectorShift			108
#define BPB_SectorsPerClusterShift		109
#define BPB_NumberOfFats				110
#define MBR_Table			            446
#define	FSI_LeadSig			0	/* FSI: Leading signature (4) */
#define	FSI_StrucSig		484	/* FSI: Structure signature (4) */
#define	FSI_Free_Count		488	/* FSI: Number of free clusters (4) */
#define	FSI_Nxt_Free		492	/* FSI: Last allocated cluster (4) */

///for FAT12/FAT16/FAT32
#define	DIR_Name			0	/* Short file name (11) */
#define	DIR_Attr			11	/* Attribute (1) */
#define	DIR_NTres			12	/* NT flag (1) */
#define DIR_CrtTimeTenth	13	/* Created time sub-second (1) */
#define	DIR_CrtTime			14	/* Created time (2) */
#define	DIR_CrtDate			16	/* Created date (2) */
#define DIR_LstAccDate		18	/* Last accessed date (2) */
#define	DIR_FstClusHI		20	/* Higher 16-bit of first cluster (2) */
#define	DIR_WrtTime			22	/* Modified time (2) */
#define	DIR_WrtDate			24	/* Modified date (2) */
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28


typedef struct {
    u8 win[512];
    u32 sector;
} SWIN_BUF;

/* File system object structure */
struct _FATFS {
    u32		fatbase;		/* FAT start sector */
    u32		dirbase;		/* Root directory start sector */
    u32		database;		/* Data start sector */
    u32 	n_fatent;		/* Maximum cluster# + 1 */
    u32     total_file;		/* 当前设备的匹配文件总数*/
    u16		n_rootdir;		/* Number of root directory entries */
    u32     boot_sect;
    u8		s_size;			/* sector size, 2 power n*/
    u8		csize;			/* Number of sectors per cluster,2 power n */
    u8		fs_type;		/* FAT sub type */
    u8		sector_512size;	/* size of per sector */
    SWIN_BUF wbuf;
    u8      *DEV_No;
};
typedef struct _FATFS FATFS;

struct _FS_NAME {
    char    *lfn;             //长文件名buffer，长度LFN_MAX_SIZE×2，（前LFN_MAX_SIZE）为长文件名，（后LFN_MAX_SIZE）为长文件夹名
    char    *ldn;             //长文件名buffer，长度LFN_MAX_SIZE×2，（前LFN_MAX_SIZE）为长文件名，（后LFN_MAX_SIZE）为长文件夹名
    u16     lfn_cnt;
    u16     ldn_cnt;
};
typedef struct _FS_NAME FS_NAME;

typedef struct {
    u8      attrib;
    u32     sclust;
    u32 fsize;
} FFOBJID;

typedef struct {
    u32     n_frag;//新申请到的簇连续时的计数。
    u32     n_cont;
} W_DIRINFO;

typedef struct {
    FFOBJID obj;
    W_DIRINFO wdir;
    u16     cindex;
    u32 	clust;		        /* Current cluster */
    u32		csect;		        /* Current sector */
} DIR;

typedef struct {
    FATFS   *fs;
    FFOBJID obj;
    W_DIRINFO wdir;
    DIR     dj;
    char 	fname[16];
} DIRINFO;

/* Directory object structure */
typedef struct {
    u32 	clust;		        /* Current cluster */
    u32		csect;		        /* Current sector */
    u16     cindex;
} DIR_INDEX;


/* File system API info */
typedef struct _FSMSG {
    DIRINFO dinfo;
    DIR     dj[MAX_DEEPTH];
    u32	    musicdir_counter;    // 包含指定文件的文件夹序号
    u32		dir_counter;		 // 文件夹序号
    u32     file_total_indir;     // 当前目录的根下有效文件的个数
    u32     file_total_outdir;	// 当前目录前的文件总数，目录循环模式下，需要用它来计算文件序号
    u32     file_number;
    u32     file_counter;
    u8      deepth;
} FSMSG;

typedef struct {
    FATFS   *fs;
    FSMSG   fs_msg;
    u32 fptr;			// File R/W pointer
    u32		csect;			// Current sector
    u32 	clust;			// Current cluster
    u32     start_clust;
    u32     end_clust;
    u8      flag;               // File status flags
    FS_NAME	fs_n;
    char    *lfn;
    u8      lfn_len;
} FIL;


/* File function return code (FRESULT) */
typedef enum {
    FR_OK = 0,
    FR_FIND_DIR = 0x80,
    FR_FIND_FILE,
    FR_DIR_END,         //前面几个位置不能变
    FR_NO_FILE,
    FR_NO_PATH,
    FR_EXIST,
    FR_INVALID_NAME,
    FR_INVALID_DRIVE,
    FR_DENIED,
    FR_RW_ERROR,
    FR_WRITE_PROTECTED,
    FR_NO_FILESYSTEM,
    FR_DEEP_LIMIT,
    FR_END_PATH,
    FR_FILE_LIMIT,
    FR_END_FILE,
    FR_LFN_ERR,
    FR_MKFS_ABORTED,
    FR_MKFP_ABORTED,
    FR_DIR_DELETE,
    FR_DISK_ERROR,
    FR_FILE_END,
    FR_FILE_ERR,
    FR_NO_WINBUF,
    FR_INT_ERR,				/* (2) Assertion failed */
    FR_NO_SEL_DRIVE,
    FR_PART_PATH_OK,
    FR_ALL_PATH_OK,
} FRESULT;


int fat_init(u8 *DEV_No);
int f_read(FIL *fp, u8 *buff, u16 btr);
FRESULT f_seek(FIL *fp, u32 offsize);
FRESULT f_open(FIL *fp, char *path, u8 mode);
extern FIL pfi;//文件信息结构体实体


#endif /* FAT */



