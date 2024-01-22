#include "update.h"

#include <stdio.h>

#include "common.h"
#include "crc.h"
#include "device.h"
#include "main.h"
#include "param.h"
#include "ring_fifo.h"

#define UPDATE_PACKAGE_MAX_SIZE 1024

typedef enum {
  UPDATE_WAITTING = 0,
  UPDATE_BEGIN,
  UPDATE_TRANSMIT,
} UPDATE_STAGE;

typedef enum {
  ERRNO_SUCC = 0,       // 无错误
  ERRNO_PROC = 7,       // 处理中
  ERRNO_CHECK_CRC = 6,  // 校验错误
  ERRNO_DATA_SIZE = 5,  // 数据长度错误
  ERRNO_PKG_NUM = 4,    // 包号错误
  ERRNO_FLASH_WR = 3,   // Flash 写入错误
  ERRNO_RES = 2,        // 预留错误码
  ERRNO_UNKNOW = 1,     // 未知错误
} UPDATE_ERRNO;

typedef struct {
  uint16_t pkg_num : 12;
  uint16_t running : 1;
  uint16_t errno : 3;
} UPDATE_STATUS;

typedef enum {
  PKG_TYPE_INIT = 0x1000,
  PKG_TYPE_HEAD = 0x0001,
  PKG_TYPE_DATA = 0x0002,
} PKG_TYPE;

typedef struct {
  uint32_t file_crc;        // 升级文件的 CRC 值
  uint32_t file_size_real;  // 升级文件实际大小
  uint16_t data_size_one;   // 单次传输的升级数据大小
  uint16_t pkg_num_total;   // 升级包数量
} PKG_HEAD;

typedef struct {
  uint32_t pkg_crc;   // 包 CRC 值，该字段以下内容
  uint16_t pkg_num;   // 当前包号
  uint16_t data_len;  // 携带的数据长度
  uint8_t data[];     // 升级数据
} PKG_DATA;

typedef struct {
  uint16_t type;
  union {
    PKG_HEAD *head;
    PKG_DATA *data;
  };
} UPDATE_PKG;

typedef enum {
  STATUS_NONE = 0,  // APP 不存在
  STATUS_RECV,      // APP 接收中
  STATUS_LOAD,      // APP 载入中
  STATUS_BOOT,      // APP 可引导
  STATUS_NORM,      // APP 可运行
} APP_STATUS;

typedef struct {
  uint32_t update_needed;  // 更新标记
  uint32_t app_status;     // APP 的状态
  uint32_t back_to_app;    // 是否允许回到 APP
  uint32_t crc_val;        // 引导参数的 CRC 校验值
} BOOT_PARAM;

typedef void (*pFunction)(void);
__IO uint32_t MspAddress;
__IO uint32_t JumpAddress;
pFunction JumpToApplication;

ring_define_static(uint8_t, update_data_buf, (sizeof(PKG_DATA) + UPDATE_PACKAGE_MAX_SIZE), 0);

static PKG_HEAD s_pkg_head;
static PKG_DATA s_pkg_data;

static BOOT_PARAM boot_param_default = {
    .update_needed = 0,
    .app_status = STATUS_BOOT,
    .back_to_app = 0,
};
const uint32_t boot_param_size16 = sizeof(BOOT_PARAM) / 2 + sizeof(BOOT_PARAM) % 2;
const uint32_t boot_param_crcdatalen = sizeof(boot_param_default) - sizeof(boot_param_default.crc_val);

static inline void crc_reset(void) {
  /* Change CRC peripheral state */
  (&hcrc)->State = HAL_CRC_STATE_BUSY;

  /* Reset CRC Calculation Unit (hcrc->Instance->INIT is
   *  written in hcrc->Instance->DR) */
  __HAL_CRC_DR_RESET(&hcrc);

  /* Change CRC peripheral state */
  (&hcrc)->State = HAL_CRC_STATE_READY;
}

static uint32_t param_crc_calc(const BOOT_PARAM *param) {
  uint32_t crc = 0;

  crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)param, boot_param_crcdatalen);

  return crc;
}

