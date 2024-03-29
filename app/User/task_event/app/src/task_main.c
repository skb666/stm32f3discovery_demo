#include <stdio.h>

#include "combo_key.h"
#include "common.h"
#include "i2c_slave.h"
#include "main.h"
#include "mycmd.h"
#include "param.h"
#include "task.h"
#include "timer.h"
#include "uart_device.h"
#include "update.h"
#include "usbd_cdc_if.h"
#include "xcmd.h"

typedef enum {
  FRAME_TYPE_DEBUG = 0,
  FRAME_TYPE_I2C_WRITE = 0xE1,
  FRAME_TYPE_I2C_READ = 0xE2,
  FRAME_TYPE_SYSTEM_CTRL = 0xF0,
  FRAME_TYPE_UPDATE_DATA = 0xF1,
  FRAME_TYPE_UPDATE_STATUS = 0xF2,
  FRAME_TYPE_MAX = FUNC_LIST_MAX,
} FRAME_TYPE;

typedef enum {
  FROM_RX,
  FROM_TX,
} DATA_FROM;

extern void led_blink_control(void);

extern void system_ctrl_frame_parse(frame_parse_t *frame);
extern void update_status_get(frame_parse_t *frame);
extern void update_frame_parse(frame_parse_t *frame);
extern void update_pkg_process(void);
extern void uart_frame_i2c_write(frame_parse_t *frame);
extern void uart_frame_i2c_read(frame_parse_t *frame);

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
      sys->status = STATUS_USB_AS_USART1;
      usb_printf("\r\nSYS_STATUS: USB_AS_USART1\r\n");
    } break;
    case STATUS_USB_AS_USART1: {
      sys->status = STATUS_USB_AS_USART3;
      usb_printf("\r\nSYS_STATUS: USB_AS_USART3\r\n");
      LL_USART_SetBaudRate(USART1, LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE), LL_USART_OVERSAMPLING_16, 115200);
      LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
      LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
      LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
    } break;
    case STATUS_USB_AS_USART3: {
      sys->status = STATUS_SHELL;
      usb_printf("\r\nSYS_STATUS: SHELL\r\n");
      LL_USART_SetBaudRate(USART3, LL_RCC_GetUSARTClockFreq(LL_RCC_USART3_CLKSOURCE), LL_USART_OVERSAMPLING_16, 115200);
      LL_USART_SetStopBitsLength(USART3, LL_USART_STOPBITS_1);
      LL_USART_SetParity(USART3, LL_USART_PARITY_NONE);
      LL_USART_SetDataWidth(USART3, LL_USART_DATAWIDTH_8B);
    } break;
    default: {
    } break;
  }
}

static void usart1_tx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      if (sys->flag.usb_tx.usart1_tx) {
        usb_puts(buf, len);
      }
    } break;
    default: {
    } break;
  }
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

static void usart1_rx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      if (sys->flag.usb_tx.usart1_rx) {
        usb_puts(buf, len);
      }
    } break;
    case STATUS_USB_AS_USART1: {
      usb_puts(buf, len);
    } break;
    default: {
    } break;
  }
}

static void usart3_rx_to_usb(uint8_t *buf, uint16_t len) {
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      if (sys->flag.usb_tx.usart3_rx) {
        usb_puts(buf, len);
      }
    } break;
    case STATUS_USB_AS_USART3: {
      usb_puts(buf, len);
    } break;
    default: {
    } break;
  }
}

static void (*usb_monitor(DEV_TYPE dev_type, DATA_FROM data_from))(uint8_t *, uint16_t) {
  switch (dev_type) {
    case DEV_USART1: {
      if (data_from == FROM_TX) {
        return usart1_tx_to_usb;
      } else if (data_from == FROM_RX) {
        return usart1_rx_to_usb;
      } else {
        return NULL;
      }
    } break;
    case DEV_USART3: {
      if (data_from == FROM_TX) {
        return usart3_tx_to_usb;
      } else if (data_from == FROM_RX) {
        return usart3_rx_to_usb;
      } else {
        return NULL;
      }
    } break;
    default: {
      return NULL;
    } break;
  }
}

