#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "device.h"
#include "i2c.h"
#include "main.h"
#include "xcmd.h"

#define I2C_MASTER_HANDLE &hi2c1

typedef enum {
  I2C_READ = 0,
  I2C_WRITE,
} I2C_OPERATE;

static uint8_t i2c_buf[1280];
static uint16_t i2c_buf_size = 0;

static inline void i2c_master_write(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {
  do {
    if (HAL_I2C_Master_Transmit_DMA(I2C_MASTER_HANDLE, (DevAddress << 1), pData, Size) != HAL_OK) {
      xcmd_print("write error: %u\r\n", HAL_I2C_GetError(I2C_MASTER_HANDLE));
      return;
    }
    while (HAL_I2C_GetState(I2C_MASTER_HANDLE) != HAL_I2C_STATE_READY) {
    }
  } while (HAL_I2C_GetError(I2C_MASTER_HANDLE) == HAL_I2C_ERROR_AF);
}

static inline void i2c_master_read(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {
  do {
    if (HAL_I2C_Master_Receive_DMA(I2C_MASTER_HANDLE, (DevAddress << 1), pData, Size) != HAL_OK) {
      xcmd_print("read error: %u\r\n", HAL_I2C_GetError(I2C_MASTER_HANDLE));
      return;
    }
    while (HAL_I2C_GetState(I2C_MASTER_HANDLE) != HAL_I2C_STATE_READY) {
    }
  } while (HAL_I2C_GetError(I2C_MASTER_HANDLE) == HAL_I2C_ERROR_AF);
}

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
      res = HAL_I2C_IsDeviceReady(I2C_MASTER_HANDLE, addr << 1, 2, 2);
      if (res == HAL_OK) {
        xcmd_print(" %02hx", addr);
      } else {
        xcmd_print(" --");
      }
    }
    xcmd_print("\r\n");
  }

  return 0;
}

int shell_i2ctransfer_cmd(int argc, char *argv[]) {
  int index = 1;
  uint16_t dev_addr;
  I2C_OPERATE operate;
  int ret;
  uint16_t data;

  if (argc < 2) {
    xcmd_print("i2ctransfer need more arguments!\r\n");
    return 0;
  }

  /* 获取操作方式 */
  if (argv[index][0] == 'r') {
    operate = I2C_READ;
  } else if (argv[index][0] == 'w') {
    operate = I2C_WRITE;
  } else {
    xcmd_print("param error!\r\n");
    return 0;
  }

  /* 获取读写数据的数量、从机地址 */
  ret = sscanf(&argv[index][1], "%hu@%hx", &data, &dev_addr);
  if (ret != 2) {
    xcmd_print("param error!\r\n");
    return 0;
  }

  /* 判断从机是否存在 */
  if (HAL_I2C_IsDeviceReady(I2C_MASTER_HANDLE, (dev_addr << 1), 2, 2) != HAL_OK) {
    xcmd_print("the i2c slave does not exist\r\n");
    return 0;
  }

  index += 1;
  i2c_buf_size = data;

  if (operate == I2C_READ) { /* 读数据 */
    i2c_master_read(dev_addr, i2c_buf, i2c_buf_size);

    for (uint16_t i = 1; i <= i2c_buf_size; ++i) {
      xcmd_print("0x%02hhx ", i2c_buf[i - 1]);
      if (i % 16 == 0) {
        xcmd_print("\r\n");
      }
    }
    xcmd_print("\r\n");
  } else if (operate == I2C_WRITE) { /* 写数据 */
    for (uint16_t i = 0; i < i2c_buf_size; ++i) {
      ret = sscanf(argv[index], "%hx", &data);
      if (ret < 1) {
        xcmd_print("missing param!\r\n");
        return 0;
      }
      i2c_buf[i] = data;
      index += 1;
    }

    i2c_master_write(dev_addr, i2c_buf, i2c_buf_size);
  }

  if (argc <= index) {
    return 0;
  }

  /* 根据参数进行后续读写操作 */
  while (index < argc) {
    /* 获取操作方式 */
    if (argv[index][0] == 'r') {
      operate = I2C_READ;
    } else if (argv[index][0] == 'w') {
      operate = I2C_WRITE;
    } else {
      xcmd_print("param error!\r\n");
      return 0;
    }

    /* 获取读写数据的数量 */
    ret = sscanf(&argv[index][1], "%hu", &data);
    if (ret != 1) {
      xcmd_print("param error!\r\n");
      return 0;
    }

    index += 1;
    i2c_buf_size = data;

    if (operate == I2C_READ) { /* 读数据 */
      i2c_master_read(dev_addr, i2c_buf, i2c_buf_size);

      for (uint16_t i = 1; i <= i2c_buf_size; ++i) {
        xcmd_print("0x%02hhx ", i2c_buf[i - 1]);
        if (i % 16 == 0) {
          xcmd_print("\r\n");
        }
      }
      xcmd_print("\r\n");
    } else if (operate == I2C_WRITE) { /* 写数据 */
      for (uint16_t i = 0; i < i2c_buf_size; ++i) {
        ret = sscanf(argv[index], "%hx", &data);
        if (ret < 1) {
          xcmd_print("missing param!\r\n");
          return 0;
        }
        i2c_buf[i] = data;
        index += 1;
      }

      i2c_master_write(dev_addr, i2c_buf, i2c_buf_size);
    }
  }

  return 0;
}

