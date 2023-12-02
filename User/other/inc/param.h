#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t LD3 : 1;
  uint8_t LD4 : 1;
  uint8_t LD5 : 1;
  uint8_t LD6 : 1;
  uint8_t LD7 : 1;
  uint8_t LD8 : 1;
  uint8_t LD9 : 1;
  uint8_t LD10 : 1;
} LED_CTRL;

typedef struct {
  uint8_t leds;
} SYS_CTRL;

typedef struct {
  uint8_t serial_to_usb;
} SYS_FLAG;

typedef struct {
  SYS_CTRL ctrl;
  SYS_FLAG flag;
} SYS_PARAM;

SYS_PARAM *sys_param_get(void);
void sys_param_init(void);

#ifdef __cplusplus
}
#endif

#endif
