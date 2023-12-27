#include "param.h"

#include <string.h>

static SYS_PARAM sys_param = {0};

SYS_PARAM *sys_param_get(void) {
  return &sys_param;
}

void sys_param_init(void) {
  memset(&sys_param, 0, sizeof(SYS_PARAM));

  sys_param.ctrl.update.stage = UPDATE_BEGIN;
  *(uint16_t *)&sys_param.ctrl.update.status = 0xEFFF;
}
