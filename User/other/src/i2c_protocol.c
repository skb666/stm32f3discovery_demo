#include "i2c_protocol.h"

#include "common.h"
#include "i2c_slave.h"
#include "param.h"

/* VERSION: 0.0.1.0 */
#define MCU_VERSION (0x0010)

#define I2C_PUT_NUM(Type, Value)                          \
  do {                                                    \
    uint8_t __data;                                       \
    Type __value = Value;                                 \
    change_byte_order((uint8_t *)&__value, sizeof(Type)); \
    i2c_tx_put((uint8_t *)&__value, sizeof(Type));        \
  } while (0)

#define I2C_GET_NUM(Type, Value)                \
  do {                                          \
    uint8_t __data;                             \
    Value = 0;                                  \
    for (size_t i = 0; i < sizeof(Type); ++i) { \
      if (i2c_rx_get(&__data, 1)) {             \
        Value <<= 8;                            \
        Value |= __data;                        \
      }                                         \
    }                                           \
  } while (0)

extern void led_all_control(void);

void version_read_cb(void) {
  I2C_PUT_NUM(uint16_t, MCU_VERSION);
}

void led_read_cb(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL *leds = &sys->ctrl.leds;

  I2C_PUT_NUM(uint16_t, leds->value);
}

void led_write_cb(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL *leds = &sys->ctrl.leds;
  uint16_t value;

  if (!i2c_rx_size()) {
    return;
  }

  I2C_GET_NUM(uint16_t, value);
  leds->value = value;

  led_all_control();
}

static REG_T s_reg_list[] = {
    {0x0000, REG_RO, version_read_cb, NULL},
    {0x0001, REG_RW, led_read_cb, led_write_cb},
};
const static uint32_t s_reg_list_size = ARRAY_SIZE(s_reg_list);

void *reg_list_get(uint32_t *num) {
  *num = s_reg_list_size;
  return s_reg_list;
}

// TODO: 按照 REG_T.addr 排序
// TODO: 根据 reg_addr 进行二分查找 REG_T