/* 调试信息打印 */
void debug_print_init(void) {
  // task_event_subscribe(EVENT_TYPE_KEY_PRESS, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_RELEASE, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_LONG_PRESS, TASK_ID_DEBUG_PRINT);
  // task_event_subscribe(EVENT_TYPE_KEY_LONG_RELEASE, TASK_ID_DEBUG_PRINT);
  // task_event_subscribe(EVENT_TYPE_KEY_COMBO, TASK_ID_DEBUG_PRINT);
  task_event_subscribe(EVENT_TYPE_KEY_COMBO_RELEASE, TASK_ID_DEBUG_PRINT);
}

static void debug_print_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_KEY_PRESS: {
      usb_printf("[KEY]: PRESS\r\n");
    } break;
    case EVENT_TYPE_KEY_RELEASE: {
      usb_printf("[KEY]: RELEASE\r\n");
      change_sys_status();
    } break;
    case EVENT_TYPE_KEY_LONG_PRESS: {
      usb_printf("[KEY]: LONG_PRESS %hu\r\n", (size_t)ev->custom_data);
    } break;
    case EVENT_TYPE_KEY_LONG_RELEASE: {
      usb_printf("[KEY]: LONG_RELEASE %hu\r\n", (size_t)ev->custom_data);
    } break;
    case EVENT_TYPE_KEY_COMBO: {
      usb_printf("[KEY]: COMBO %hu\r\n", (size_t)ev->custom_data);
    } break;
    case EVENT_TYPE_KEY_COMBO_RELEASE: {
      usb_printf("[KEY]: COMBO_RELEASE %hu\r\n", (size_t)ev->custom_data);
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
  task_event_subscribe(EVENT_TYPE_TICK_5MS, TASK_ID_KEY_SCAN);
  key_register(0, getKey, NULL, 4, 75, 200);
}

static void key_scan_cb(EVENT *ev) {
  KEY *key_list;
  int num;

  switch (ev->type) {
    case EVENT_TYPE_TICK_5MS: {
      key_list = key_list_get(&num);
      for (int i = 0; i < num; ++i) {
        switch (combo_key_event_check(&key_list[i])) {
          case KE_PRESS: {
            task_event_publish(EVENT_TYPE_KEY_PRESS, NULL, 0);
          } break;
          case KE_RELEASE: {
            task_event_publish(EVENT_TYPE_KEY_RELEASE, NULL, 0);
          } break;
          case KE_LONG_PRESS: {
            task_event_publish(EVENT_TYPE_KEY_LONG_PRESS, (void *)(size_t)key_combo_release_count(&key_list[i]), 0);
          } break;
          case KE_LONG_RELEASE: {
            task_event_publish(EVENT_TYPE_KEY_LONG_RELEASE, (void *)(size_t)key_combo_release_count(&key_list[i]), 0);
          } break;
          case KE_COMBO: {
            task_event_publish(EVENT_TYPE_KEY_COMBO, (void *)(size_t)key_combo_press_count(&key_list[i]), 0);
          } break;
          case KE_COMBO_RELEASE: {
            task_event_publish(EVENT_TYPE_KEY_COMBO_RELEASE, (void *)(size_t)key_combo_press_count(&key_list[i]), 0);
          } break;
          default: {
          } break;
        }
      }
      task_delay_ms(TASK_ID_KEY_SCAN, 5);
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

  uart_set_tx_monitor(DEV_USART1, usb_monitor(DEV_USART1, FROM_TX));
  uart_set_tx_monitor(DEV_USART3, usb_monitor(DEV_USART3, FROM_TX));
}

static void timer_1ms_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_TICK_1MS: {
      // i2c 异常检测
      i2c_abnormal_check(300);
      // 串口发送
      uart_tx_poll(DEV_USART1);
      uart_tx_poll(DEV_USART3);
      // usb cdc 发送
      (void)usb_tx_trans();
    } break;
    default: {
    } break;
  }
}

void timer_1ms_handle(TASK *task) {
  task_event_process(task, timer_1ms_cb);
}

/* 500ms 周期任务 */
void timer_500ms_init(void) {
  SYS_PARAM *sys = sys_param_get();

  task_event_subscribe(EVENT_TYPE_TICK_500MS, TASK_ID_TIMER_500MS);

  sys->ctrl.leds.value = 0xFF;
}

