#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "param.h"
#include "xcmd.h"

typedef struct {
  GPIO_TypeDef *port;
  uint32_t pin;
} LED_GPIO;

typedef enum {
  LED_LD3,
  LED_LD4,
  LED_LD5,
  LED_LD6,
  LED_LD7,
  LED_LD8,
  LED_LD9,
  LED_LD10,
  LED_NUM,
} LED_TypeDef;

const static LED_GPIO led_list[LED_NUM] = {
    {LD3_GPIO_Port, LD3_Pin},
    {LD5_GPIO_Port, LD5_Pin},
    {LD7_GPIO_Port, LD7_Pin},
    {LD9_GPIO_Port, LD9_Pin},
    {LD10_GPIO_Port, LD10_Pin},
    {LD8_GPIO_Port, LD8_Pin},
    {LD6_GPIO_Port, LD6_Pin},
    {LD4_GPIO_Port, LD4_Pin},
};

const static uint8_t led_index[LED_NUM] = {
    [LED_LD3] = 0,
    [LED_LD4] = 7,
    [LED_LD5] = 1,
    [LED_LD6] = 6,
    [LED_LD7] = 2,
    [LED_LD8] = 5,
    [LED_LD9] = 3,
    [LED_LD10] = 4,
};

static void led_reset(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL leds = sys->ctrl.leds;
  size_t i = 0;

  for (i = 0; i < LED_NUM; ++i, leds.value >>= 1) {
    LL_GPIO_ResetOutputPin(led_list[i].port, led_list[i].pin);
  }
}

void led_blink_control(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL leds = sys->ctrl.leds;
  size_t i = 0;

  for (i = 0; i < LED_NUM; ++i, leds.value >>= 1) {
    if (leds.value & 1) {
      LL_GPIO_TogglePin(led_list[i].port, led_list[i].pin);
    } else {
      LL_GPIO_ResetOutputPin(led_list[i].port, led_list[i].pin);
    }
  }
}

/**
 * @brief led command
 */
int shell_led_cmd(int argc, char *argv[]) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL *leds = &sys->ctrl.leds;
  uint8_t index = 0;
  uint8_t sw = 0;

  if (argc > 1) {
    if (!strcmp("all", argv[1])) {
      if (argc <= 2) {
        xcmd_print("led: 0x%02x\r\n", leds->value);
        return 0;
      }
      if (sscanf(argv[2], "%hx", &leds->value) > 0) {
        xcmd_print("led control: 0x%02x\r\n", leds->value);
        led_reset();
      } else {
        xcmd_print("param error!\r\n");
      }
    } else if (sscanf(argv[1], "%hhu", &index) > 0 && index >= 3 && index <= 10) {
      index -= 3;
      if (argc <= 2) {
        xcmd_print("LD%hhu: %d\r\n", index + 3, !!(leds->value & (1 << led_index[index])));
        return 0;
      }
      if (sscanf(argv[2], "%hhu", &sw) > 0 && sw < 2) {
        if (sw) {
          leds->value |= (1 << led_index[index]);
        } else {
          leds->value &= ~(1 << led_index[index]);
        }
        xcmd_print("LD%hhu %s\r\n", index + 3, sw ? "ON" : "OFF");
        led_reset();
      } else {
        xcmd_print("param error!\r\n");
      }
    } else {
      xcmd_print("invalid arguments!\r\n");
    }
  } else {
    xcmd_print("led need more arguments!\r\n");
  }

  return 0;
}
