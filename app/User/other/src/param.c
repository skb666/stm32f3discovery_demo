#include <string.h>
#include "param.h"

static SYS_PARAM sys_param = {0};

SYS_PARAM *sys_param_get(void) {
  return &sys_param;
}

void sys_param_init(void) {
  memset(&sys_param, 0, sizeof(SYS_PARAM));
}
