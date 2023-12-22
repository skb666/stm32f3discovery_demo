#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>

/* VERSION: 0.0.1.0 */
#define MCU_SOFTWARE_VERSION (0x0010)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  STATUS_SHELL = 0,
  STATUS_USB_AS_USART1,
  STATUS_USB_AS_USART3,
  STATUS_NUM,
} SYS_STATUS;

typedef struct {
  uint16_t value;
} LED_CTRL;

typedef struct {
  LED_CTRL leds;
} SYS_CTRL;

typedef struct {
  uint8_t usart1_rx : 1;
  uint8_t usart1_tx : 1;
  uint8_t usart3_rx : 1;
  uint8_t usart3_tx : 1;
  uint8_t : 4;
} USB_TX_FLAG;

typedef struct {
  USB_TX_FLAG usb_tx;
} SYS_FLAG;

typedef struct {
  SYS_STATUS status;
  SYS_CTRL ctrl;
  SYS_FLAG flag;
} SYS_PARAM;

SYS_PARAM *sys_param_get(void);
void sys_param_init(void);

#ifdef __cplusplus
}
#endif

#endif
