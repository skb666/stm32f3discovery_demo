#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ring_fifo.h"

#define FRAME_DATA_LEN_MAX 1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t status;
  uint8_t id;
  uint8_t byte_order;
  uint16_t length;
  uint16_t recv_size;
  uint8_t data[FRAME_DATA_LEN_MAX];
} frame_parse_t;

extern char print_buf[];

void uart_config(void);
void uart_dmarx_done_isr(void (*func)(uint8_t *, uint16_t));
void uart_dmarx_part_done_isr(void (*func)(uint8_t *, uint16_t));
void uart_dmatx_done_isr(void);

void uart_wait_tx(void);
void uart_tx_poll(void (*func)(uint8_t *, uint16_t));
uint16_t uart_read(uint8_t *buf, uint16_t size);
uint16_t uart_write(const uint8_t *buf, uint16_t size);

void change_byte_order(uint8_t *addr, size_t size);
int8_t frame_parse_register(uint8_t index, void (*func)(frame_parse_t *));
void uart_frame_parse(void);

#define usart3_printf(fmt, args...)                      \
  do {                                                   \
    uart_wait_tx();                                      \
    sprintf((char *)print_buf, fmt, ##args);             \
    uart_write((uint8_t *)print_buf, strlen(print_buf)); \
    uart_wait_tx();                                      \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
