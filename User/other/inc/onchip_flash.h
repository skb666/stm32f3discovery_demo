#ifndef __ONCHIP_FLASH_H__
#define __ONCHIP_FLASH_H__

#include <stdint.h>

#define STMFLASH_BASE (0x08000000UL) /*!< FLASH(up to 256 KB) base address */
#define STMFLASH_END (0x08040000UL)  /*!< FLASH END address                */

#ifdef __cplusplus
extern "C" {
#endif

void STMFLASH_Write(uint32_t WriteAddr, uint64_t *pBuffer, uint32_t Num);  // 从指定地址开始写入指定长度的数据
void STMFLASH_Read(uint32_t ReadAddr, uint64_t *pBuffer, uint32_t size);   // 从指定地址开始读出指定长度的数据

#ifdef __cplusplus
}
#endif

#endif