static int8_t boot_param_save(uint32_t addr, BOOT_PARAM *param) {
  int8_t err = 0;

  param->crc_val = param_crc_calc(param);

  disable_global_irq();
  err = STMFLASH_Erase(addr, PART_SIZE_PARAM, 1);
  enable_global_irq();
  if (err) {
    printf_dbg("ERROR: Flash Erase\r\n");
    return -1;
  }

  disable_global_irq();
  err = STMFLASH_Write(addr, (uint16_t *)param, boot_param_size16);
  enable_global_irq();
  if (err) {
    printf_dbg("ERROR: Flash Write\r\n");
    return -2;
  }

  return 0;
}

static int8_t boot_param_update(BOOT_PARAM *param) {
  int8_t err = 0;

  err = boot_param_save(ADDR_BASE_PARAM, param);
  if (err) {
    return err;
  }

  err = boot_param_save(ADDR_BASE_PARAM_BAK, param);
  if (err) {
    return err;
  }

  return 0;
}

static void boot_param_get(BOOT_PARAM *pdata) {
  (void)STMFLASH_Read(ADDR_BASE_PARAM, (uint16_t *)pdata, boot_param_size16);
}

static int8_t boot_param_get_with_check(BOOT_PARAM *pdata) {
  BOOT_PARAM param, param_bak;

  (void)STMFLASH_Read(ADDR_BASE_PARAM, (uint16_t *)&param, boot_param_size16);
  (void)STMFLASH_Read(ADDR_BASE_PARAM_BAK, (uint16_t *)&param_bak, boot_param_size16);

  if (param_crc_calc(&param) == param.crc_val) {
    printf_dbg("boot param checked Ok\r\n");
    if (param_crc_calc(&param_bak) == param_bak.crc_val) {
      printf_dbg("boot param backup checked Ok\r\n");
      if (memcmp(&param, &param_bak, sizeof(BOOT_PARAM)) != 0) {
        printf_dbg("boot param main sector and backup sector data are different, update bakup sector data\r\n");
        if (boot_param_save(ADDR_BASE_PARAM_BAK, &param)) {
          return -1;
        }
      } else {
        printf_dbg("boot param main sector and backup sector data are the same\r\n");
      }
    } else {
      printf_dbg("boot param backup checked Fail, update backup sector data\r\n");
      if (boot_param_save(ADDR_BASE_PARAM_BAK, &param)) {
        return -1;
      }
    }
    memcpy(pdata, &param, sizeof(BOOT_PARAM));
  } else {
    printf_dbg("boot param checked Fail\r\n");
    if (param_crc_calc(&param_bak) == param_bak.crc_val) {
      printf_dbg("boot param backup checked Ok\r\n");
      printf_dbg("update main sector data\r\n");
      if (boot_param_save(ADDR_BASE_PARAM, &param_bak)) {
        return -1;
      }
      memcpy(pdata, &param_bak, sizeof(BOOT_PARAM));
    } else {
      printf_dbg("boot param backup checked Fail\r\n");
      printf_dbg("restore defaults\r\n");
      if (boot_param_update(&boot_param_default)) {
        return -1;
      }
      memcpy(pdata, &boot_param_default, sizeof(BOOT_PARAM));
    }
  }

  return 0;
}

static int8_t load_app_from_backup(void) {
  uint16_t buf[512];
  int8_t err = 0;
  uint32_t addr_write = ADDR_BASE_APP;
  uint32_t addr_read = ADDR_BASE_APP_BAK;

  disable_global_irq();
  err = STMFLASH_Erase(ADDR_BASE_APP, PART_SIZE_APP, 1);
  enable_global_irq();
  if (err) {
    return err;
  }

  while (addr_write < ADDR_BASE_APP_BAK) {
    (void)STMFLASH_Read(addr_read, buf, sizeof(buf) >> 1);
    disable_global_irq();
    err = STMFLASH_Write(addr_write, buf, sizeof(buf) >> 1);
    enable_global_irq();
    if (!err) {
      addr_read += sizeof(buf);
      addr_write += sizeof(buf);
    }
  }

  return 0;
}

