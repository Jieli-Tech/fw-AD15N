// *INDENT-OFF*
cd /d %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe

if exist %OBJDUMP% (
set NAME=sdk
)

if exist %NAME%.elf (

%OBJDUMP% -section-headers %NAME%.elf
%OBJDUMP% -d -print-imm-hex -print-dbg %NAME%.elf > %NAME%.lst

%OBJCOPY% -O binary -j .app_code %NAME%.elf  %NAME%.bin
%OBJCOPY% -O binary -j .data %NAME%.elf  data.bin
%OBJCOPY% -O binary -j .debug_data %NAME%.elf  debug_data.bin
%OBJDUMP% -section-headers  %NAME%.elf
%OBJDUMP% -t %NAME%.elf > %NAME%.symbol.txt

copy /b %NAME%.bin+data.bin+debug_data.bin app.bin
copy app.bin toy/app.bin

)


cd toy
isd_download.exe -tonorflash -dev sh55 -boot 0x0 -div8 -wait 300 -uboot uboot.boot  -uboot_compress -app app.bin 0x20000 -res dir_a dir_song dir_eng dir_poetry dir_story dir_notice dir_midi midi_cfg -wflash dir_song 0 [PA05_1B_NULL]

@REM
@rem -format vm
@rem -format all
@rem -reboot 500


@REM //烧写外置flash 命令说明：
@rem -wflash dir_song 0 [PA05_1B_NULL]
@rem //        dir_song : 要烧写的文件名（文件需在download.bat文件夹下）
@rem //        0        : 文件烧录到外置flash的起始地址
@rem //        [PA05_1B_NULL]: PA05：外置flash片选引脚（注意：不能选USBDP/USBDM）
@rem //                        1B  ：spi1 ,B端口
@rem //                        NULL: power_io，连接到外置flash vcc引脚 可控制flash电源（例:PA00, 不能选USBDP/USBDM）
@rem //注意：spi端口只能选B/C（B:PA11,PA12,PA10[CLK,DO,DI]; C:PA4,PA5,PA6[CLK,DO,DI]）




ping /n 2 127.1>null
IF EXIST null del null


