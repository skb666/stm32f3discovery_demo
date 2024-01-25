#include "i2c_reg_list.h"

#include "common.h"
#include "i2c_protocol.h"
#include "i2c_slave.h"

static REG_T s_reg_list[] = {
    {REG_VERSION, REG_RO, reg_read_cb_version, NULL},
    {REG_LED, REG_RW, reg_read_cb_led, reg_write_cb_led},
};
const static uint32_t s_reg_list_size = ARRAY_SIZE(s_reg_list);

void *reg_list_get(uint32_t *num) {
  *num = s_reg_list_size;
  return s_reg_list;
}
