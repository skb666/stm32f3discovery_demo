#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i2c.h"
#include "main.h"
#include "usbd_cdc_if.h"

int shell_i2cdetect_cmd(int argc, char *argv[]) {
  HAL_StatusTypeDef res;
  uint16_t addr = 0;
  uint8_t filter = 1;

  if (argc > 1) {
    if (strncmp(argv[1], "-a", 2) == 0) {
      filter = 0;
    }
  }

  usb_printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
  for (uint8_t i = 0; i < 8; ++i) {
    usb_printf("%02hhx:", i * 16);
    for (uint8_t j = 0; j < 16; ++j) {
      addr = i * 16 + j;
      if (filter && (addr < 0x03 || addr >= 0x78)) {
        usb_printf("   ");
        continue;
      }
      res = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 2);
      if (res == HAL_OK) {
        usb_printf(" %02hx", addr);
      } else {
        usb_printf(" --");
      }
    }
    usb_printf("\r\n");
  }

  return 0;
}
