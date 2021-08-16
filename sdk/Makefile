# 总的 Makefile，用于调用目录下各个子工程对应的 Makefile
# 支持的目标
# make ad15n_app_mg
# make ad14n_simple
# make ad14n_mcu
# make ad14n_app_mg
# make ad15n_mcu
# make ad15n_simple

.PHONY: all clean ad15n_app_mg ad14n_simple ad14n_mcu ad14n_app_mg ad15n_mcu ad15n_simple clean_ad15n_app_mg clean_ad14n_simple clean_ad14n_mcu clean_ad14n_app_mg clean_ad15n_mcu clean_ad15n_simple

all: ad15n_app_mg ad14n_simple ad14n_mcu ad14n_app_mg ad15n_mcu ad15n_simple
	@echo +ALL DONE

clean: clean_ad15n_app_mg clean_ad14n_simple clean_ad14n_mcu clean_ad14n_app_mg clean_ad15n_mcu clean_ad15n_simple
	@echo +CLEAN DONE

ad15n_app_mg:
	$(MAKE) -C . -f Makefile.ad15n_app_mg

clean_ad15n_app_mg:
	$(MAKE) -C . -f Makefile.ad15n_app_mg clean

ad14n_simple:
	$(MAKE) -C . -f Makefile.ad14n_simple

clean_ad14n_simple:
	$(MAKE) -C . -f Makefile.ad14n_simple clean

ad14n_mcu:
	$(MAKE) -C . -f Makefile.ad14n_mcu

clean_ad14n_mcu:
	$(MAKE) -C . -f Makefile.ad14n_mcu clean

ad14n_app_mg:
	$(MAKE) -C . -f Makefile.ad14n_app_mg

clean_ad14n_app_mg:
	$(MAKE) -C . -f Makefile.ad14n_app_mg clean

ad15n_mcu:
	$(MAKE) -C . -f Makefile.ad15n_mcu

clean_ad15n_mcu:
	$(MAKE) -C . -f Makefile.ad15n_mcu clean

ad15n_simple:
	$(MAKE) -C . -f Makefile.ad15n_simple

clean_ad15n_simple:
	$(MAKE) -C . -f Makefile.ad15n_simple clean
