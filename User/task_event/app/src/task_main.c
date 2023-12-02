#include <stdio.h>

#include "device.h"
#include "key.h"
#include "main.h"
#include "param.h"
#include "task.h"
#include "timer.h"
#include "usbd_cdc_if.h"

#define SERIAL_TO_USB_TX 0x01
#define SERIAL_TO_USB_RX 0x10

typedef enum {
  FRAME_TYPE_DEBUG = 0,
  FRAME_TYPE_MAX,
} FRAME_TYPE;

static void task_event_process(TASK *task, void (*callback)(EVENT *)) {
  int8_t err;
  EVENT *ev;

  if (event_empty(&task->events)) {
    return;
  }

  while (event_count(&task->events)) {
    err = event_peek(&task->events, &ev);
    if (err) {
      return;
    }

    callback(ev);

    event_pop_only(&task->events);
    task_update_times(task);
  }
}

static void change_flag_serial_to_usb(void) {
  SYS_PARAM *sys = sys_param_get();

  sys->flag.serial_to_usb = (sys->flag.serial_to_usb + 1) % 4;
}

/* 调试信息打印 */
void debug_print_init(void) {
  task_event_subscribe(EVENT_TYPE_KEY_PRESS, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_RELEASE, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_LONG_PRESS, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_LONG_RELEASE, TASK_ID_DEBUG_PRINT);
}

static void debug_print_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_KEY_PRESS: {
      printf("[KEY]: PRESS\n");
      change_flag_serial_to_usb();
    } break;
    case EVENT_TYPE_KEY_RELEASE: {
      printf("[KEY]: RELEASE\n");
    } break;
    case EVENT_TYPE_KEY_LONG_PRESS: {
      printf("[KEY]: LONG_PRESS\n");
    } break;
    case EVENT_TYPE_KEY_LONG_RELEASE: {
      printf("[KEY]: LONG_RELEASE\n");
    } break;
    default: {
    } break;
  }
}

void debug_print_handle(TASK *task) {
  task_event_process(task, debug_print_cb);
}

/* 按键扫描 */
static KEY_VALUE getKey(void) {
  if (LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin) == 1) {
    return K_PRESS;
  } else {
    return K_RELEASE;
  }
}

void key_scan_init(void) {
  task_event_subscribe(EVENT_TYPE_TICK_25MS, TASK_ID_KEY_SCAN);
}

static void key_scan_cb(EVENT *ev) {
  static KEY key = {
      .status = KS_RELEASE,
      .count = 0,
      .get = getKey,
  };
  static KEY_EVENT k_ev;

  switch (ev->type) {
    case EVENT_TYPE_TICK_25MS: {
      k_ev = key_status_check(&key, 20);
      switch (k_ev) {
        case KE_PRESS: {
          task_event_publish(EVENT_TYPE_KEY_PRESS, NULL, 0);
        } break;
        case KE_RELEASE: {
          task_event_publish(EVENT_TYPE_KEY_RELEASE, NULL, 0);
        } break;
        case KE_LONG_PRESS: {
          task_event_publish(EVENT_TYPE_KEY_LONG_PRESS, NULL, 0);
        } break;
        case KE_LONG_RELEASE: {
          task_event_publish(EVENT_TYPE_KEY_LONG_RELEASE, NULL, 0);
        } break;
        default: {
        } break;
      }
    } break;
    default: {
    } break;
  }
}

void key_scan_handle(TASK *task) {
  task_event_process(task, key_scan_cb);
}

/* USART1 TX */
void usart1_tx_init(void) {
  task_event_subscribe(EVENT_TYPE_TICK_1MS, TASK_ID_USART1_TX);
}

static void usart1_tx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  if (sys->flag.serial_to_usb & SERIAL_TO_USB_TX) {
    usb_puts(buf, len);
  }
}

static void usart1_tx_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_TICK_1MS: {
      uart_tx_poll(usart1_tx_to_usb);
    } break;
    default: {
    } break;
  }
}

void usart1_tx_handle(TASK *task) {
  task_event_process(task, usart1_tx_cb);
}

/* USART1 RX */
static void print_frame(frame_parse_t *frame) {
  uart_write(frame->data, frame->length);
}

static void usart1_rx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  if (sys->flag.serial_to_usb & SERIAL_TO_USB_RX) {
    usb_puts(buf, len);
  }
}

void usart1_rx_init(void) {
  frame_parse_register(FRAME_TYPE_DEBUG, print_frame);
}

void usart1_rx_handle(TASK *task) {
  // USART1 帧解析
  uart_frame_parse(usart1_rx_to_usb);

  task_update_times(task);
}
