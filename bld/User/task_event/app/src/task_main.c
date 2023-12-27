#include <stdio.h>

#include "device.h"
#include "main.h"
#include "task.h"
#include "timer.h"

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

/* 1ms 周期任务 */
void timer_1ms_init(void) {
  task_event_subscribe(EVENT_TYPE_TICK_1MS, TASK_ID_TIMER_1MS);
}

static void timer_1ms_cb(EVENT *ev) {
  switch (ev->type) {
    case EVENT_TYPE_TICK_1MS: {
      // 串口发送
      uart_tx_poll(DEV_USART1);
    } break;
    default: {
    } break;
  }
}

void timer_1ms_handle(TASK *task) {
  task_event_process(task, timer_1ms_cb);
}

/* 循环任务 */
static void print_frame_usart1(frame_parse_t *frame) {
  uart_puts(DEV_USART1, frame->data, frame->length);
}

void main_loop_init(void) {
  frame_parse_register(DEV_USART1, FRAME_TYPE_DEBUG, print_frame_usart1);
}

void main_loop_handle(TASK *task) {
  // USART1 帧解析
  uart_frame_parse(DEV_USART1);

  task_update_times(task);
}
