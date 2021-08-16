#ifndef __CPU_H__
#define __CPU_H__

//DSM 模式
#define DAC_DSM2     (0<<5)
#define DAC_DSM3     (1<<5)
#define DAC_DSM7     (2<<5)
#define DAC_DSM11    (3<<5)


#define DAC_MODE_BITS   ((3<<5) | BIT(4) | BIT(7))

#endif
