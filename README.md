# fw-AD14N-AD15N-AC104N
About Firmware for Generic MCU SDK（AD14N-AD15N-AC104N series）, Support AD14N,AD15N,AC104N

[tag download]:https://github.com/Jieli-Tech/fw-AD15N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/fw-AD15N?style=plastic&labelColor=ffffff&color=informational&label=Tag&

# fw-AD14N-AD15N-AC104N_SDK   [![tag][tag_badgen]][tag download]

中文 | [EN](./README-en.md)

AD14N-AD15N 系列通用MCU SDK 固件程序

本仓库包含SDK release 版本代码，线下线上支持同步发布，支持玩具类产品和通用MCU类应用二次开发.

本工程提供的例子，需要结合对应命名规则的库文件(lib.a) 和对应的子仓库进行编译.

快速开始
------------

欢迎使用杰理开源项目，在开始进入项目之前，请详细阅读SDK 介绍文档，
从而获得对杰理系列芯片和SDK 的大概认识，并且可以通过快速开始介绍来进行开发.

工具链
------------

关于如何获取`杰理工具链` 和 如何进行环境搭建，请阅读以下内容：

* 编译工具 ：请安装杰理编译工具来搭建起编译环境, [下载链接](https://pan.baidu.com/s/1f5pK7ZaBNnvbflD-7R22zA) 提取码: `ukgx`
* 编译器邀请码：4VlZaQCq-lImlZX2u-GBeCs501-ektNxDGu

* USB 升级工具 : 在开发完成后，需要使用杰理烧写工具将对应的`hex`文件烧录到目标板，进行开发调试, 关于如何获取工具请进入申请 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) 并详细阅读对应的[文档](doc/stuff/usb%20updater.pdf)，以及相关下载脚本[配置](doc/stuff/ISD_CONFIG.INI配置文件说明.pdf)

介绍文档
------------

* 芯片简介 : [SoC 数据手册扼要](./doc)

* SDK 版本信息 : [SDK 历史版本](doc/AD14N-AD15N-AC104N_SDK_发布版本信息.pdf)

* SDK 介绍文档 : [SDK 快速开始简介](./doc/AD14N_AD15N_AC104N_SDK手册.pdf)

* SDK 结构文档 : [SDK 模块结构](./doc/)

编译工程
-------------
请选择以下一个工程进行编译，下列目录包含了便于开发的工程文件：

* 玩具类应用 : ./sdk/AD14N_app_mg.cbp ./sdk/AD15N_app_mg.cbp , 适用领域：

* 示例类应用 : ./sdk/AD14N_simple.cbp ./sdk/AD15N_simple.cbp ，适用领域：

* 通用MCU应用 : ./sdk/AD14N_mcu.cbp ./sdk/AD15N_mcu.cbp ，适用领域：

* 小音箱应用: ./sdk/AC104N_mbox_mg ，适用领域： 

即将发布：
------------

SDK 支持Codeblock编译环境，请确保编译前已经搭建好编译环境，

* Codeblock 编译 : 进入对应的工程目录并找到后缀为 `.cbp` 的文件, 双击打开便可进行编译.

* Makefile 编译 : `apps/app_cfg` 开始编译之前，需要先选择好目标应用并编辑保存, 请双击 `make_prompt` 并输入 `make`

  `在编译下载代码前，请确保USB 升级工具正确连接并且进入编程模式`
  

硬件环境
-------------

* 开发评估板 ：开发板申请入口[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

* 生产烧写工具 : 为量产和裸片烧写而设计, 申请入口 [连接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.8.504d246bXKwyeH&id=620941819219) 并仔细阅读相关 [文档](./doc/stuff/烧写器使用说明文档.pdf)
  
打包工具&音频文件转换工具
-------------

* [下载链接](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) 提取码：`3jey` 

SDK主要功能
-------------
* 支持内置FLASH的解码播放
* 支持外置FLASH的解码播放
* 支持解码MIO功能
* 支持.a/.b/.e、.f1a/.f1b/.f1c这两种种格式的解码播放
* 支持MIDI播放
* 支持变速变调（不过需要把系统时钟调整到100Mhz以上）
* 最多支持.a/.b/.e + .f1a/.f1b/.f1c 两路音频同时解码播放
* DAC支持PWM差分输出，以及外接单端功放
* DAC支持8K、12k、16K、24K、32K采样率
* 支持硬件的重采样
* 关机功耗1.7uA+

MCU信息
-------------
* RISC / 160MHz /20K+8K
* flash / 2Mbit
* UART * 2 / IIC / SPI * 2
* 10bit ADC
* 16bit dac
* SD IO
* IR
* PWM * 4
* 16bit timer * 3

社区
--------------

* 技术交流群[钉钉](./doc/stuff/dingtalk.jpg)


免责声明
------------

AD15N_SDK 支持AD15 系列芯片开发.
AD15 系列芯片支持了通用MCU 常见应用，可以作为开发，评估，样品，甚至量产使用，对应SDK 版本见Release
