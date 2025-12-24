// *INDENT-OFF*
cd /d %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set OBJSIZEDUMP=C:\JL\pi32\bin\llvm-objsizedump.exe

if exist %OBJDUMP% (
set NAME=sdk
)

if exist %NAME%.elf (

%OBJDUMP% -section-headers %NAME%.elf
%OBJDUMP% -d -print-imm-hex -print-dbg %NAME%.elf > %NAME%.lst
%OBJSIZEDUMP% -lite -skip-zero -enable-dbg-info %NAME%.elf > %NAME%.txt

%OBJCOPY% -O binary -j .app_code %NAME%.elf  %NAME%.bin
%OBJCOPY% -O binary -j .data %NAME%.elf  data.bin
%OBJCOPY% -O binary -j .debug_data %NAME%.elf  debug_data.bin
%OBJCOPY% -O binary -j .lowpower_overlay %NAME%.elf  lowpower_overlay.bin
%OBJDUMP% -section-headers  %NAME%.elf
%OBJDUMP% -t %NAME%.elf > %NAME%.symbol.txt

copy /b %NAME%.bin+data.bin+debug_data.bin+lowpower_overlay.bin app.bin
copy app.bin mcu/app.bin

)


cd mcu
isd_download.exe -tonorflash -dev sh57 -boot 0x3f01000 -div8 -wait 300 -uboot uboot.boot -app app.bin

@REM
@rem -format vm
@rem -format all
@rem -reboot 500

@REM //烧写外置flash 命令说明：
@rem -ex_flash dir_song_ext
@rem -ex_flash 烧写到外挂flash命令
@rem dir_song_ext 经过两次打包的资源文件

@rem 打包工具 ：packres.exe -n res -o dir_song_ext dir_song 0 -normal

ping /n 2 127.1>null
IF EXIST null del null


