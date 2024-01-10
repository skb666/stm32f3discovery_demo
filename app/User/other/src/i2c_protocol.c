#include "i2c_protocol.h"

#include "common.h"
#include "i2c_slave.h"
#include "param.h"

#define I2C_PUT_NUM(Type, Value)                         \
  do {                                                   \
    uint8_t __data;                                      \
    Type __value = Value;                                \
    change_byte_order(&__value, sizeof(Type));           \
    i2c_slave_tx_put((uint8_t *)&__value, sizeof(Type)); \
  } while (0)

#define I2C_GET_NUM(Type, Value)                \
  do {                                          \
    uint8_t __data;                             \
    Value = 0;                                  \
    for (size_t i = 0; i < sizeof(Type); ++i) { \
      if (i2c_slave_rx_get(&__data, 1)) {       \
        Value <<= 8;                            \
        Value |= __data;                        \
      }                                         \
    }                                           \
  } while (0)

extern void led_all_control(void);

void reg_read_cb_version(void) {
  I2C_PUT_NUM(uint16_t, MCU_SOFTWARE_VERSION);
}

void reg_read_cb_led(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL *leds = &sys->ctrl.leds;

  I2C_PUT_NUM(uint16_t, leds->value);
}

void reg_write_cb_led(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL *leds = &sys->ctrl.leds;
  uint16_t value;

  if (i2c_slave_rx_size() < sizeof(uint16_t)) {
    return;
  }

  I2C_GET_NUM(uint16_t, value);
  leds->value = value;

  led_all_control();
}
