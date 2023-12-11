#include "device.h"

#include "common.h"
#include "main.h"

#define UART_TX_RING_SIZE 1280
#define UART_RX_RING_SIZE 1280
#define UART_DMATX_BUF_SIZE 256
#define UART_DMARX_BUF_SIZE 256

#define FRAME_HEAD1 0x55
#define FRAME_HEAD2 0xAA

#define FUNC_LIST_MAX 250

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

char print_buf[64];

static void (*func_list[FUNC_LIST_MAX])(frame_parse_t *);

ring_define_static(uint8_t _CCM_DATA, uart_tx_ring, UART_TX_RING_SIZE, 1);
ring_define_static(uint8_t _CCM_DATA, uart_rx_ring, UART_RX_RING_SIZE, 1);
static uint8_t uart_dmatx_buf[UART_DMATX_BUF_SIZE];
static uint8_t uart_dmarx_buf[UART_DMARX_BUF_SIZE];

static uart_device_t uart_dev = {0};
static frame_parse_t rx_frame = {
    .status = PARSE_STAT_HEAD1,
};

void uart_config(void) {
  /* USART_RX DMA */
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_5,
      LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE),
      (uint32_t)uart_dmarx_buf,
      LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5));
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, UART_DMARX_BUF_SIZE);

  /* USART_TX DMA */
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_4,
      (uint32_t)uart_dmatx_buf,
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
}

/**
 * @brief  串口dma接收完成中断处理
 * @param
 * @retval
 */
void uart_dmarx_done_isr(void (*func)(uint8_t *, uint16_t)) {
  uint16_t recv_size;

  recv_size = UART_DMARX_BUF_SIZE - uart_dev.last_dmarx_size;

  disable_global_irq();
  ring_push_mult(&uart_rx_ring, &uart_dmarx_buf[uart_dev.last_dmarx_size], recv_size);
  enable_global_irq();

  if (func) {
    func(&uart_dmarx_buf[uart_dev.last_dmarx_size], recv_size);
  }

  uart_dev.last_dmarx_size = 0;
}

/**
 * @brief  串口dma接收部分数据中断处理
 * @param
 * @retval
 */
void uart_dmarx_part_done_isr(void (*func)(uint8_t *, uint16_t)) {
  uint16_t recv_total_size;
  uint16_t recv_size;

  recv_total_size = UART_DMARX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_5);
  recv_size = recv_total_size - uart_dev.last_dmarx_size;

  disable_global_irq();
  ring_push_mult(&uart_rx_ring, &uart_dmarx_buf[uart_dev.last_dmarx_size], recv_size);
  enable_global_irq();

  if (func) {
    func(&uart_dmarx_buf[uart_dev.last_dmarx_size], recv_size);
  }

  uart_dev.last_dmarx_size = recv_total_size;
}

/**
 * @brief  串口dma发送完成中断处理
 * @param
 * @retval
 */
void uart_dmatx_done_isr(void) {
  uart_dev.status = 0; /* DMA发送空闲 */
}

void uart_wait_tx(void) {
  while (uart_dev.status) {
  }
}

void uart_tx_poll(void (*func)(uint8_t *, uint16_t)) {
  uint16_t size = 0;

  if (uart_dev.status) {
    return;
  }

  if (ring_is_empty(&uart_tx_ring)) {
    return;
  }

  disable_global_irq();
  size = ring_pop_mult(&uart_tx_ring, uart_dmatx_buf, UART_DMATX_BUF_SIZE);
  enable_global_irq();

  if (func) {
    func(uart_dmatx_buf, size);
  }

  uart_dev.status = 1;

  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, size);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
}

uint16_t uart_read(uint8_t *buf, uint16_t size) {
  uint16_t ok = 0;

  if (buf == NULL) {
    return 0;
  }

  if (ring_is_empty(&uart_rx_ring)) {
    return 0;
  }

  disable_global_irq();
  ok = ring_pop_mult(&uart_rx_ring, buf, size);
  enable_global_irq();

  return ok;
}

uint16_t uart_write(const uint8_t *buf, uint16_t size) {
  uint16_t ok = 0;

  if (buf == NULL) {
    return 0;
  }

  disable_global_irq();
  ok = ring_push_mult(&uart_tx_ring, buf, size);
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

int8_t frame_parse_register(uint8_t index, void (*func)(frame_parse_t *)) {
  if (func == 0 || index >= FUNC_LIST_MAX) {
    return -1;
  }

  if (func_list[index] == 0) {
    func_list[index] = func;
    return 0;
  } else {
    return -1;
  }
}

void uart_frame_parse(void) {
  uint16_t size = 0;
  uint8_t rx;

  switch (rx_frame.status) {
    case PARSE_STAT_HEAD1: {
      size = uart_read(&rx, 1);
      if (size) {
        if (rx == FRAME_HEAD1) {
          rx_frame.status = PARSE_STAT_HEAD2;
        }
      }
    } break;
    case PARSE_STAT_HEAD2: {
      size = uart_read(&rx, 1);
      if (size) {
        if (rx == FRAME_HEAD2) {
          rx_frame.status = PARSE_STAT_ID;
        } else {
          rx_frame.status = PARSE_STAT_HEAD1;
        }
      }
    } break;
    case PARSE_STAT_ID: {
      size = uart_read(&rx_frame.id, 1);
      if (size) {
        if (rx_frame.id == 0xff) {
          rx_frame.byte_order = 1;
          rx_frame.status = PARSE_STAT_HEAD1;
        } else if (rx_frame.id == 0xfe) {
          rx_frame.byte_order = 0;
          rx_frame.status = PARSE_STAT_HEAD1;
        } else if (rx_frame.id < FUNC_LIST_MAX && func_list[rx_frame.id]) {
          rx_frame.status = PARSE_STAT_LENGTH;
        } else {
          rx_frame.status = PARSE_STAT_HEAD1;
          Error_Handler();
        }
      }
    } break;
    case PARSE_STAT_LENGTH: {
      size = uart_read((uint8_t *)&rx_frame.length + rx_frame.recv_size, sizeof(rx_frame.length) - rx_frame.recv_size);
      if (size) {
        rx_frame.recv_size += size;
      }
      if (rx_frame.recv_size >= sizeof(rx_frame.length)) {
        if (rx_frame.byte_order) {
          change_byte_order((uint8_t *)&rx_frame.length, sizeof(rx_frame.length));
        }
        if (rx_frame.length > FRAME_DATA_LEN_MAX) {
#ifdef DEBUG
          printf("frame length error!!! (%hu)\n", rx_frame.length);
#endif
          rx_frame.status = PARSE_STAT_HEAD1;
        } else {
          rx_frame.status = PARSE_STAT_DATA;
        }
        rx_frame.recv_size = 0;
      }
    } break;
    case PARSE_STAT_DATA: {
      size = uart_read(rx_frame.data + rx_frame.recv_size, rx_frame.length - rx_frame.recv_size);
      if (size) {
        rx_frame.recv_size += size;
      }
      if (rx_frame.recv_size >= rx_frame.length) {
        rx_frame.status = PARSE_STAT_HEAD1;
        rx_frame.recv_size = 0;
        func_list[rx_frame.id](&rx_frame);
      }
    } break;
    default: {
#ifdef DEBUG
      printf("frame status error!!!\n");
#endif
    } break;
  }
}