static void timer_500ms_cb(EVENT *ev) {
  SYS_PARAM *sys = sys_param_get();

  switch (ev->type) {
    case EVENT_TYPE_TICK_500MS: {
      if (sys->ctrl.update.stage != 0) {
        sys->ctrl.leds.value = 1 << (sys->ctrl.update.status << 13);
      }

      led_blink_control();

      task_delay_ms(TASK_ID_TIMER_500MS, 500);
    } break;
    default: {
    } break;
  }
}

void timer_500ms_handle(TASK *task) {
  task_event_process(task, timer_500ms_cb);
}

/* 循环任务 */
static int cmd_get_char(uint8_t *ch) {
  return !usb_rx_get(ch);
}

static int cmd_put_char(uint8_t ch) {
  usb_putchar(ch);
  return 1;
}

static void print_frame_usart(frame_parse_t *frame) {
  uart_puts(frame->dev_type, frame->data, frame->length);
}

static void system_ctrl_check(void) {
  SYS_PARAM *sys = sys_param_get();
  BOOT_PARAM param;

  switch (sys->ctrl.system) {
    case SYSTEM_CTRL_REBOOT: {
      printf_dbg("SYSTEM_CTRL_REBOOT\r\n");
      LL_mDelay(500);
      NVIC_SystemReset();
    } break;
    case SYSTEM_CTRL_UPDATE_START: {
      boot_param_get(&param);

      param.update_needed = 1;
      if (boot_param_update(&param)) {
        Error_Handler();
      }

      printf_dbg("SYSTEM_CTRL_UPDATE_START\r\n");
      NVIC_SystemReset();
    } break;
    default: {
    } break;
  }
}

void main_loop_init(void) {
  xcmd_init(cmd_get_char, cmd_put_char);
  mycmd_init();

  uart_set_rx_monitor(DEV_USART1, usb_monitor(DEV_USART1, FROM_RX));
  uart_set_rx_monitor(DEV_USART3, usb_monitor(DEV_USART3, FROM_RX));

  frame_parse_register(DEV_USART1, FRAME_TYPE_DEBUG, print_frame_usart);
  frame_parse_register(DEV_USART1, FRAME_TYPE_I2C_WRITE, uart_frame_i2c_write);
  frame_parse_register(DEV_USART1, FRAME_TYPE_I2C_READ, uart_frame_i2c_read);
  frame_parse_register(DEV_USART1, FRAME_TYPE_SYSTEM_CTRL, system_ctrl_frame_parse);
  frame_parse_register(DEV_USART1, FRAME_TYPE_UPDATE_DATA, update_frame_parse);
  frame_parse_register(DEV_USART1, FRAME_TYPE_UPDATE_STATUS, update_status_get);
  frame_parse_register(DEV_USART3, FRAME_TYPE_DEBUG, print_frame_usart);
  frame_parse_register(DEV_USART3, FRAME_TYPE_I2C_WRITE, uart_frame_i2c_write);
  frame_parse_register(DEV_USART3, FRAME_TYPE_I2C_READ, uart_frame_i2c_read);
  frame_parse_register(DEV_USART3, FRAME_TYPE_SYSTEM_CTRL, system_ctrl_frame_parse);
  frame_parse_register(DEV_USART3, FRAME_TYPE_UPDATE_DATA, update_frame_parse);
  frame_parse_register(DEV_USART3, FRAME_TYPE_UPDATE_STATUS, update_status_get);
}

void main_loop_handle(TASK *task) {
  SYS_PARAM *sys = sys_param_get();
  uint8_t ch;

  switch (sys->status) {
    case STATUS_SHELL: {
      // USB CDC Shell
      xcmd_task();
      // USART1 帧解析
      uart_frame_parse(DEV_USART1);
      // USART3 帧解析
      uart_frame_parse(DEV_USART3);
    } break;
    case STATUS_USB_AS_USART1: {
      // USART3 帧解析
      uart_frame_parse(DEV_USART3);
      // 清理 usart1_rx 缓冲区
      uart_read(DEV_USART1, &ch, 1);
    } break;
    case STATUS_USB_AS_USART3: {
      // USART1 帧解析
      uart_frame_parse(DEV_USART1);
      // 清理 usart3_rx 缓冲区
      uart_read(DEV_USART3, &ch, 1);
    } break;
    default: {
    } break;
  }

  // 固件升级
  update_pkg_process();

  // 系统控制
  system_ctrl_check();

  task_update_times(task);
}
