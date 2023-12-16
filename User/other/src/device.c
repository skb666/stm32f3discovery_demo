#include "device.h"

#include "common.h"
#include "main.h"

#define UART_TX_RING_SIZE 1280
#define UART_RX_RING_SIZE 1280
#define UART_DMATX_BUF_SIZE 256
#define UART_DMARX_BUF_SIZE 256

#define FRAME_HEAD1 0x55
#define FRAME_HEAD2 0xAA

#define FUNC_LIST_MAX 254

typedef enum {
  PARSE_STAT_HEAD1 = 0,
  PARSE_STAT_HEAD2,
  PARSE_STAT_ID,
  PARSE_STAT_LENGTH,
  PARSE_STAT_DATA,
} FRAME_PARSE_STAT;

typedef struct {
  volatile uint16_t status; /* 发送状态 */
  uint16_t last_dmarx_size; /* dma上一次接收数据大小 */
} uart_device_t;

char print_buf[DEV_NUM][64];

static void (*func_list[DEV_NUM][FUNC_LIST_MAX])(frame_parse_t *);

static uint8_t __uart_tx_ring_data[DEV_NUM][UART_TX_RING_SIZE];
_CCM_DATA static RING_FIFO uart_tx_ring[DEV_NUM] = {
    [DEV_USART1] = {
        .buffer = __uart_tx_ring_data[DEV_USART1],
        .capacity = UART_TX_RING_SIZE,
        .element_size = sizeof(__uart_tx_ring_data[DEV_USART1][0]),
        .cover = 1,
        .head = 0,
        .tail = 0,
        .size = 0,
    },
    [DEV_USART3] = {
        .buffer = __uart_tx_ring_data[DEV_USART3],
        .capacity = UART_TX_RING_SIZE,
        .element_size = sizeof(__uart_tx_ring_data[DEV_USART3][0]),
        .cover = 1,
        .head = 0,
        .tail = 0,
        .size = 0,
    },
};
static uint8_t __uart_rx_ring_data[DEV_NUM][UART_RX_RING_SIZE];
_CCM_DATA static RING_FIFO uart_rx_ring[DEV_NUM] = {
    [DEV_USART1] = {
        .buffer = __uart_rx_ring_data[DEV_USART1],
        .capacity = UART_RX_RING_SIZE,
        .element_size = sizeof(__uart_rx_ring_data[DEV_USART1][0]),
        .cover = 1,
        .head = 0,
        .tail = 0,
        .size = 0,
    },
    [DEV_USART3] = {
        .buffer = __uart_rx_ring_data[DEV_USART3],
        .capacity = UART_RX_RING_SIZE,
        .element_size = sizeof(__uart_rx_ring_data[DEV_USART3][0]),
        .cover = 1,
        .head = 0,
        .tail = 0,
        .size = 0,
    },
};
static uint8_t uart_dmatx_buf[DEV_NUM][UART_DMATX_BUF_SIZE];
static uint8_t uart_dmarx_buf[DEV_NUM][UART_DMARX_BUF_SIZE];

_CCM_DATA static uart_device_t uart_dev[DEV_NUM] = {0};
_CCM_DATA static frame_parse_t rx_frame[DEV_NUM] = {
    [DEV_USART1] = {
        .status = PARSE_STAT_HEAD1,
    },
    [DEV_USART3] = {
        .status = PARSE_STAT_HEAD1,
    },
};

