#ifndef _KEY_MATRIX_H
#define _KEY_MATRIX_H

#include "key.h"
#include "gpio.h"
///X轴 io 要求是AD口，详细AD口看adc_drv.h
#define X_ADC_CH_INIT 		X_ADC_CH_SEL
///Y轴 io 要求是普通IO口
#define Y_PORT_INIT 		Y_PORT_SEL

#define	MATRIX_KEY_THRESHOLD		300

extern const key_interface_t key_matrix_info;
#endif
