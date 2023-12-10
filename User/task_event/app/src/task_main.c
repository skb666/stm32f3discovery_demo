#include <stdio.h>

#include "device.h"
#include "key.h"
#include "main.h"
#include "mycmd.h"
#include "param.h"
#include "task.h"
#include "timer.h"
#include "usbd_cdc_if.h"
#include "xcmd.h"

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

static void change_sys_status(void) {
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      sys->status = STATUS_USB_AS_USART3;
      usb_printf("SYS_STATUS: USB_AS_USART3\r\n");
    } break;
    case STATUS_USB_AS_USART3: {
      sys->status = STATUS_SHELL;
      usb_printf("SYS_STATUS: SHELL\r\n");
      LL_USART_SetBaudRate(USART3, LL_RCC_GetUSARTClockFreq(LL_RCC_USART3_CLKSOURCE), LL_USART_OVERSAMPLING_16, 921600);
    } break;
    default: {
    } break;
  }
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
      change_sys_status();
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

/* 1ms 周期任务 */
void timer_1ms_init(void) {
  task_event_subscribe(EVENT_TYPE_TICK_1MS, TASK_ID_TIMER_1MS);
}

static void usart3_tx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      if (sys->flag.usb_tx.usart3_tx) {
        usb_puts(buf, len);
      }
    } break;
    default: {
    } break;
  }
}

static void timer_1ms_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_TICK_1MS: {
      // 串口发送
      uart_tx_poll(usart3_tx_to_usb);
      // usb cdc 发送
      usb_tx_trans();
    } break;
    default: {
    } break;
  }
}

void timer_1ms_handle(TASK *task) {
  task_event_process(task, timer_1ms_cb);
}

/* 循环任务 */
static int cmd_get_char(uint8_t *ch) {
  return !usb_rx_get(ch);
}

static int cmd_put_char(uint8_t ch) {
  usb_putchar(ch);
  return 1;
}

static void print_frame(frame_parse_t *frame) {
  uart_write(frame->data, frame->length);
}

void main_loop_init(void) {
  xcmd_init(cmd_get_char, cmd_put_char);
  mycmd_init();

  frame_parse_register(FRAME_TYPE_DEBUG, print_frame);
}

void main_loop_handle(TASK *task) {
  SYS_PARAM *sys = sys_param_get();
  uint8_t ch;

  switch (sys->status) {
    case STATUS_SHELL: {
      // USB CDC Shell
      xcmd_task();
      // USART3 帧解析
      uart_frame_parse();
    } break;
    case STATUS_USB_AS_USART3: {
      // 清理 usart3_rx 缓冲区
      if (!uart_read(&ch, 1)) {
      }
    } break;
    default: {
    } break;
  }

  task_update_times(task);
}
