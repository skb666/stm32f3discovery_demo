/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <stdarg.h>
#include "ring_fifo.h"
#include "common.h"
#include "param.h"
#include "uart_device.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
_CCM_DATA static char tx_buffer[APP_TX_DATA_SIZE];
ring_define_static(_CCM_DATA uint8_t, usb_rx_buffer, APP_RX_DATA_SIZE, 1);
ring_define_static(_CCM_DATA uint8_t, usb_tx_buffer, APP_TX_DATA_SIZE, 0);
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
typedef struct {
  uint32_t dwDTERate;
  uint8_t bCharFormat;
  uint8_t bParityType;
  uint8_t bDataBits;
} UART_CONFIG;
/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  SYS_PARAM *sys = sys_param_get();
  UART_CONFIG *uart_conf = NULL;

  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING: {
      uart_conf = (UART_CONFIG *)pbuf;
      switch (sys->status) {
        case STATUS_USB_AS_USART1: {
          LL_USART_SetBaudRate(USART1, LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE), LL_USART_OVERSAMPLING_16, uart_conf->dwDTERate);
          switch (uart_conf->bCharFormat) {
            default: {
              LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
            } break;
            case 1: {
              LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1_5);
            } break;
            case 2: {
              LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_2);
            } break;
          }
          switch (uart_conf->bParityType) {
            default: {
              LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
            } break;
            case 1: {
              LL_USART_SetParity(USART1, LL_USART_PARITY_ODD);
            } break;
            case 2: {
              LL_USART_SetParity(USART1, LL_USART_PARITY_EVEN);
            } break;
          }
          switch (uart_conf->bDataBits) {
            default: {
              LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
            } break;
#if defined(USART_7BITS_SUPPORT)
            case 7: {
              LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_7B);
            } break;
#endif
            case 9: {
              LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_9B);
            } break;
          }
        } break;
        case STATUS_USB_AS_USART3: {
          LL_USART_SetBaudRate(USART3, LL_RCC_GetUSARTClockFreq(LL_RCC_USART3_CLKSOURCE), LL_USART_OVERSAMPLING_16, uart_conf->dwDTERate);
          switch (uart_conf->bCharFormat) {
            default: {
              LL_USART_SetStopBitsLength(USART3, LL_USART_STOPBITS_1);
            } break;
            case 1: {
              LL_USART_SetStopBitsLength(USART3, LL_USART_STOPBITS_1_5);
            } break;
            case 2: {
              LL_USART_SetStopBitsLength(USART3, LL_USART_STOPBITS_2);
            } break;
          }
          switch (uart_conf->bParityType) {
            default: {
              LL_USART_SetParity(USART3, LL_USART_PARITY_NONE);
            } break;
            case 1: {
              LL_USART_SetParity(USART3, LL_USART_PARITY_ODD);
            } break;
            case 2: {
              LL_USART_SetParity(USART3, LL_USART_PARITY_EVEN);
            } break;
          }
          switch (uart_conf->bDataBits) {
            default: {
              LL_USART_SetDataWidth(USART3, LL_USART_DATAWIDTH_8B);
            } break;
#if defined(USART_7BITS_SUPPORT)
            case 7: {
              LL_USART_SetDataWidth(USART3, LL_USART_DATAWIDTH_7B);
            } break;
#endif
            case 9: {
              LL_USART_SetDataWidth(USART3, LL_USART_DATAWIDTH_9B);
            } break;
          }
        } break;
        default: {
        } break;
      }
    } break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  SYS_PARAM *sys = sys_param_get();

  switch (sys->status) {
    case STATUS_SHELL: {
      disable_global_irq();
      ring_push_mult(&usb_rx_buffer, Buf, *Len);
      enable_global_irq();
    } break;
    case STATUS_USB_AS_USART1: {
      uart_puts(DEV_USART1, Buf, *Len);
    } break;
    case STATUS_USB_AS_USART3: {
      uart_puts(DEV_USART3, Buf, *Len);
    } break;
    default: {
    } break;
  }

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
int8_t usb_tx_trans(void)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  NUM_TYPE length = 0;

  if (hcdc->TxState != 0){
    return -1;
  }

  if (!ring_is_empty(&usb_tx_buffer))
  {
    length = ring_size(&usb_tx_buffer);

    disable_global_irq();
    ring_pop_mult(&usb_tx_buffer, UserTxBufferFS, length);
    enable_global_irq();

    return CDC_Transmit_FS(UserTxBufferFS, length);
  }

  return 0;
}

void usb_printf(const char *format, ...)
{
  va_list args;
  uint32_t length;
  uint16_t success = 0;
  uint8_t *pbuf;

  va_start(args, format);
  length = vsnprintf(tx_buffer, APP_TX_DATA_SIZE, (char *)format, args);
  va_end(args);

  pbuf = (uint8_t *)tx_buffer;
  
  do {
    disable_global_irq();
    success = ring_push_mult(&usb_tx_buffer, pbuf, length);
    enable_global_irq();

    if (success == length) {
      return;
    }

    if (usb_tx_trans()) {
      // 防死机，可能会丢数据
      return;
    }

    pbuf += success;
    length -= success;
  } while (length);
}

void usb_puts(uint8_t* buf, uint16_t len)
{
  uint16_t success = 0;
  uint8_t *pbuf;

  pbuf = buf;

  do {
    disable_global_irq();
    success = ring_push_mult(&usb_tx_buffer, pbuf, len);
    enable_global_irq();

    if (success == len) {
      return;
    }

    if (usb_tx_trans()) {
      // 防死机，可能会丢数据
      return;
    }

    pbuf += success;
    len -= success;
  } while (len);
}

void usb_putchar(const char ch)
{

  tx_buffer[0] = ch;

  if (ring_is_full(&usb_tx_buffer)) {
    (void)usb_tx_trans();
  }
  
  disable_global_irq();
  ring_push(&usb_tx_buffer, tx_buffer);
  enable_global_irq();
}

int8_t usb_rx_get(uint8_t *ch)
{
  int8_t res = 0;

  if (ring_is_empty(&usb_rx_buffer))
  {
    return -1;
  }

  disable_global_irq();
  res = ring_pop(&usb_rx_buffer, ch);
  enable_global_irq();

  return res;
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
