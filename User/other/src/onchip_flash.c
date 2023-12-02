#include "onchip_flash.h"

#include "main.h"

#define FLASH_WAITETIME 50000  // FLASH等待超时时间

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static inline uint32_t STMFLASH_GetPage(uint32_t Addr) {
  uint32_t page = 0;

  page = (Addr - STMFLASH_BASE) / FLASH_PAGE_SIZE;

  return page;
}

static inline uint64_t STMFLASH_ReadDoubleWord(uint32_t faddr) {
  return *(__IO uint64_t *)faddr;
}

void STMFLASH_Write(uint32_t WriteAddr, uint64_t *pBuffer, uint32_t Num) {
  FLASH_EraseInitTypeDef FlashEraseInit;
  HAL_StatusTypeDef FlashStatus = HAL_OK;
  uint32_t PageError = 0;
  uint32_t addrx = 0;
  uint32_t endaddr = 0;

  if (WriteAddr < STMFLASH_BASE || WriteAddr % 8) {
    return;  // 非法地址
  }

  HAL_FLASH_Unlock();             // 解锁
  addrx = WriteAddr;              // 写入的起始地址
  endaddr = WriteAddr + Num * 8;  // 写入的结束地址
  endaddr = WriteAddr + Num * 8;  // 写入的结束地址

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

  if (addrx < STMFLASH_END) {
    while (addrx < endaddr) {
      if (STMFLASH_ReadDoubleWord(addrx) != 0XFFFFFFFFFFFFFFFF) {
        FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;  // 擦除类型，页擦除
        FlashEraseInit.PageAddress = addrx;     // 要擦除的页
        FlashEraseInit.NbPages = 1;                        // 一次只擦除一页
        if (HAL_FLASHEx_Erase(&FlashEraseInit, &PageError) != HAL_OK) {
          break;  // 发生错误了
        }
      } else {
        addrx += 8;
      }
      FLASH_WaitForLastOperation(FLASH_WAITETIME);  // 等待上次操作完成
    }
  }
  FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME);  // 等待上次操作完成
  if (FlashStatus == HAL_OK) {
    while (WriteAddr < endaddr) {                                                            // 写数据
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, WriteAddr, *pBuffer) != HAL_OK) {  // 写入数据
        break;                                                                               // 写入异常
      }
      WriteAddr += 8;
      pBuffer++;
    }
  }
  HAL_FLASH_Lock();  // 上锁
}

void STMFLASH_Read(uint32_t ReadAddr, uint64_t *pBuffer, uint32_t size) {
  uint32_t i;
  for (i = 0; i < size; i++) {
    pBuffer[i] = STMFLASH_ReadDoubleWord(ReadAddr);  // 读取8个字节.
    ReadAddr += 8;                                   // 偏移8个字节.
  }
}
