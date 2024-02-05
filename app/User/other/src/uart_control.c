#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "param.h"
#include "uart_device.h"
#include "xcmd.h"

int shell_usart_cmd(int argc, char *argv[]) {
  int index = 1;
  DEV_TYPE dev_type;
  uint8_t mode = 0;
  uint8_t data;
  int ret;
  SYS_PARAM *sys = sys_param_get();

  if (argc < 3) {
    xcmd_print("usart need more arguments!\r\n");
    return 0;
  }

  switch (atoi(argv[index])) {
    case 1: {
      dev_type = DEV_USART1;
    } break;
    case 3: {
      dev_type = DEV_USART3;
    } break;
    default: {
      return -1;
    } break;
  }

  index += 1;

  if (strcmp(argv[index], "-r") == 0) {
    mode = 1;
  } else if (strcmp(argv[index], "-t") == 0) {
    mode = 2;
  } else if (strcmp(argv[index], "-s") == 0) {
    mode = 3;
  } else if (strcmp(argv[index], "-x") == 0) {
    mode = 4;
  }

  index += 1;

  if (argc <= index) {
    switch (mode) {
      case 0: {
        xcmd_print("param error!\r\n");
      } break;
      case 1: {
        switch (dev_type) {
          case DEV_USART1: {
            xcmd_print("%s rx monitor is %s\r\n", "usart1", (sys->flag.usb_tx.usart1_rx ? "on" : "off"));
          } break;
          case DEV_USART3: {
            xcmd_print("%s rx monitor is %s\r\n", "usart3", (sys->flag.usb_tx.usart3_rx ? "on" : "off"));
          } break;
          default: {
          } break;
        }
      } break;
      case 2: {
        switch (dev_type) {
          case DEV_USART1: {
            xcmd_print("%s tx monitor is %s\r\n", "usart1", (sys->flag.usb_tx.usart1_tx ? "on" : "off"));
          } break;
          case DEV_USART3: {
            xcmd_print("%s tx monitor is %s\r\n", "usart3", (sys->flag.usb_tx.usart3_tx ? "on" : "off"));
          } break;
          default: {
          } break;
        }
      } break;
      default: {
        xcmd_print("missing param!\r\n");
      } break;
    }
    return 0;
  }

  switch (mode) {
    default: {
      xcmd_print("param error!\r\n");
    } break;
    case 1: {
      ret = sscanf(argv[index], "%hhu", &data);
      if (ret < 1) {
        xcmd_print("param error!\r\n");
        return 0;
      }

      switch (dev_type) {
        case DEV_USART1: {
          if (data) {
            sys->flag.usb_tx.usart1_rx = 1;
            xcmd_print("%s rx monitor %s\r\n", "usart1", "on");
          } else {
            sys->flag.usb_tx.usart1_rx = 0;
            xcmd_print("%s rx monitor %s\r\n", "usart1", "off");
          }
        } break;
        case DEV_USART3: {
          if (data) {
            sys->flag.usb_tx.usart3_rx = 1;
            xcmd_print("%s rx monitor %s\r\n", "usart3", "on");
          } else {
            sys->flag.usb_tx.usart3_rx = 0;
            xcmd_print("%s rx monitor %s\r\n", "usart3", "off");
          }
        } break;
        default: {
        } break;
      }
    } break;
    case 2: {
      ret = sscanf(argv[index], "%hhu", &data);
      if (ret < 1) {
        xcmd_print("param error!\r\n");
        return 0;
      }

      switch (dev_type) {
        case DEV_USART1: {
          if (data) {
            sys->flag.usb_tx.usart1_tx = 1;
            xcmd_print("%s tx monitor %s\r\n", "usart1", "on");
          } else {
            sys->flag.usb_tx.usart1_tx = 0;
            xcmd_print("%s tx monitor %s\r\n", "usart1", "off");
          }
        } break;
        case DEV_USART3: {
          if (data) {
            sys->flag.usb_tx.usart3_tx = 1;
            xcmd_print("%s tx monitor %s\r\n", "usart3", "on");
          } else {
            sys->flag.usb_tx.usart3_tx = 0;
            xcmd_print("%s tx monitor %s\r\n", "usart3", "off");
          }
        } break;
        default: {
        } break;
      }
    } break;
    case 3: {
      while (index < argc) {
        uart_printf(dev_type, "%s\r\n", argv[index]);

        index += 1;
      }
    } break;
    case 4: {
      while (index < argc) {
        ret = sscanf(argv[index], "%hhx", &data);
        if (ret < 1) {
          xcmd_print("param error!\r\n");
          return 0;
        }

        uart_puts(dev_type, &data, 1);

        index += 1;
      }
    } break;
  }

  return 0;
}
