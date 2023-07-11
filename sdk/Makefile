# 总的 Makefile，用于调用目录下各个子工程对应的 Makefile
# 注意： Linux 下编译方式：
# 1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
# 2. 下载后，解压到 /opt/jieli 目录下，保证
#   /opt/jieli/common/bin/clang 存在（注意目录层次）
# 3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#   可以通过 ulimit -n 8096 来设置一个较大的值
# 支持的目标
# make ad15n_voice_toy
# make ad17n_voice_toy
# make ad14n_mcu
# make ad14n_voice_toy
# make ac104n_mbox_mg
# make ad15n_mcu
# make ad17n_mcu

.PHONY: all clean ad15n_voice_toy ad17n_voice_toy ad14n_mcu ad14n_voice_toy ac104n_mbox_mg ad15n_mcu ad17n_mcu clean_ad15n_voice_toy clean_ad17n_voice_toy clean_ad14n_mcu clean_ad14n_voice_toy clean_ac104n_mbox_mg clean_ad15n_mcu clean_ad17n_mcu

all: ad15n_voice_toy ad17n_voice_toy ad14n_mcu ad14n_voice_toy ac104n_mbox_mg ad15n_mcu ad17n_mcu
	@echo +ALL DONE

clean: clean_ad15n_voice_toy clean_ad17n_voice_toy clean_ad14n_mcu clean_ad14n_voice_toy clean_ac104n_mbox_mg clean_ad15n_mcu clean_ad17n_mcu
	@echo +CLEAN DONE

ad15n_voice_toy:
	$(MAKE) -C . -f Makefile.ad15n_voice_toy

clean_ad15n_voice_toy:
	$(MAKE) -C . -f Makefile.ad15n_voice_toy clean

ad17n_voice_toy:
	$(MAKE) -C . -f Makefile.ad17n_voice_toy

clean_ad17n_voice_toy:
	$(MAKE) -C . -f Makefile.ad17n_voice_toy clean

ad14n_mcu:
	$(MAKE) -C . -f Makefile.ad14n_mcu

clean_ad14n_mcu:
	$(MAKE) -C . -f Makefile.ad14n_mcu clean

ad14n_voice_toy:
	$(MAKE) -C . -f Makefile.ad14n_voice_toy

clean_ad14n_voice_toy:
	$(MAKE) -C . -f Makefile.ad14n_voice_toy clean

ac104n_mbox_mg:
	$(MAKE) -C . -f Makefile.ac104n_mbox_mg

clean_ac104n_mbox_mg:
	$(MAKE) -C . -f Makefile.ac104n_mbox_mg clean

ad15n_mcu:
	$(MAKE) -C . -f Makefile.ad15n_mcu

clean_ad15n_mcu:
	$(MAKE) -C . -f Makefile.ad15n_mcu clean

ad17n_mcu:
	$(MAKE) -C . -f Makefile.ad17n_mcu

clean_ad17n_mcu:
	$(MAKE) -C . -f Makefile.ad17n_mcu clean
