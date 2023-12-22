#ifndef __I2C_SLAVE_H__
#define __I2C_SLAVE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  REG_RO = 0,
  REG_RW,
} REG_ATTRIB;

typedef struct {
  uint16_t addr;
  REG_ATTRIB attrib;
  void (*read_cb)(void);
  void (*write_cb)(void);
} REG_T;

void i2c_slave_config(void);
void i2c_ev_isr(void);

uint16_t i2c_tx_get(uint8_t *buf, uint16_t size);
uint16_t i2c_tx_put(const uint8_t *buf, uint16_t size);
uint16_t i2c_rx_get(uint8_t *buf, uint16_t size);
uint16_t i2c_rx_put(const uint8_t *buf, uint16_t size);
uint16_t i2c_tx_size(void);
uint16_t i2c_rx_size(void);

#ifdef __cplusplus
}
#endif

#endif
