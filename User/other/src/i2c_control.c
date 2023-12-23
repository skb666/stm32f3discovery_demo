#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i2c.h"
#include "main.h"
#include "xcmd.h"

uint8_t i2c_tx_buf[2] = {0x00, 0x00};
uint8_t i2c_rx_buf[2];

int shell_i2cdetect_cmd(int argc, char *argv[]) {
  HAL_StatusTypeDef res;
  uint16_t addr = 0;
  uint8_t filter = 1;

  if (argc > 1) {
    if (strncmp(argv[1], "-a", 2) == 0) {
      filter = 0;
    }
  }

  xcmd_print("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
  for (uint8_t i = 0; i < 8; ++i) {
    xcmd_print("%02hhx:", i * 16);
    for (uint8_t j = 0; j < 16; ++j) {
      addr = i * 16 + j;
      if (filter && (addr < 0x03 || addr >= 0x78)) {
        xcmd_print("   ");
        continue;
      }
      res = HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 2);
      if (res == HAL_OK) {
        xcmd_print(" %02hx", addr);
      } else {
        xcmd_print(" --");
      }
    }
    xcmd_print("\r\n");
  }

  uint8_t test[] = {0x00, 0x01, 0x00, 0xdd};

  do {
    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x33 << 1, test, sizeof(test)) != HAL_OK) {
      Error_Handler();
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
    }
  } while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

  do {
    if (HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x33 << 1, i2c_tx_buf, sizeof(i2c_tx_buf)) != HAL_OK) {
      Error_Handler();
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
    }
  } while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

  do {
    if (HAL_I2C_Master_Receive_DMA(&hi2c1, 0x33 << 1, i2c_rx_buf, sizeof(i2c_rx_buf)) != HAL_OK) {
      Error_Handler();
    }
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
    }
  } while (HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);

  for (size_t i = 0; i < sizeof(i2c_rx_buf); ++i) {
    printf("0x%02x ", i2c_rx_buf[i]);
  }
  printf("\r\n");

  return 0;
}
