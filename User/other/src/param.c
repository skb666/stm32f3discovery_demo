#include "param.h"

static SYS_PARAM sys_param = {0};

SYS_PARAM *sys_param_get(void) {
  return &sys_param;
}

void sys_param_init(void) {
  sys_param.ctrl.leds.value = 0;
  
  sys_param.flag.serial_to_usb = 0;
}
