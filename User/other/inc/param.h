#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint16_t value;
} LED_CTRL;

typedef struct {
  LED_CTRL leds;
} SYS_CTRL;

typedef struct {
  uint8_t usart3_rx : 1;
  uint8_t usart3_tx : 1;
  uint8_t : 6;
} USB_TX_FLAG;

typedef struct {
  USB_TX_FLAG usb_tx;
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