void uart_frame_i2c_write(frame_parse_t *frame) {
  uint8_t *frame_data;
  uint16_t frame_length;
  uint16_t dev_addr;
  uint16_t reg_addr;

  if (frame->length < sizeof(dev_addr) + sizeof(reg_addr)) {
    return;
  }

  frame_data = frame->data;
  frame_length = frame->length;

  /* 获取从机地址 */
  memcpy(&dev_addr, frame_data, sizeof(dev_addr));
  frame_data += sizeof(dev_addr);
  frame_length -= sizeof(dev_addr);
  if (frame->byte_order) {
    change_byte_order(&dev_addr, sizeof(dev_addr));
  }

  /* 获取寄存器地址 */
  memcpy(&reg_addr, frame_data, sizeof(reg_addr));
  frame_data += sizeof(reg_addr);
  frame_length -= sizeof(reg_addr);
  if (!frame->byte_order) {
    change_byte_order(&reg_addr, sizeof(reg_addr));
  }

  /* 发送寄存器地址和寄存器值 */
  memcpy(i2c_buf, &reg_addr, sizeof(reg_addr));
  i2c_buf_size = sizeof(reg_addr);
  memcpy(i2c_buf + i2c_buf_size, frame_data, frame_length);
  i2c_buf_size += frame_length;
  i2c_master_write(dev_addr, i2c_buf, i2c_buf_size);
}

void uart_frame_i2c_read(frame_parse_t *frame) {
  uint8_t *frame_data;
  uint16_t frame_length;
  uint16_t dev_addr;
  uint16_t reg_addr;
  uint16_t data_len;

  if (frame->length < sizeof(dev_addr) + sizeof(reg_addr) + sizeof(data_len)) {
    return;
  }

  frame_data = frame->data;
  frame_length = frame->length;

  /* 获取从机地址 */
  memcpy(&dev_addr, frame_data, sizeof(dev_addr));
  frame_data += sizeof(dev_addr);
  frame_length -= sizeof(dev_addr);
  if (frame->byte_order) {
    change_byte_order(&dev_addr, sizeof(dev_addr));
  }

  /* 获取寄存器地址 */
  memcpy(&reg_addr, frame_data, sizeof(reg_addr));
  frame_data += sizeof(reg_addr);
  frame_length -= sizeof(reg_addr);
  if (!frame->byte_order) {
    change_byte_order(&reg_addr, sizeof(reg_addr));
  }

  /* 获取数据长度 */
  memcpy(&data_len, frame_data, sizeof(data_len));
  frame_data += sizeof(data_len);
  frame_length -= sizeof(data_len);
  if (frame->byte_order) {
    change_byte_order(&data_len, sizeof(data_len));
  }

  /* 发送寄存器地址 */
  memcpy(i2c_buf, &reg_addr, sizeof(reg_addr));
  i2c_buf_size = sizeof(reg_addr);
  i2c_master_write(dev_addr, i2c_buf, i2c_buf_size);

  /* 读取寄存器值 */
  i2c_buf_size = data_len;
  i2c_master_read(dev_addr, i2c_buf, i2c_buf_size);

  uart_puts(DEV_USART1, i2c_buf, i2c_buf_size);
}
