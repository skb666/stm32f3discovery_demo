#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UART_RX_RING_SIZE 1280
#define UART_TX_RING_SIZE 1280
#define UART_DMARX_BUF_SIZE 256
#define UART_DMATX_BUF_SIZE 256

#define FRAME_DATA_LEN_MAX 1280
#define FRAME_HEAD1 0x55
#define FRAME_HEAD2 0xAA
#define FUNC_LIST_MAX 254

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DEV_USART1,
  //DEV_USART2,
  DEV_USART3,
  DEV_NUM,
} DEV_TYPE;

typedef struct {
  uint8_t status;
  uint8_t id;
  uint8_t byte_order;
  uint16_t length;
  uint16_t recv_size;
  uint8_t *data;
} frame_parse_t;

extern char print_buf[DEV_NUM][64];

void uart_config(DEV_TYPE dev_type);
void uart_dmarx_done_isr(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t));
void uart_dmarx_part_done_isr(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t));
void uart_dmatx_done_isr(DEV_TYPE dev_type);

void uart_wait_tx(DEV_TYPE dev_type);
void uart_tx_poll(DEV_TYPE dev_type, void (*func)(uint8_t *, uint16_t));
uint16_t uart_read(DEV_TYPE dev_type, uint8_t *buf, uint16_t size);
uint16_t uart_write(DEV_TYPE dev_type, const uint8_t *buf, uint16_t size);

int8_t frame_parse_register(DEV_TYPE dev_type, uint8_t index, void (*func)(frame_parse_t *));
void uart_frame_parse(DEV_TYPE dev_type);

#define uart_printf(dev_type, fmt, args...)                                            \
  do {                                                                                 \
    sprintf((char *)print_buf[dev_type], fmt, ##args);                                 \
    uart_write(dev_type, (uint8_t *)print_buf[dev_type], strlen(print_buf[dev_type])); \
    uart_wait_tx(dev_type);                                                            \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
