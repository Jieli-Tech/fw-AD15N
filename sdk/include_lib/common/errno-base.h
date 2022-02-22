#ifndef _ASM_GENERIC_ERRNO_BASE_H
#define _ASM_GENERIC_ERRNO_BASE_H

#define ENONE        0  /* Err None */

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

#define	E_PFILE_NULL  (0X8000 + 1)
#define	E_PFS_NULL    (0X8000 + 2)
#define	E_PPATH_NULL  (0X8000 + 3)
#define	E_FSDIR_CRC   (0X8000 + 4)
#define	E_FILEINDEX   (0X8000 + 5)
#define	E_FILE_NAME   (0X8000 + 6)
#define	E_DIR_NULL    (0X8000 + 7)
#define	E_NO_FS       (0X8000 + 8)
#define	E_NO_FSFUNC   (0X8000 + 9)
#define	E_NO_VFS      (0X8000 + 10)
#define	E_VFS_HDL     (0X8000 + 11)
#define	E_VFILE_HDL   (0X8000 + 12)
#define	E_VFS_OPS     (0X8000 + 13)
#define	E_FS_PFILE    (0X8000 + 14)
#define E_DEV_NULL	  (0X8000 + 15)
#define	E_SEEK_END    (0X8010 + 0)
#define	E_SEEK_START  (0X8010 + 1)
#define	E_PFILE_CURR  (0X8010 + 2)
#define E_PNAME_LEN   (0x8010 + 3)
#define E_PFILE_NOEXT (0x8010 + 4)
#define E_PEXT_NULL   (0x8010 + 5)

#define E_DECODER     (0X8100 + 1)
#define E_PARG_NULL   (0x8100 + 2)

#define E_MIO_NO_MEN  (0x8200 + 1)
#define E_MIO_READ    (0x8200 + 2)
#define E_MIO_LOGO    (0x8200 + 3)
#define E_MIO_VER     (0x8200 + 4)
#define E_MIO_CHL     (0x8200 + 5)
#define E_MIO_END     (0x8200 + 6)
#define E_MIO_LOCATE  (0x8200 + 7)
#define E_MIO_LEVEL   (0x8200 + 8)

#define E_F1A_INDEX	  (0X9000 + 0)
#define E_F1A_DBUF	  (0X9000 + 1)
#define E_F1A_FORMAT  (0X9000 + 2)
#define E_F1A_TYPE	  (0X9000 + 3)
#define E_F1A_OUTRGE  (0X9000 + 3)


#define E_MIDI_INDEX  (0X9010 + 0)
#define E_MIDI_DBUF	  (0X9010 + 1)
#define E_MIDIFORMAT  (0X9010 + 2)
#define E_MIDI_TYPE	  (0X9010 + 3)
#define E_MIDI_NO_CFG (0X9010 + 4)
#define E_MIDI_FILEHDL (0X9010 + 5)

#define E_A_INDEX	  (0X9020 + 0)
#define E_A_DBUF	  (0X9020 + 1)
#define E_A_FORMAT    (0X9020 + 2)
#define E_A_TYPE	  (0X9020 + 3)

#define E_UMP3_INDEX	  (0X9030 + 0)
#define E_UMP3_DBUF	  (0X9030 + 1)
#define E_UMP3_FORMAT  (0X9030 + 2)
#define E_UMP3_TYPE	  (0X9030 + 3)

#define E_MP3_ST_INDEX	  (0X9040 + 0)
#define E_MP3_ST_DBUF	  (0X9040 + 1)
#define E_MP3_ST_FORMAT   (0X9040 + 2)
#define E_MP3_ST_TYPE	  (0X9040 + 3)

#define E_WAV_INDEX	  	(0X9050 + 0)
#define E_WAV_DBUF	  	(0X9050 + 1)
#define E_WAV_FORMAT   	(0X9050 + 2)
#define E_WAV_TYPE	  	(0X9050 + 3)

#define E_F1X_NO_HLEN  (0X9060 + 1)
#define E_F1X_NO_ULEN  (0X9060 + 2)
#define E_F1X_HU_ZERO  (0X9060 + 3)
#define E_F1X_HU_ERR   (0X9060 + 4)
#define E_F1X_U2LONG   (0X9060 + 5)
#define E_F1X_FORMAT   (0X9060 + 6)

#define E_ADCANA_PARA (0xA000 + 1)
#define E_ADC_SR      (0xA000 + 2)
#define E_ADC_BUF     (0xA000 + 3)

#define E_BSP_EVENT   (0xB000 + 1)

#endif
