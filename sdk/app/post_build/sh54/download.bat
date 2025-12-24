
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

%OBJCOPY% -O binary -j .app_code %NAME%.elf %NAME%.bin
%OBJCOPY% -O binary -j .data %NAME%.elf data.bin
%OBJCOPY% -O binary -j .debug_data %NAME%.elf debug_data.bin
%OBJCOPY% -O binary -j .lowpower_overlay %NAME%.elf lowpower_overlay.bin
%OBJDUMP% -section-headers %NAME%.elf
%OBJDUMP% -t %NAME%.elf > %NAME%.symbol.txt

copy /b %NAME%.bin+data.bin+debug_data.bin+lowpower_overlay.bin app.bin
copy app.bin mbox_mg/app.bin

)


cd mbox_mg
isd_download.exe -tonorflash -dev sh54 -boot 0xb00 -div8 -wait 300 -uboot uboot.boot -uboot_compress -app app.bin 0x38000

@REM
@rem -format vm
@rem -format all
@rem -reboot 500


@REM
@rem -wflash dir_song 0 [PA05_1B_NULL]
@rem
@rem
@rem
@rem
@rem
@rem
@rem
@rem
@rem


@REM
fw_add.exe -noenc -fw jl_isd.fw -add ota.bin -type 100 -out jl_isd.fw
ufw_maker.exe -fw_to_ufw jl_isd.fw
copy jl_isd.ufw update.ufw
del jl_isd.ufw


ping /n 2 127.1>null
IF EXIST null del null