inline __attribute__((always_inline)) void boot_to_app(uint32_t boot_addr) {
  MspAddress = STMFLASH_ReadWord(boot_addr);
  JumpAddress = STMFLASH_ReadWord(boot_addr + 4);
  JumpToApplication = (pFunction)JumpAddress;

  if ((MspAddress & 0xFFFFC000) != 0x10000000 && (MspAddress & 0xFFFF0000) != 0x20000000) {
    return;
  }

  LL_SYSTICK_DisableIT();
  LL_USART_Disable(USART1);
  LL_USART_DisableIT_IDLE(USART1);
  LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_5);
  LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_5);
  LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_4);
  LL_USART_DeInit(USART1);
  LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_5);
  LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_4);
  HAL_CRC_DeInit(&hcrc);
  HAL_DeInit();

  SCB->VTOR = boot_addr;

  __set_CONTROL(0);
  __set_PSP(MspAddress);
  __set_MSP(MspAddress);
  JumpToApplication();
}

void boot_param_check(void) {
  BOOT_PARAM param;
  uint32_t msp_addr;

  if (boot_param_get_with_check(&param)) {
    Error_Handler();
  }

  if (SCB->VTOR == ADDR_BASE_APP) {
    param.update_needed = 0;
    param.app_status = STATUS_NORM;
    param.back_to_app = 1;
    if (boot_param_update(&param)) {
      Error_Handler();
    }
    printf_dbg("running in app\r\n");
    return;
  } else {
    printf_dbg("running in bld\r\n");
  }

  if (!param.update_needed) {
    switch (param.app_status) {
      /* 默认状态，尝试引导 */
      case STATUS_BOOT: {
        param.update_needed = 1;
        param.back_to_app = 0;
        if (boot_param_update(&param)) {
          Error_Handler();
        }
      } break;
      /* 正常状态，正常引导 */
      case STATUS_NORM: {
      } break;
      /* 异常状态，进入升级 */
      case STATUS_NONE:
      case STATUS_RECV:
      case STATUS_LOAD:
      default: {
        param.update_needed = 1;
        param.app_status = STATUS_NONE;
        param.back_to_app = 0;
        if (boot_param_update(&param)) {
          Error_Handler();
        }
        return;
      } break;
    }
  } else {
    switch (param.app_status) {
      /* 升级过程中断电 */
      case STATUS_RECV: {
        if (param.back_to_app) {
          /* 引导升级前的 APP */
          param.app_status = STATUS_BOOT;
          if (boot_param_update(&param)) {
            Error_Handler();
          }
        } else {
          /* 进入升级 */
          param.app_status = STATUS_NONE;
          if (boot_param_update(&param)) {
            Error_Handler();
          }
          return;
        }
      } break;
      /* 加载程序时断电，程序加载后引导 */
      case STATUS_LOAD: {
        if (load_app_from_backup()) {
          Error_Handler();
        }
        param.app_status = STATUS_BOOT;
        param.back_to_app = 0;
        if (boot_param_update(&param)) {
          Error_Handler();
        }
      } break;
      /* 正常状态，进入升级 */
      case STATUS_NORM: {
        return;
      } break;
      /* 异常状态，进入升级 */
      case STATUS_BOOT:
      case STATUS_NONE:
      default: {
        param.app_status = STATUS_NONE;
        param.back_to_app = 0;
        if (boot_param_update(&param)) {
          Error_Handler();
        }
        return;
      } break;
    }
  }

  /* 引导到 APP */
  boot_to_app(ADDR_BASE_APP);
  printf_dbg("ERROR: boot to app\r\n");

  /* 引导失败，进入升级 */
  param.update_needed = 1;
  param.app_status = STATUS_NONE;
  param.back_to_app = 0;
  if (boot_param_update(&param)) {
    Error_Handler();
  }
}

void boot_test(void) {
#ifdef DEBUG
  crc_reset();
  boot_param_default.crc_val = param_crc_calc(&boot_param_default);
  printf_dbg("0x%08x\r\n", boot_param_default.crc_val);
#endif

  boot_param_check();
}

int shell_update_cmd(int argc, char *argv[]) {
  BOOT_PARAM param;

  boot_param_get(&param);

  param.update_needed = 1;
  if (boot_param_update(&param)) {
    Error_Handler();
  }

  NVIC_SystemReset();

  return 0;
}
