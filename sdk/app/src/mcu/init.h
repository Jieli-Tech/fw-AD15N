#ifndef __INIT_H__
#define __INIT_H__

void emu_init();
void exception_analyze(unsigned int *sp);
void emu_test();

void system_init(void);
void all_init_isr(void);
#endif