void uart_config(DEV_TYPE dev_type) {
  switch (dev_type) {
    case DEV_USART1: {
      /* USART_RX DMA */
      LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_5,
          LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE),
          (uint32_t)uart_dmarx_buf[DEV_USART1],
          LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5));
      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, UART_DMARX_BUF_SIZE);

      /* USART_TX DMA */
      LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_4,
          (uint32_t)uart_dmatx_buf[DEV_USART1],
          LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT),
          LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4));

      LL_DMA_ClearFlag_HT5(DMA1);
      LL_DMA_ClearFlag_TC5(DMA1);
      LL_DMA_ClearFlag_TE5(DMA1);

      LL_DMA_ClearFlag_TC4(DMA1);
      LL_DMA_ClearFlag_TE4(DMA1);

      LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_5);
      LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);
      LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);

      LL_USART_EnableDMAReq_RX(USART1);
      LL_USART_EnableDMAReq_TX(USART1);
      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);

      LL_USART_EnableIT_IDLE(USART1);
    } break;
    case DEV_USART3: {
      /* USART_RX DMA */
      LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
          LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_RECEIVE),
          (uint32_t)uart_dmarx_buf[DEV_USART3],
          LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, UART_DMARX_BUF_SIZE);

      /* USART_TX DMA */
      LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
          (uint32_t)uart_dmatx_buf[DEV_USART3],
          LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT),
          LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));

      LL_DMA_ClearFlag_HT3(DMA1);
      LL_DMA_ClearFlag_TC3(DMA1);
      LL_DMA_ClearFlag_TE3(DMA1);

      LL_DMA_ClearFlag_TC2(DMA1);
      LL_DMA_ClearFlag_TE2(DMA1);

      LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_3);
      LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
      LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);

      LL_USART_EnableDMAReq_RX(USART3);
      LL_USART_EnableDMAReq_TX(USART3);
      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

      LL_USART_EnableIT_IDLE(USART3);
    } break;
    default: {
    } break;
  }
}

/**
 * @brief  串口dma接收完成中断处理
 * @param
 * @retval
 */
void uart_dmarx_done_isr(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t)) {
  uint16_t recv_size;

  recv_size = UART_DMARX_BUF_SIZE - uart_dev[dev_type].last_dmarx_size;

  disable_global_irq();
  ring_push_mult(&uart_rx_ring[dev_type], &uart_dmarx_buf[dev_type][uart_dev[dev_type].last_dmarx_size], recv_size);
  enable_global_irq();

  if (func) {
    func(&uart_dmarx_buf[dev_type][uart_dev[dev_type].last_dmarx_size], recv_size);
  }

  uart_dev[dev_type].last_dmarx_size = 0;
}

/**
 * @brief  串口dma接收部分数据中断处理
 * @param
 * @retval
 */
void uart_dmarx_part_done_isr(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t)) {
  uint16_t recv_total_size;
  uint16_t recv_size;

  switch (dev_type) {
    case DEV_USART1: {
      recv_total_size = UART_DMARX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_5);
    } break;
    case DEV_USART3: {
      recv_total_size = UART_DMARX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_3);
    } break;
    default: {
    } break;
  }
  recv_size = recv_total_size - uart_dev[dev_type].last_dmarx_size;

  disable_global_irq();
  ring_push_mult(&uart_rx_ring[dev_type], &uart_dmarx_buf[dev_type][uart_dev[dev_type].last_dmarx_size], recv_size);
  enable_global_irq();

  if (func) {
    func(&uart_dmarx_buf[dev_type][uart_dev[dev_type].last_dmarx_size], recv_size);
  }

  uart_dev[dev_type].last_dmarx_size = recv_total_size;
}

/**
 * @brief  串口dma发送完成中断处理
 * @param
 * @retval
 */
void uart_dmatx_done_isr(DEV_TYPE dev_type) {
  uart_dev[dev_type].status = 0; /* DMA发送空闲 */
}

void uart_wait_tx(DEV_TYPE dev_type) {
  while (uart_dev[dev_type].status) {
  }
}

void uart_tx_poll(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t)) {
  uint16_t size = 0;

  if (uart_dev[dev_type].status) {
    return;
  }

  if (ring_is_empty(&uart_tx_ring[dev_type])) {
    return;
  }

  disable_global_irq();
  size = ring_pop_mult(&uart_tx_ring[dev_type], uart_dmatx_buf[dev_type], UART_DMATX_BUF_SIZE);
  enable_global_irq();

  if (func) {
    func(uart_dmatx_buf[dev_type], size);
  }

  uart_dev[dev_type].status = 1;

  switch (dev_type) {
    case DEV_USART1: {
      LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, size);
      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    } break;
    case DEV_USART3: {
      LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, size);
      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    } break;
    default: {
    } break;
  }
}

uint16_t uart_read(DEV_TYPE dev_type, uint8_t *buf, uint16_t size) {
  uint16_t ok = 0;

  if (buf == NULL) {
    return 0;
  }

  if (ring_is_empty(&uart_rx_ring[dev_type])) {
    return 0;
  }

  disable_global_irq();
  ok = ring_pop_mult(&uart_rx_ring[dev_type], buf, size);
  enable_global_irq();

  return ok;
}

