#include "onchip_flash.h"

#include "main.h"

#define FLASH_WAITETIME 50000  // FLASH等待超时时间

static inline uint32_t STMFLASH_GetPage(uint32_t Addr) {
  uint32_t page = 0;

  page = (Addr - STMFLASH_BASE) / FLASH_PAGE_SIZE;

  return page;
}

static inline uint16_t STMFLASH_ReadHalfWord(uint32_t faddr) {
  return *(__IO uint16_t *)faddr;
}

static inline uint32_t STMFLASH_ReadWord(uint32_t faddr) {
  return *(__IO uint32_t *)faddr;
}

static inline uint64_t STMFLASH_ReadDoubleWord(uint32_t faddr) {
  return *(__IO uint64_t *)faddr;
}

int8_t STMFLASH_Read(const uint32_t ReadAddr, uint16_t *pBuffer, uint32_t Num) {
  uint32_t addrx = 0;
  uint32_t addr_end = 0;

  addrx = ReadAddr;
  addr_end = ReadAddr + Num * 2;

  if (addrx < STMFLASH_BASE || addr_end >= STMFLASH_END || addrx % 2) {
    return -1;  // 非法地址
  }

  for (uint32_t i = 0; i < Num; i++) {
    pBuffer[i] = STMFLASH_ReadHalfWord(addrx);
    addrx += 2;
  }

  return 0;
}


int8_t STMFLASH_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint32_t Num) {
  FLASH_EraseInitTypeDef FlashEraseInit;
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t addrx = 0;
  uint32_t addr_end = 0;

  addrx = WriteAddr;               // 写入的起始地址
  addr_end = WriteAddr + Num * 2;  // 写入的结束地址

  if (addrx < STMFLASH_BASE || addr_end >= STMFLASH_END || addrx % 2) {
    return -1;  // 非法地址
  }

  HAL_FLASH_Unlock();  // 解锁

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

  while (addrx < addr_end) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addrx, *pBuffer);
    if (status != HAL_OK) {
      break;
    }
    addrx += 2;
    pBuffer++;
  }

  HAL_FLASH_Lock();  // 上锁

  if (status != HAL_OK) {
    return -2;
  }

  return 0;
}

int8_t STMFLASH_Erase(uint32_t addr_start, uint32_t addr_end, uint32_t retry) {
  uint32_t PageError = addr_start;
  FLASH_EraseInitTypeDef FlashEraseInit;
  HAL_StatusTypeDef status = HAL_OK;
  uint64_t temp;

  if (addr_start < STMFLASH_BASE || addr_end >= STMFLASH_END || addr_start > addr_end) {
    return -1;  // 非法地址
  }

  while (addr_start < addr_end) {
    if (STMFLASH_ReadDoubleWord(addr_start) != 0xFFFFFFFFFFFFFFFF) {
      status = HAL_ERROR;
      break;
    }
  }

  if (status == HAL_OK) {
    return 0;
  }

  HAL_FLASH_Unlock();  // 解锁

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

  do {
    FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;                                       // 擦除类型，页擦除
    FlashEraseInit.PageAddress = STMFLASH_GetPage(PageError);                               // 要擦除的页
    FlashEraseInit.NbPages = STMFLASH_GetPage(addr_end) - STMFLASH_GetPage(PageError) + 1;  // 擦除页数

    status = HAL_FLASHEx_Erase(&FlashEraseInit, &PageError);
  } while ((status != HAL_OK) && (retry--));

  HAL_FLASH_Lock();  // 上锁

  if (status != HAL_OK) {
    return -2;
  }

  return 0;
}
