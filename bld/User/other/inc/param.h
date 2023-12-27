#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UPDATE_BEGIN,
  UPDATE_WAITTING,
  UPDATE_TRANSMIT,
  UPDATE_END,
} UPDATE_STAGE;

typedef struct {
  uint16_t pkg_num : 12;
  uint16_t running : 1;
  uint16_t errno : 3;
} UPDATE_STATUS;

typedef struct {
  UPDATE_STAGE stage;
  UPDATE_STATUS status;
} UPDATE_CTRL;

typedef struct {
  UPDATE_CTRL update;
} SYS_CTRL;

typedef struct {
  SYS_CTRL ctrl;
} SYS_PARAM;

SYS_PARAM *sys_param_get(void);
void sys_param_init(void);

#ifdef __cplusplus
}
#endif

#endif
