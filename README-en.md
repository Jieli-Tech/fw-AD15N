[tag download]:https://github.com/Jieli-Tech/fw-AD15N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/fw-AD15N?style=plastic&labelColor=ffffff&color=informational&label=Tag&

# fw-AD14N-AD15N-AC104N-AD17N-AD18N  [![tag][tag_badgen]][tag download]

<div align="center">

**Jieli AD14N / AD15N / AC104N / AD17N / AD18N Series Universal MCU SDK Firmware**

[中文](./README.md) · [Documentation Center](https://doc.zh-jieli.com/AD14/zh-cn/master/index.html) · [SDK Release History](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf) · [Report an Issue](https://github.com/Jieli-Tech/fw-AD15N/issues)

</div>

---

## Table of Contents

- [1. Overview](#1-overview)
- [2. Supported Chips and Platforms](#2-supported-chips-and-platforms)
- [3. Environment Setup](#3-environment-setup)
- [4. Quick Start](#4-quick-start)
- [5. Project Structure](#5-project-structure)
- [6. Applications and Examples](#6-applications-and-examples)
- [7. Build Guide](#7-build-guide)
- [8. Flashing and Upgrade](#8-flashing-and-upgrade)
- [9. Configuration](#9-configuration)
- [10. FAQ](#10-faq)
- [11. Community and Support](#11-community-and-support)
- [12. Disclaimer](#12-disclaimer)

---

## 1. Overview

`fw-AD1x-4578_AC104_SDK` is a universal MCU SDK development package provided by Jieli Technology for the AD14N / AD15N / AC104N / AD17N / AD18N series chips. These chips are primarily targeted at the following application scenarios:

| Application Type | Typical Products |
|---------|---------|
| **Voice Toys** | Storytelling machines, learning devices, voice-controlled toys, MIDI instruments |
| **General MCU** | Smart control, sensor acquisition, general peripheral applications |
| **Mini Speakers** | Music players, FM radios, voice recorders, loudspeakers |

### Core Features

- **Audio Decoding**: Supports playback of .a/.b/.e, .f1a/.f1b/.f1c, and other audio formats
- **MIDI Playback**: Supports MIDI synthesis and playback
- **Audio Encoding**: Supports A/MP3/UMP3 encoding
- **Multi-channel Playback**: Up to two simultaneous audio decode streams
- **Variable Speed/Pitch**: Supports variable-speed and pitch-shifted audio playback (requires system clock ≥ 100 MHz)
- **Hardware Resampling**: Built-in hardware resampling
- **Low Power**: Shutdown current as low as 1.7 µA+
- **Multiple Storage Options**: Supports internal/external FLASH with FAT/NORFS/SYDF file systems
- **DAC Output**: Supports PWM differential output and external single-ended amplifiers, supports 8K–32K sample rates

This repository contains SDK release versions and sample projects. Compilation requires the corresponding library files (`lib.a`) that follow the naming convention.

---

## 2. Supported Chips and Platforms

### 2.1 SoC Families

| CPU Platform | Chip Series | Application Domain |
|---------|---------|---------|
| **sh54** | AD14N | Voice Toys / General MCU |
| **sh55** | AD15N | Voice Toys / General MCU |
| **sh57** | AD17N | Voice Toys / General MCU |
| **ch58** | AD18N | Voice Toys / General MCU (supports segment LCD) |
| — | **AC104N** | Mini Speakers (mbox_mg) |

> For chip models, datasheets, and schematic resources, see: [doc/ directory](doc/)

### 2.2 MCU Hardware/Software Parameter Differences

![Chip](jl_ad_chip.png)

---

## 3. Environment Setup

### 3.1 Prerequisites

| System | Description |
|------|------|
| **Windows** | Code::Blocks IDE recommended for compilation |
| **Linux** | Command-line compilation via Makefile (requires rewriting `download_bat.c` for Linux compatibility) |
| **macOS** | Cross-compilation toolchain must be configured manually |

### 3.2 Install the Build Toolchain

1. Download and install the **Jieli Build Toolchain**: [Download Link](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html)
2. Linux users can download from: [pkgman.jieliapp.com](http://pkgman.jieliapp.com/doc/all)
   - Extract to `/opt/jieli` after downloading
   - Ensure `/opt/jieli/pi32/bin/clang` exists
3. Verify installation:

```bash
# Verify toolchain installation
clang --version
```

### 3.3 Install Flashing Tools

| Tool | Purpose | How to Obtain |
|------|------|---------|
| **USB Upgrade Tool** | Flash firmware to the target board | [Application Link](https://item.taobao.com/item.htm?id=620295020803) · [User Guide](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html) |
| **Mass Production Burner** | Mass production / bare-die programming | **From distributor** · [User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) |

---

### 3.4 Audio Tools

Universal audio tools for packaging, audio file conversion, MIDI, etc.: [Download Link](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) Password: `3jey`

---

## 4. Quick Start

### 4.1 Clone the Repository

```bash
git clone https://github.com/Jieli-Tech/fw-AD15N.git
cd fw-AD15N/sdk
```

### 4.2 Project Entry Points

The SDK includes the following application projects, located in the `sdk/` root directory:

| Project File | Chip | Application Type |
|---------|------|---------|
| `AD14N_voice_toy.cbp` | AD14N (sh54) | Voice Toys |
| `AD14N_mcu.cbp` | AD14N (sh54) | General MCU |
| `AD15N_voice_toy.cbp` | AD15N (sh55) | Voice Toys |
| `AD15N_mcu.cbp` | AD15N (sh55) | General MCU |
| `AD17N_voice_toy.cbp` | AD17N (sh57) | Voice Toys |
| `AD17N_mcu.cbp` | AD17N (sh57) | General MCU |
| `AD18N_voice_toy.cbp` | AD18N (ch58) | Voice Toys |
| `AD18N_mcu.cbp` | AD18N (ch58) | General MCU |
| `AC104N_mbox_mg.cbp` | AC104N | Mini Speakers |

### 4.3 Application Code Entry Points

```
sdk/app/src/
├── voice_toy/          # Voice toy application (toy_music / toy_midi / toy_record / toy_linein / toy_speaker / toy_idle / toy_softoff / toy_usb_slave)
├── mbox_mg/            # Mini speaker application (music / fm / rec / line_in / loudspeaker / usb_device)
└── mcu/                # General MCU application
```

### 4.4 Build and Flash

**Method 1: Code::Blocks (Recommended for Windows users)**

1. Double-click the corresponding `.cbp` project file
2. Click **Build → Build** (Ctrl+F9)
3. After a successful build, use the USB Upgrade Tool to flash the generated firmware

**Method 2: Makefile (Command Line)**

```bash
# Windows users
Double-click sdk/tools/make_prompt.bat to open the command-line environment

# Select the corresponding Makefile for compilation
make -f Makefile.ad15n_voice_toy all -j4
make -f Makefile.ad15n_mcu all -j4
```

> **💡 Tip**: Before building, ensure the USB Upgrade Tool is properly connected and the target board has entered programming mode.

> **💡 Tip**: For all supported target names, see the comments at the top of [Makefile](sdk/Makefile).

**Method 3: VS Code Build**

The repository comes pre-configured with VS Code tasks. Press `Ctrl+Shift+B` to select a build target.


---

## 5. Project Structure

```
fw-AD15N/
├── sdk/                           # SDK root directory
│   ├── app/                       # Application layer code
│   │   ├── src/                   #   Application entry source code
│   │   │   ├── voice_toy/         #     Voice toy application
│   │   │   ├── mbox_mg/           #     Mini speaker application
│   │   │   └── mcu/               #     General MCU application
│   │   ├── bsp/                   #   Board Support Package (BSP)
│   │   │   ├── common/            #     Common modules
│   │   │   │   ├── decoder/       #       Decoders (A/F1A/MP3/MIDI/WAV)
│   │   │   │   ├── encoder/       #       Encoders (A/MP3/UMP3)
│   │   │   │   ├── fs/            #       File systems (FAT/NORFS/SYDF)
│   │   │   │   ├── key/           #       Key drivers (IO/AD/Matrix/IR/Touch)
│   │   │   │   ├── usb/           #       USB (device/host/usr)
│   │   │   │   ├── fm/            #       FM Radio (BK1080/QN8035/RDA5807)
│   │   │   │   ├── midi_open/     #       MIDI open-source library
│   │   │   │   ├── sound_effect_list/  #  Audio effects (ANS/Echo/EQ/Speed/Pitch)
│   │   │   │   ├── vm/            #       Virtual memory (new_vm/old_vm)
│   │   │   │   ├── msg/           #       Message mechanism
│   │   │   │   ├── norflash/      #       External Flash driver
│   │   │   │   ├── speaker/       #       Loudspeaker
│   │   │   │   ├── power_manage/  #       Power management
│   │   │   │   ├── rtc/           #       RTC
│   │   │   │   ├── iic_soft/      #       Software I²C
│   │   │   │   ├── spi_soft/      #       Software SPI
│   │   │   │   └── uart_update/   #       UART upgrade
│   │   │   └── cpu/               #     CPU platform drivers
│   │   │       ├── sh54/          #       AD14N (SPI/I²C/UART/ADC/DAC/PWM)
│   │   │       ├── sh55/          #       AD15N (SPI/I²C/UART/ADC/DAC/PWM)
│   │   │       ├── sh57/          #       AD17N (SPI/I²C/UART/ADC/DAC/PWM)
│   │   │       └── ch58/          #       AD18N (SPI/I²C/UART/ADC/DAC/PWM/LCD)
│   │   └── post_build/            #   Post-build scripts and tools
│   ├── include_lib/               # Header files
│   │   ├── driver/                #   Driver headers
│   │   ├── system/                #   System headers
│   │   ├── cpu/                   #   CPU platform headers (sh54/sh55/sh57/ch58)
│   │   ├── decoder/               #   Decoder API headers
│   │   ├── encoder/               #   Encoder API headers
│   │   ├── fs/                    #   File system headers
│   │   ├── audio/                 #   Audio API headers
│   │   ├── device/                #   Device driver headers
│   │   ├── dev_mg/                #   Device management headers
│   │   ├── common/                #   Common headers
│   │   └── liba/                  #   Precompiled libraries (.a)
│   │       ├── ARCH/pi32_lto/     #     General algorithm library
│   │       ├── sh54/              #     AD14N platform library
│   │       ├── sh55/              #     AD15N platform library
│   │       ├── sh57/              #     AD17N platform library
│   │       └── ch58/              #     AD18N platform library
│   ├── tools/                     # Build tools and scripts
│   │   ├── make_prompt.bat        #   Windows build command-line launcher
│   │   └── utils/                 #   Utilities (make, rm, etc.)
│   ├── Makefile                   # Top-level Makefile
│   └── *.cbp                      # Code::Blocks project files
├── doc/                           # Documentation
│   ├── ad14n/                     #   AD14N datasheets / schematics
│   ├── ad15n/                     #   AD15N datasheets / schematics
│   ├── ad17n/                     #   AD17N datasheets / schematics
│   ├── ad18n/                     #   AD18N datasheets / schematics
│   ├── ac104n/                    #   AC104N datasheets / schematics
│   ├── stuff/                     #   Miscellaneous (DingTalk group, USB upgrade tool docs, etc.)
│   └── *.pdf                      #   SDK manual / release notes / selection guide
└── README.md                      # This file
```

### 5.1 Key Directory Descriptions

| Directory | Purpose |
|------|------|
| `sdk/app/src/` | **Application entry**: main functions, message handling, scene switching for each application |
| `sdk/app/bsp/common/` | **Common modules**: decoders, encoders, keys, file systems, USB, FM, MIDI, and other core modules |
| `sdk/app/bsp/cpu/` | **CPU drivers**: peripheral drivers for each platform (GPIO, UART, SPI, I²C, ADC, DAC, PWM, etc.) |
| `sdk/include_lib/liba/` | **Precompiled libraries**: `.a` static library files for each platform |
| `sdk/app/post_build/` | **Download tools**: `isd_download.exe`, `isd_config.ini`, download scripts |
| `doc/` | **Documentation**: chip manuals, schematics, SDK manual, selection guide |

---

## 6. Applications and Examples

### 6.1 Voice Toy Application (`app/src/voice_toy/`)

| Submodule | Description |
|-------|------|
| **toy_music** | Music playback (local/external FLASH file playback) |
| **toy_midi** | MIDI instrument performance and playback |
| **toy_record** | Recording functionality |
| **toy_linein** | Line input (AUX) |
| **toy_speaker** | Loudspeaker / voice amplifier |
| **toy_idle** | Standby / idle handling |
| **toy_softoff** | Soft power-off |
| **toy_usb_slave** | USB slave device |

Target domains: voice toys, storytelling machines, learning devices, MIDI instruments, etc.

### 6.2 General MCU Application (`app/src/mcu/`)

| Item | Description |
|------|------|
| **Target scenarios** | Smart control, sensor acquisition, general peripheral applications |
| **Key peripherals** | UART, I²C, SPI, ADC, DAC, PWM, RTC, GPIO, IR, touch keys |

### 6.3 Mini Speaker Application (`app/src/mbox_mg/`)

| Submodule | Description |
|-------|------|
| **music** | Music playback (FAT/SD/USB file systems) |
| **fm** | FM Radio (supports BK1080/QN8035/RDA5807, etc.) |
| **rec** | Recording mode |
| **line_in** | Line input (AUX) |
| **loudspeaker** | Loudspeaker / voice amplifier |
| **usb_device** | USB device (UAC/CDC/MSD/HID) |

Target domains: portable speakers, music players, FM radios, etc.

---

## 7. Build Guide

### 7.1 Build Command Quick Reference

Run the following commands from the `sdk/` directory:

| Target | Chip | Command |
|------|------|------|
| **Voice Toys** | AD14N | `make -f Makefile.ad14n_voice_toy all -j4` |
| **Voice Toys** | AD15N | `make -f Makefile.ad15n_voice_toy all -j4` |
| **Voice Toys** | AD17N | `make -f Makefile.ad17n_voice_toy all -j4` |
| **Voice Toys** | AD18N | `make -f Makefile.ad18n_voice_toy all -j4` |
| **General MCU** | AD14N | `make -f Makefile.ad14n_mcu all -j4` |
| **General MCU** | AD15N | `make -f Makefile.ad15n_mcu all -j4` |
| **General MCU** | AD17N | `make -f Makefile.ad17n_mcu all -j4` |
| **General MCU** | AD18N | `make -f Makefile.ad18n_mcu all -j4` |
| **Mini Speakers** | AC104N | `make -f Makefile.ac104n_mbox_mg all -j4` |
| **Build All** | All | `make all` |
| **Clean All** | All | `make clean` |


### 7.2 Code::Blocks Build (Recommended for Windows users)

1. Ensure the Jieli build toolchain is installed
2. Double-click the corresponding `.cbp` project file to open Code::Blocks
3. Click **Build → Build** (Ctrl+F9)
4. After a successful build, the firmware will be generated in the `post_build/` directory

### 7.3 Makefile Build

```bash
# Windows users
Double-click sdk/tools/make_prompt.bat to open the command-line environment
make -f Makefile.ad15n_voice_toy all -j4

# Linux users (requires modifying download_bat.c for Linux compatibility)
cd sdk
make -f Makefile.ad15n_voice_toy all -j`nproc`
```

### 7.4 Common Build Errors

| Error Message | Solution |
|---------|---------|
| `clang: command not found` | Jieli build toolchain is not installed, or environment variables are not configured |
| `cannot find -lxxx` | Missing corresponding `.a` library file; check the `include_lib/liba/` directory |
| `make: command not found` | On Windows, use `tools/make_prompt.bat` to open the build command environment |
| Link errors | Verify that the correct chip Makefile/CBP project is selected |

---

## 8. Flashing and Upgrade

### 8.1 First-Time Flashing

1. **Connect Hardware**: Connect the development board to the PC via **USB** or **USB Upgrade Tool**
2. **Enter Programming Mode**:
    - Method 1 (USB): Hold the flash button on the development board, then reset or power-cycle
    - Method 2 (USB/UART): Use the USB Upgrade Tool to enter programming mode
3. **Launch USB Upgrade Tool**: Start the flashing host software
4. **Select Firmware**: Choose the compiled firmware file
5. **Start Flashing**: Click the download button and wait for completion

> **Note**: Before flashing, ensure the USB Upgrade Tool is properly connected and the target board has entered programming mode. For details on ISD_CONFIG.INI, see [ISD Configuration Guide](https://doc.zh-jieli.com/AD14/zh-cn/master/download_and_tools_introduction/download_introduction.html#).

### 8.2 Mass Production Flashing

For mass production scenarios, use Jieli's mass production burner (one-to-two / one-to-eight), which supports bare-die programming. See [One-to-Two Burner User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) · [One-to-Eight Burner User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo8/index.html)

### 8.3 OTA Upgrade

Dual-bank firmware upgrade (`dual_bank`) is supported. See `sdk/app/bsp/common/dual_bank_demo.c` for details.

---

## 9. Configuration

- Edit `sdk/app/src/<application>/app_config.h` to configure feature toggles for the target application
- Platform-specific configurations for each CPU are located in `sdk/app/src/<application>/<platform>/app_modules.h`

---

## 10. FAQ

### 10.1 Development Workflow

**Q: How do I create a new project?**
A: Start from an existing `.cbp` project and the application code in `app/src/`, then configure the corresponding application examples.

**Q: How do I switch between different chip platforms?**
A: Select the corresponding `.cbp` project or Makefile. The SDK provides pre-configured build entry points for each chip platform.

### 10.2 Build Issues

**Q: On Windows, `make` is reported as an invalid command?**
A: Use `sdk/tools/make_prompt.bat` to enter the pre-configured command-line environment, which sets up all environment variables and the `make` path.

**Q: How can I speed up compilation?**
A: Use the `-j` flag for parallel compilation, e.g. `make -j4` (the number specifies the parallel job count).

### 10.3 Debugging Tips

- **UART Logging**: Debug logs can be output via UART
- **GPIO Debug**: Use idle GPIO pins to output debug waveforms for timing measurement

---

## 11. Community and Support

### Technical Discussion

| Platform | Group / Link | Status |
|------|-----------|------|
| **DingTalk Tech Group** | See [Group QR Code](doc/stuff/dingtalk.jpg) | ✅ Joinable |

### Resource Links

| Resource | Link |
|------|------|
| 📖 **Online Documentation Center** | [doc.zh-jieli.com/AD14](https://doc.zh-jieli.com/AD14/zh-cn/master/index.html) |
| 📚 **SDK Release History** | [SDK Release Notes](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf) |
| 🔧 **SDK Quick Start** | [SDK Manual](doc/AD14N_AD15N_AD17N_AD18N_AC104N_SDK手册.pdf) |
| 📄 **Chip Selection Guide** | [Selection Table](doc/杰理科技AD14_AD15_AD16_AD17_AD18系列语音MCU选型表.pdf) |
| 🎬 **Video Tutorials** | [Bilibili Homepage](https://space.bilibili.com/3493277347088769/dynamic) |
| 🎵 **MIDI Development Manual** | [MIDI Application Development Guide](https://doc.zh-jieli.com/MIDI/zh-cn/master/index.html) |
| 📦 **FAE Support** | [FAE Support Repository](https://gitee.com/jieli-tech_fae/fw-jl) |
| 🛒 **Dev Board / Flashing Tool Purchase** | [Jieli Official Store](https://shop321455197.taobao.com/) |
| 🐛 **Issue Tracker** | [Github Issues](https://github.com/Jieli-Tech/fw-AD15N/issues) |

---

## 12. Disclaimer

`fw-AD1x-4578_AC104_SDK` supports development for the AD14N / AD15N / AC104N / AD17N / AD18N series chips. These chips support common general-purpose MCU applications and may be used for development, evaluation, sampling, and mass production. For the corresponding SDK version, please refer to [SDK Release History](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf).

---

<div align="center">
  <sub>Copyright &copy; Zhuhai Jieli Technology Co., Ltd. All rights reserved.</sub>
</div>
