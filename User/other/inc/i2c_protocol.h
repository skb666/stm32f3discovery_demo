#ifndef __I2C_PROTOCOL_H__
#define __I2C_PROTOCOL_H__

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

void *reg_list_get(uint32_t *num);

#ifdef __cplusplus
}
#endif

#endif
