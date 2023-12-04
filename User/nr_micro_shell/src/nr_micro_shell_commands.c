/**
 * @file      nr_micro_shell_commands.c
 * @author    Nrush
 * @version   V0.1
 * @date      28 Oct 2019
 * *****************************************************************************
 * @attention
 *
 * MIT License
 *
 * Copyright (C) 2019 Nrush. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "nr_micro_shell.h"
#include "param.h"

/**
 * @brief ls command
 */
void shell_ls_cmd(char argc, char *argv) {
  unsigned int i = 0;
  if (argc > 1) {
    if (!strcmp("cmd", &argv[argv[1]])) {
      for (i = 0; nr_shell.static_cmd[i].fp != NULL; i++) {
        shell_printf("%s", nr_shell.static_cmd[i].cmd);
        shell_printf("\r\n");
      }
    } else if (!strcmp("-v", &argv[argv[1]])) {
      shell_printf("ls version 1.0.\r\n");
    } else if (!strcmp("-h", &argv[argv[1]])) {
      shell_printf("useage: ls [options]\r\n");
      shell_printf("options: \r\n");
      shell_printf("\t -h \t: show help\r\n");
      shell_printf("\t -v \t: show version\r\n");
      shell_printf("\t cmd \t: show all commands\r\n");
    }
  } else {
    shell_printf("ls need more arguments!\r\n");
  }
}

/**
 * @brief test command
 */
void shell_test_cmd(char argc, char *argv) {
  unsigned int i;
  shell_printf("test command:\r\n");
  for (i = 0; i < argc; i++) {
    shell_printf("paras %d: %s\r\n", i, &(argv[argv[i]]));
  }
}

static void led_control(void) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL leds = (LED_CTRL)&sys->ctrl.leds;

  if (leds->LD3) {
    LL_GPIO_SetOutputPin(LD3_GPIO_Port, LD3_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD3_GPIO_Port, LD3_Pin);
  }

  if (leds->LD4) {
    LL_GPIO_SetOutputPin(LD4_GPIO_Port, LD4_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD4_GPIO_Port, LD4_Pin);
  }

  if (leds->LD5) {
    LL_GPIO_SetOutputPin(LD5_GPIO_Port, LD5_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD5_GPIO_Port, LD5_Pin);
  }

  if (leds->LD6) {
    LL_GPIO_SetOutputPin(LD6_GPIO_Port, LD6_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD6_GPIO_Port, LD6_Pin);
  }

  if (leds->LD7) {
    LL_GPIO_SetOutputPin(LD7_GPIO_Port, LD7_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD7_GPIO_Port, LD7_Pin);
  }

  if (leds->LD8) {
    LL_GPIO_SetOutputPin(LD8_GPIO_Port, LD8_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD8_GPIO_Port, LD8_Pin);
  }

  if (leds->LD9) {
    LL_GPIO_SetOutputPin(LD9_GPIO_Port, LD9_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD9_GPIO_Port, LD9_Pin);
  }

  if (leds->LD10) {
    LL_GPIO_SetOutputPin(LD10_GPIO_Port, LD10_Pin);
  } else {
    LL_GPIO_ResetOutputPin(LD10_GPIO_Port, LD10_Pin);
  }
}

/**
 * @brief led command
 */
void shell_led_cmd(char argc, char *argv) {
  SYS_PARAM *sys = sys_param_get();
  LED_CTRL leds = (LED_CTRL)&sys->ctrl.leds;
  uint8_t sw = 0;

  if (argc > 1) {
    if (!strcmp("all", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhx", &sys->ctrl.leds) > 0) {
        shell_printf("led control: 0x%02x\r\n", sys->ctrl.leds);
        led_control();
      }
    } else if (!strcmp("ld3", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD3 control: %s\r\n", sw ? "on" : "off");
        leds->LD3 = sw;
        led_control();
      }
    } else if (!strcmp("ld4", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD4 control: %s\r\n", sw ? "on" : "off");
        leds->LD4 = sw;
        led_control();
      }
    } else if (!strcmp("ld5", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD5 control: %s\r\n", sw ? "on" : "off");
        leds->LD5 = sw;
        led_control();
      }
    } else if (!strcmp("ld6", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD6 control: %s\r\n", sw ? "on" : "off");
        leds->LD6 = sw;
        led_control();
      }
    } else if (!strcmp("ld7", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD7 control: %s\r\n", sw ? "on" : "off");
        leds->LD7 = sw;
        led_control();
      }
    } else if (!strcmp("ld8", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD8 control: %s\r\n", sw ? "on" : "off");
        leds->LD8 = sw;
        led_control();
      }
    } else if (!strcmp("ld9", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD9 control: %s\r\n", sw ? "on" : "off");
        leds->LD9 = sw;
        led_control();
      }
    } else if (!strcmp("ld10", &argv[argv[1]])) {
      if (argc <= 2) {
        shell_printf("led need more arguments!\r\n");
      }
      if (sscanf(&argv[argv[2]], "%hhu", &sw) > 0 && sw < 2) {
        shell_printf("LD10 control: %s\r\n", sw ? "on" : "off");
        leds->LD10 = sw;
        led_control();
      }
    } else if (!strcmp("-h", &argv[argv[1]])) {
      shell_printf("useage: led [options]\r\n");
      shell_printf("options: \r\n");
      shell_printf("\t -h \t: show help\r\n");
    }
  } else {
    shell_printf("led need more arguments!\r\n");
  }
}

#ifdef NR_SHELL_USING_EXPORT_CMD
NR_SHELL_CMD_EXPORT(ls, shell_ls_cmd);
NR_SHELL_CMD_EXPORT(test, shell_test_cmd);
#else
const static_cmd_st static_cmd[] = {
    {"ls", shell_ls_cmd},
    {"test", shell_test_cmd},
    {"led", shell_led_cmd},
    {"\0", NULL},
};
#endif

/******************* (C) COPYRIGHT 2019 Nrush *****END OF FILE*****************/
