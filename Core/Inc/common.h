#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define _CCM_DATA __attribute__((section(".CCM_RAM1.data")))
#define _RAM_DATA __attribute__((section(".RAM1.data")))
#define ARRAY_SIZE(array_name) ((int)(sizeof(array_name) / sizeof(array_name[0])))

#ifdef __cplusplus
extern "C" {
#endif

void reset_global_irq(void);
void disable_global_irq(void);
void enable_global_irq(void);

#ifdef __cplusplus
}
#endif

#endif
