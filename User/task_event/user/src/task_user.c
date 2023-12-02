#include "common.h"
#include "task.h"

extern void debug_print_init(void);
extern void debug_print_handle(TASK *task);
extern void key_scan_init(void);
extern void key_scan_handle(TASK *task);
extern void usart1_tx_init(void);
extern void usart1_tx_handle(TASK *task);
extern void usart1_rx_init(void);
extern void usart1_rx_handle(TASK *task);

static TASK s_task_list[] = {
    {
        .id = TASK_ID_DEBUG_PRINT,
        .times = -1,
        .delay = 0,
        .init = debug_print_init,
        .handle = debug_print_handle,
    },
    {
        .id = TASK_ID_KEY_SCAN,
        .times = -1,
        .delay = 0,
        .init = key_scan_init,
        .handle = key_scan_handle,
    },
    {
        .id = TASK_ID_USART1_TX,
        .times = -1,
        .delay = 0,
        .init = usart1_tx_init,
        .handle = usart1_tx_handle,
    },
    {
        .id = TASK_ID_USART1_RX,
        .times = -1,
        .delay = 0,
        .init = usart1_rx_init,
        .handle = usart1_rx_handle,
    },
};
const static uint32_t s_task_list_size = ARRAY_SIZE(s_task_list);

void *task_list_get(uint32_t *num) {
  *num = s_task_list_size;
  return s_task_list;
}
