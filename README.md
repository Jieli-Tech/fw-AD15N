[tag download]:https://github.com/Jieli-Tech/fw-AD15N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/fw-AD15N?style=plastic&labelColor=ffffff&color=informational&label=Tag&

# fw-AD14N-AD15N-AC104N-AD17N-AD18N_GP-MCU_SDK  [![tag][tag_badgen]][tag download]

<div align="center">

**杰理 AD14N / AD15N / AC104N / AD17N / AD18N 系列通用 MCU SDK 固件程序**

[English](./README-en.md) · [文档中心](https://doc.zh-jieli.com/AD14/zh-cn/master/index.html) · [SDK 版本历史](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf) · [报告问题](https://github.com/Jieli-Tech/fw-AD15N/issues)

</div>

---

## 目录

- [一、概述](#一概述)
- [二、支持的芯片与平台](#二支持的芯片与平台)
- [三、环境搭建](#三环境搭建)
- [四、快速开始](#四快速开始)
- [五、工程结构](#五工程结构)
- [六、应用与示例](#六应用与示例)
- [七、编译指南](#七编译指南)
- [八、烧录与升级](#八烧录与升级)
- [九、配置说明](#九配置说明)
- [十、常见问题](#十常见问题)
- [十一、社区与支持](#十一社区与支持)
- [十二、免责声明](#十二免责声明)

---

## 一、概述

`fw-AD1x-4578_AC104_SDK` 是杰理科技为 AD14N / AD15N / AC104N / AD17N / AD18N 系列芯片提供的通用 MCU SDK 开发包。本系列芯片主要面向以下应用场景：

| 应用类型 | 典型产品 |
|---------|---------|
| **语音玩具** | 故事机、学习机、语音遥控玩具、MIDI 乐器 |
| **通用 MCU** | 智能控制、传感器采集、通用外设应用 |
| **小音箱** | 音乐播放器、FM 收音机、录音笔、扩音器 |

### 核心特性

- **音频解码**：支持 .a/.b/.e、.f1a/.f1b/.f1c 等多种音频格式解码播放
- **MIDI 播放**：支持 MIDI 合成与播放
- **音频编码**：支持 A/MP3/UMP3 编码
- **多路播放**：最多支持两路音频同时解码播放
- **变速变调**：支持音频变速变调播放（需系统时钟 100MHz 以上）
- **硬件重采样**：内置硬件重采样
- **低功耗**：关机功耗低至 1.7uA+
- **多种存储**：支持内置/外置 FLASH、FAT/NORFS/SYDF 文件系统
- **DAC 输出**：支持 PWM 差分输出及外接单端功放，支持 8K~32K 采样率

本仓库包含 SDK Release 版本代码及示例工程，需配合对应命名规则的库文件 (`lib.a`) 进行编译。

---

## 二、支持的芯片与平台

### 2.1 SoC 系列

| CPU 平台 | 芯片系列 | 应用领域 |
|---------|---------|---------|
| **sh54** | AD14N | 语音玩具 / 通用 MCU |
| **sh55** | AD15N | 语音玩具 / 通用 MCU |
| **sh57** | AD17N | 语音玩具 / 通用 MCU |
| **ch58** | AD18N | 语音玩具 / 通用 MCU（支持段码 LCD） |
| — | **AC104N** | 小音箱（mbox_mg） |

> 芯片型号/规格书/原理图资料请查阅：[doc/ 目录](doc/)

### 2.2 MCU 软硬件参数差异

![芯片](jl_ad_chip.png)

---

## 三、环境搭建

### 3.1 前提条件

| 系统 | 说明 |
|------|------|
| **Windows** | 推荐使用 Code::Blocks IDE 编译 |
| **Linux** | Makefile 命令行编译（需要重写download_bat.c脚本适配Linux环境） |
| **macOS** | 需自行配置交叉编译工具链 |

### 3.2 安装编译工具链

1. 下载并安装 **杰理编译工具链**：[下载链接](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html)
2. Linux 用户可从此处下载：[pkgman.jieliapp.com](http://pkgman.jieliapp.com/doc/all)
   - 下载后解压到 `/opt/jieli` 目录
   - 确保 `/opt/jieli/pi32/bin/clang` 存在
3. 安装完成后验证：

```bash
# 验证工具链是否安装成功
clang --version
```

### 3.3 安装烧录工具

| 工具 | 用途 | 获取方式 |
|------|------|---------|
| **USB 升级工具** | 将固件烧录到目标板 | [申请链接](https://item.taobao.com/item.htm?id=620295020803) · [使用文档](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html) |
| **生产烧写工具** | 量产/裸片烧写 | **代理商处** · [使用文档](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) |

---

### 3.4 音频工具

打包、音频文件转换、MIDI 等通用音频工具：[下载链接](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) 提取码：`3jey`

---

## 四、快速开始

### 4.1 克隆仓库

```bash
git clone https://github.com/Jieli-Tech/fw-AD15N.git
cd fw-AD15N/sdk
```

### 4.2 工程入口

SDK 包含以下应用工程，位于 `sdk/` 根目录：

| 工程文件 | 芯片 | 应用类型 |
|---------|------|---------|
| `AD14N_voice_toy.cbp` | AD14N (sh54) | 语音玩具 |
| `AD14N_mcu.cbp` | AD14N (sh54) | 通用 MCU |
| `AD15N_voice_toy.cbp` | AD15N (sh55) | 语音玩具 |
| `AD15N_mcu.cbp` | AD15N (sh55) | 通用 MCU |
| `AD17N_voice_toy.cbp` | AD17N (sh57) | 语音玩具 |
| `AD17N_mcu.cbp` | AD17N (sh57) | 通用 MCU |
| `AD18N_voice_toy.cbp` | AD18N (ch58) | 语音玩具 |
| `AD18N_mcu.cbp` | AD18N (ch58) | 通用 MCU |
| `AC104N_mbox_mg.cbp` | AC104N | 小音箱 |

### 4.3 应用代码入口

```
sdk/app/src/
├── voice_toy/          # 语音玩具应用（toy_music / toy_midi / toy_record / toy_linein / toy_speaker / toy_idle / toy_softoff / toy_usb_slave）
├── mbox_mg/            # 小音箱应用（music / fm / rec / line_in / loudspeaker / usb_device）
└── mcu/                # 通用 MCU 应用
```

### 4.4 编译并烧录

**方式一：Code::Blocks（推荐 Windows 用户）**

1. 双击打开对应的 `.cbp` 工程文件
2. 点击 **Build → Build**（Ctrl+F9）
3. 编译成功后，使用 USB 升级工具烧录生成的固件

**方式二：Makefile 命令行**

```bash
# Windows 用户
双击 sdk/tools/make_prompt.bat 打开命令行环境

# 选择对应的 Makefile 进行编译
make -f Makefile.ad15n_voice_toy all -j4
make -f Makefile.ad15n_mcu all -j4
```

> **💡 提示**：编译前请确保 USB 升级工具正确连接且目标板已进入编程模式。

> **💡 提示**：所有支持的 target 名称见 [Makefile](sdk/Makefile) 开头的注释。

**方式三：VS Code 编译**

仓库已预配置 VS Code 任务，按 `Ctrl+Shift+B` 即可选择编译目标。


---

## 五、工程结构

```
fw-AD15N/
├── sdk/                           # SDK 主目录
│   ├── app/                       # 应用层代码
│   │   ├── src/                   #   应用入口源码
│   │   │   ├── voice_toy/         #     语音玩具应用
│   │   │   ├── mbox_mg/           #     小音箱应用
│   │   │   └── mcu/               #     通用 MCU 应用
│   │   ├── bsp/                   #   板级支持包（BSP）
│   │   │   ├── common/            #     通用模块
│   │   │   │   ├── decoder/       #       解码器（A/F1A/MP3/MIDI/WAV）
│   │   │   │   ├── encoder/       #       编码器（A/MP3/UMP3）
│   │   │   │   ├── fs/            #       文件系统（FAT/NORFS/SYDF）
│   │   │   │   ├── key/           #       按键驱动（IO/AD/Matrix/IR/Touch）
│   │   │   │   ├── usb/           #       USB（device/host/usr）
│   │   │   │   ├── fm/            #       FM 收音机（BK1080/QN8035/RDA5807）
│   │   │   │   ├── midi_open/     #       MIDI 开源库
│   │   │   │   ├── sound_effect_list/  #  音效算法（ANS/Echo/EQ/Speed/Pitch）
│   │   │   │   ├── vm/            #       虚拟存储（new_vm/old_vm）
│   │   │   │   ├── msg/           #       消息机制
│   │   │   │   ├── norflash/      #       外挂 Flash 驱动
│   │   │   │   ├── speaker/       #       扩音
│   │   │   │   ├── power_manage/  #       电源管理
│   │   │   │   ├── rtc/           #       RTC
│   │   │   │   ├── iic_soft/      #       软件模拟 IIC
│   │   │   │   ├── spi_soft/      #       软件模拟 SPI
│   │   │   │   └── uart_update/   #       UART 升级
│   │   │   └── cpu/               #     CPU 平台驱动
│   │   │       ├── sh54/          #       AD14N（SPI/IIC/UART/ADC/DAC/PWM）
│   │   │       ├── sh55/          #       AD15N（SPI/IIC/UART/ADC/DAC/PWM）
│   │   │       ├── sh57/          #       AD17N（SPI/IIC/UART/ADC/DAC/PWM）
│   │   │       └── ch58/          #       AD18N（SPI/IIC/UART/ADC/DAC/PWM/LCD）
│   │   └── post_build/            #   编译后处理脚本与工具
│   ├── include_lib/               # 头文件
│   │   ├── driver/                #   驱动头文件
│   │   ├── system/                #   系统头文件
│   │   ├── cpu/                   #   CPU 平台头文件（sh54/sh55/sh57/ch58）
│   │   ├── decoder/               #   解码器 API 头文件
│   │   ├── encoder/               #   编码器 API 头文件
│   │   ├── fs/                    #   文件系统头文件
│   │   ├── audio/                 #   音频 API 头文件
│   │   ├── device/                #   设备驱动头文件
│   │   ├── dev_mg/                #   设备管理头文件
│   │   ├── common/                #   公共头文件
│   │   └── liba/                  #   预编译库 (.a)
│   │       ├── ARCH/pi32_lto/     #     通用算法库
│   │       ├── sh54/              #     AD14N 平台库
│   │       ├── sh55/              #     AD15N 平台库
│   │       ├── sh57/              #     AD17N 平台库
│   │       └── ch58/              #     AD18N 平台库
│   ├── tools/                     # 编译工具与脚本
│   │   ├── make_prompt.bat        #   Windows 编译命令行入口
│   │   └── utils/                 #   工具集（make、rm 等）
│   ├── Makefile                   # 顶层 Makefile
│   └── *.cbp                      # Code::Blocks 工程文件
├── doc/                           # 文档
│   ├── ad14n/                     #   AD14N 规格书/原理图
│   ├── ad15n/                     #   AD15N 规格书/原理图
│   ├── ad17n/                     #   AD17N 规格书/原理图
│   ├── ad18n/                     #   AD18N 规格书/原理图
│   ├── ac104n/                    #   AC104N 规格书/原理图
│   ├── stuff/                     #   杂项（钉钉群、USB升级工具文档等）
│   └── *.pdf                      #   SDK 手册 / 版本信息 / 选型表
└── README.md                      # 本文件
```

### 5.1 关键目录说明

| 目录 | 作用 |
|------|------|
| `sdk/app/src/` | **应用入口**：各应用的主函数、消息处理、场景切换 |
| `sdk/app/bsp/common/` | **通用模块**：解码器、编码器、按键、文件系统、USB、FM、MIDI 等核心模块 |
| `sdk/app/bsp/cpu/` | **CPU 驱动**：各平台的外设驱动（GPIO、UART、SPI、IIC、ADC、DAC、PWM 等） |
| `sdk/include_lib/liba/` | **预编译库**：各平台的 `.a` 静态库文件 |
| `sdk/app/post_build/` | **下载工具**：`isd_download.exe`、`isd_config.ini`、下载脚本 |
| `doc/` | **文档**：芯片手册、原理图、SDK 手册、选型表 |

---

## 六、应用与示例

### 6.1 语音玩具应用 (`app/src/voice_toy/`)

| 子模块 | 说明 |
|-------|------|
| **toy_music** | 音乐播放（本地/外置 FLASH 文件播放） |
| **toy_midi** | MIDI 乐器演奏与播放 |
| **toy_record** | 录音功能 |
| **toy_linein** | 线路输入（AUX） |
| **toy_speaker** | 扩音/喊话 |
| **toy_idle** | 待机/空闲处理 |
| **toy_softoff** | 软关机 |
| **toy_usb_slave** | USB 从设备 |

适用领域：语音玩具、故事机、学习机、MIDI 乐器等。

### 6.2 通用 MCU 应用 (`app/src/mcu/`)

| 项目 | 说明 |
|------|------|
| **适用场景** | 智能控制、传感器采集、通用外设应用 |
| **关键外设** | UART、IIC、SPI、ADC、DAC、PWM、RTC、GPIO、IR、触摸按键 |

### 6.3 小音箱应用 (`app/src/mbox_mg/`)

| 子模块 | 说明 |
|-------|------|
| **music** | 音乐播放（FAT/SD/USB 文件系统） |
| **fm** | FM 收音机（支持 BK1080/QN8035/RDA5807等） |
| **rec** | 录音模式 |
| **line_in** | 线路输入（AUX） |
| **loudspeaker** | 扩音/喊话 |
| **usb_device** | USB 从设备（UAC/CDC/MSD/HID） |

适用领域：便携音箱、音乐播放器、FM 收音机等。

---

## 七、编译指南

### 7.1 编译命令速查表

以下命令在 `sdk/` 目录下执行：

| 目标 | 芯片 | 命令 |
|------|------|------|
| **语音玩具** | AD14N | `make -f Makefile.ad14n_voice_toy all -j4` |
| **语音玩具** | AD15N | `make -f Makefile.ad15n_voice_toy all -j4` |
| **语音玩具** | AD17N | `make -f Makefile.ad17n_voice_toy all -j4` |
| **语音玩具** | AD18N | `make -f Makefile.ad18n_voice_toy all -j4` |
| **通用 MCU** | AD14N | `make -f Makefile.ad14n_mcu all -j4` |
| **通用 MCU** | AD15N | `make -f Makefile.ad15n_mcu all -j4` |
| **通用 MCU** | AD17N | `make -f Makefile.ad17n_mcu all -j4` |
| **通用 MCU** | AD18N | `make -f Makefile.ad18n_mcu all -j4` |
| **小音箱** | AC104N | `make -f Makefile.ac104n_mbox_mg all -j4` |
| **编译全部** | 全部 | `make all` |
| **清理全部** | 全部 | `make clean` |

### 7.2 Code::Blocks 编译（推荐 Windows 用户）

1. 确保已安装杰理编译工具链
2. 双击对应的 `.cbp` 工程文件打开 Code::Blocks
3. 点击 **Build → Build**（Ctrl+F9）
4. 编译成功后在 `post_build/` 目录下生成固件

### 7.3 Makefile 编译

```bash
# Windows 用户
双击 sdk/tools/make_prompt.bat 打开命令行环境
make -f Makefile.ad15n_voice_toy all -j4

# Linux 用户 (需要自行修改download_bat.c脚本适配)
cd sdk
make -f Makefile.ad15n_voice_toy all -j`nproc`
```

### 7.4 常见编译错误

| 错误提示 | 解决方法 |
|---------|---------|
| `clang: command not found` | 未安装杰理编译工具链，或环境变量未配置 |
| `cannot find -lxxx` | 缺少对应的 `.a` 库文件，检查 `include_lib/liba/` 目录 |
| `make: command not found` | Windows 下使用 `tools/make_prompt.bat` 打开编译命令环境 |
| 链接错误 | 检查是否选择了正确芯片的 Makefile/CBP 工程 |

---

## 八、烧录与升级

### 8.1 首次烧录

1. **连接硬件**：将开发板通过 **USB** 或者 **USB升级工具** 连接到 PC
2. **进入编程模式**：
    - 方式一（USB）：按住开发板上的烧录按键，然后复位或重新上电
    - 方式二（USB/UART）：通过 USB 升级工具进入编程模式
3. **打开 USB 升级工具**：启动烧录上位机
4. **选择固件**：选择编译生成的固件文件
5. **开始烧录**：点击下载按钮，等待烧录完成

> **注意**：烧录前请确保 USB 升级工具正确连接且目标板已进入编程模式。关于 ISD_CONFIG.INI 配置详见 [ISD 配置说明](https://doc.zh-jieli.com/AD14/zh-cn/master/download_and_tools_introduction/download_introduction.html#)。

### 8.2 生产烧写

量产场景请使用杰理生产烧写工具（一拖二 / 一拖八），支持裸片烧写。详见 [一拖二烧写器使用说明](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) · [一拖八烧写器使用说明](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo8/index.html)

### 8.3 OTA 升级

支持双备份固件升级（dual_bank），详见 `sdk/app/bsp/common/dual_bank_demo.c`。

---

## 九、配置说明

- 编辑 `sdk/app/src/<应用>/app_config.h` 可配置目标应用的功能开关
- 不同 CPU 平台的配置位于 `sdk/app/src/<应用>/<平台>/app_modules.h`

---

## 十、常见问题

### 10.1 开发流程相关

**Q: 如何创建一个新的工程？**
A: 基于现有的 `.cbp` 工程和 `app/src/` 中的应用代码进行修改，配置对应用例即可。

**Q: 如何切换不同的芯片平台？**
A: 选择对应的 `.cbp` 工程或 Makefile，SDK 已为每个芯片平台预配置了独立的编译入口。

### 10.2 编译相关

**Q: Windows 下编译报错 `make` 不是有效命令？**
A: 使用 `sdk/tools/make_prompt.bat` 进入预配置的命令行环境，该脚本已设置好所有环境变量和 `make` 的路径。

**Q: 如何加快编译速度？**
A: 使用 `-j` 参数进行并行编译，如 `make -j4`（数字为并行任务数）。

### 10.3 调试技巧

- **串口日志**：可通过 UART 输出调试日志
- **GPIO Debug**：利用空闲 GPIO 输出调试波形，测量时序

---

## 十一、社区与支持

### 技术交流

| 平台 | 群号/链接 | 状态 |
|------|-----------|------|
| **钉钉技术交流群** | 见 [群二维码](doc/stuff/dingtalk.jpg) | ✅ 可加入 |

### 资源链接

| 资源 | 链接 |
|------|------|
| 📖 **在线文档中心** | [doc.zh-jieli.com/AD14](https://doc.zh-jieli.com/AD14/zh-cn/master/index.html) |
| 📚 **SDK 版本历史** | [SDK 发布版本信息](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf) |
| 🔧 **SDK 快速入门** | [SDK 手册](doc/AD14N_AD15N_AD17N_AD18N_AC104N_SDK手册.pdf) |
| 📄 **芯片选型** | [选型表](doc/杰理科技AD14_AD15_AD16_AD17_AD18系列语音MCU选型表.pdf) |
| 🎬 **视频教程** | [Bilibili 主页](https://space.bilibili.com/3493277347088769/dynamic) |
| 🎵 **MIDI 开发手册** | [MIDI 应用开发文档](https://doc.zh-jieli.com/MIDI/zh-cn/master/index.html) |
| 📦 **FAE 支持** | [FAE 支持仓库](https://gitee.com/jieli-tech_fae/fw-jl) |
| 🛒 **开发板/烧录工具购买** | [杰理官方店铺](https://shop321455197.taobao.com/) |
| 🐛 **问题反馈** | [Github Issues](https://github.com/Jieli-Tech/fw-AD15N/issues) |

---

## 十二、免责声明

`fw-AD1x-4578_AC104_SDK` 支持 AD14N / AD15N / AC104N / AD17N / AD18N 系列芯片开发。本系列芯片支持通用 MCU 常见应用，可作为开发、评估、样品及量产使用，对应 SDK 版本请见 [SDK 版本历史](doc/AD1x-4578_AC104_SDK_发布版本信息.pdf)。

---

<div align="center">
  <sub>Copyright &copy; 珠海杰理科技股份有限公司. All rights reserved.</sub>
</div>