uint16_t uart_write(DEV_TYPE dev_type, const uint8_t *buf, uint16_t size) {
  uint16_t ok = 0;

  if (buf == NULL) {
    return 0;
  }

  disable_global_irq();
  ok = ring_push_mult(&uart_tx_ring[dev_type], buf, size);
  enable_global_irq();

  return ok;
}

void change_byte_order(uint8_t *addr, size_t size) {
  uint8_t tmp;
  size_t i, imax = size / 2;

  for (i = 0; i < imax; ++i) {
    tmp = addr[i];
    addr[i] = addr[size - i - 1];
    addr[size - i - 1] = tmp;
  }
}

int8_t frame_parse_register(DEV_TYPE dev_type, uint8_t index, void (*func)(frame_parse_t *)) {
  if (func == 0 || index >= FUNC_LIST_MAX) {
    return -1;
  }

  if (func_list[dev_type][index] == 0) {
    func_list[dev_type][index] = func;
    return 0;
  } else {
    return -1;
  }
}

void uart_frame_parse(DEV_TYPE dev_type) {
  uint16_t size = 0;
  uint8_t rx;

  switch (rx_frame[dev_type].status) {
    case PARSE_STAT_HEAD1: {
      size = uart_read(dev_type, &rx, 1);
      if (size) {
        if (rx == FRAME_HEAD1) {
          rx_frame[dev_type].status = PARSE_STAT_HEAD2;
        }
      }
    } break;
    case PARSE_STAT_HEAD2: {
      size = uart_read(dev_type, &rx, 1);
      if (size) {
        if (rx == FRAME_HEAD2) {
          rx_frame[dev_type].status = PARSE_STAT_ID;
        } else {
          rx_frame[dev_type].status = PARSE_STAT_HEAD1;
        }
      }
    } break;
    case PARSE_STAT_ID: {
      size = uart_read(dev_type, &rx_frame[dev_type].id, 1);
      if (size) {
        if (rx_frame[dev_type].id == 0xff) {
          rx_frame[dev_type].byte_order = 1;
          rx_frame[dev_type].status = PARSE_STAT_HEAD1;
        } else if (rx_frame[dev_type].id == 0xfe) {
          rx_frame[dev_type].byte_order = 0;
          rx_frame[dev_type].status = PARSE_STAT_HEAD1;
        } else if (rx_frame[dev_type].id < FUNC_LIST_MAX && func_list[dev_type][rx_frame[dev_type].id]) {
          rx_frame[dev_type].status = PARSE_STAT_LENGTH;
        } else {
          rx_frame[dev_type].status = PARSE_STAT_HEAD1;
          //Error_Handler();
        }
      }
    } break;
    case PARSE_STAT_LENGTH: {
      size = uart_read(dev_type, (uint8_t *)&rx_frame[dev_type].length + rx_frame[dev_type].recv_size, sizeof(rx_frame[dev_type].length) - rx_frame[dev_type].recv_size);
      if (size) {
        rx_frame[dev_type].recv_size += size;
      }
      if (rx_frame[dev_type].recv_size >= sizeof(rx_frame[dev_type].length)) {
        if (rx_frame[dev_type].byte_order) {
          change_byte_order((uint8_t *)&rx_frame[dev_type].length, sizeof(rx_frame[dev_type].length));
        }
        if (rx_frame[dev_type].length > FRAME_DATA_LEN_MAX) {
#ifdef DEBUG
          printf("frame length error!!! (%hu)\n", rx_frame[dev_type].length);
#endif
          rx_frame[dev_type].status = PARSE_STAT_HEAD1;
        } else {
          rx_frame[dev_type].status = PARSE_STAT_DATA;
        }
        rx_frame[dev_type].recv_size = 0;
      }
    } break;
    case PARSE_STAT_DATA: {
      size = uart_read(dev_type, rx_frame[dev_type].data + rx_frame[dev_type].recv_size, rx_frame[dev_type].length - rx_frame[dev_type].recv_size);
      if (size) {
        rx_frame[dev_type].recv_size += size;
      }
      if (rx_frame[dev_type].recv_size >= rx_frame[dev_type].length) {
        rx_frame[dev_type].status = PARSE_STAT_HEAD1;
        rx_frame[dev_type].recv_size = 0;
        func_list[dev_type][rx_frame[dev_type].id](&rx_frame[dev_type]);
      }
    } break;
    default: {
#ifdef DEBUG
      printf("frame status error!!!\n");
#endif
    } break;
  }
}
