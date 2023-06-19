/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */

static int8_t USB_CDC_IF_Init(USBD_HandleTypeDef *pdev);
static int8_t USB_CDC_IF_DeInit(void);
static int8_t USB_CDC_IF_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t USB_CDC_IF_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t USB_CDC_IF_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

USBD_CDC_ItfTypeDef USBD_CDC_IF_fops =
{
  USB_CDC_IF_Init,
  USB_CDC_IF_DeInit,
  USB_CDC_IF_Control,
  USB_CDC_IF_Receive,
  USB_CDC_IF_TransmitCplt
};

static USBD_CDC_LineCodingTypeDef linecoding =
{
  115200, /* baud rate*/
  0x00,   /* stop bits-1*/
  0x00,   /* parity - none*/
  0x08    /* nb. of bits 8*/
};

struct USBD_CDC_CircularBuffer {
  uint16_t write_index;
  uint16_t read_index;
  uint8_t buffer[4096];
  uint8_t usb_buffer[CDC_DATA_HS_MAX_PACKET_SIZE];
  uint8_t usb_busy;
};

static struct USBD_CDC_CircularBuffer rxBuffer;
static struct USBD_CDC_CircularBuffer txBuffer;
static USBD_HandleTypeDef *usb_pdev;

/* Private functions ---------------------------------------------------------*/

static void USB_CDC_IF_BUFFER_write_char(struct USBD_CDC_CircularBuffer* buffer, uint8_t c)
{
  uint16_t next_write_index = (buffer->write_index + 1) % sizeof(buffer->buffer);
  if(next_write_index != buffer->read_index) {
    // Buffer not full
    buffer->buffer[buffer->write_index] = c;
    buffer->write_index = next_write_index;
  }
}

static uint8_t USB_CDC_IF_BUFFER_read_char(struct USBD_CDC_CircularBuffer* buffer, uint8_t* c)
{
  if(buffer->read_index == buffer->write_index) {
    // Buffer empty
    return 0;
  }


  *c = buffer->buffer[buffer->read_index];
  buffer->read_index = (buffer->read_index + 1) % sizeof(buffer->buffer);

  return 1;
}

static void USB_CDC_IF_sendPending() {
  if(txBuffer.usb_busy || !usb_pdev) {
    return;
  }

  uint16_t start = txBuffer.read_index;
  uint16_t end = txBuffer.write_index;
  uint16_t size = (end - start + sizeof(rxBuffer.buffer)) % sizeof(rxBuffer.buffer);

  if(size > sizeof(txBuffer.usb_buffer)) {
    size = sizeof(txBuffer.usb_buffer);
    end = (txBuffer.read_index + size) % sizeof(rxBuffer.buffer);
  }

  uint16_t total_size = 0;

  if(end < start) {
    // Copy between start and end of buffer
    uint16_t first_chunk_size = sizeof(txBuffer.buffer) - start;
    memcpy(txBuffer.usb_buffer, &txBuffer.buffer[start], first_chunk_size);

    // then between begin of buffer and end
    memcpy(&txBuffer.usb_buffer[first_chunk_size], &txBuffer.buffer[0], end);

    total_size = first_chunk_size + end;
  } else {
    // Copy from start to end
    memcpy(txBuffer.usb_buffer, &txBuffer.buffer[start], end - start);
    total_size = end - start;
  }

  txBuffer.read_index = end;

  if(total_size > 0) {
    txBuffer.usb_busy = 1;
    USBD_CDC_SetTxBuffer(usb_pdev, txBuffer.usb_buffer, total_size);
    USBD_CDC_TransmitPacket(usb_pdev);
  }
}

/**
  * @brief  USB_CDC_IF_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USB_CDC_IF_Init(USBD_HandleTypeDef *pdev)
{
  /*
     Add your initialization code here
  */
  usb_pdev = pdev;

  USBD_CDC_SetRxBuffer(pdev, rxBuffer.usb_buffer);
  USB_CDC_IF_sendPending();

  return (0);
}

/**
  * @brief  USB_CDC_IF_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USB_CDC_IF_DeInit(void)
{
  /*
     Add your deinitialization code here
  */
  return (0);
}


/**
  * @brief  USB_CDC_IF_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USB_CDC_IF_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  UNUSED(length);

  switch (cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      /* Add your code here */
      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
      /* Add your code here */
      break;

    case CDC_SET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_GET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_CLEAR_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_SET_LINE_CODING:
      linecoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | \
                                         (pbuf[2] << 16) | (pbuf[3] << 24));
      linecoding.format     = pbuf[4];
      linecoding.paritytype = pbuf[5];
      linecoding.datatype   = pbuf[6];

      /* Add your code here */
      break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(linecoding.bitrate);
      pbuf[1] = (uint8_t)(linecoding.bitrate >> 8);
      pbuf[2] = (uint8_t)(linecoding.bitrate >> 16);
      pbuf[3] = (uint8_t)(linecoding.bitrate >> 24);
      pbuf[4] = linecoding.format;
      pbuf[5] = linecoding.paritytype;
      pbuf[6] = linecoding.datatype;

      /* Add your code here */
      break;

    case CDC_SET_CONTROL_LINE_STATE:
      /* Add your code here */
      break;

    case CDC_SEND_BREAK:
      /* Add your code here */
      break;

    default:
      break;
  }

  return (0);
}

/**
  * @brief  USB_CDC_IF_Receive
  *         Data received over USB OUT endpoint are sent over CDC interface
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
static int8_t USB_CDC_IF_Receive(uint8_t *Buf, uint32_t *Len)
{
  uint16_t start = rxBuffer.write_index;
  uint16_t size = *Len;
  uint16_t max_size = (rxBuffer.read_index - rxBuffer.write_index - 1 + sizeof(rxBuffer.buffer)) % sizeof(rxBuffer.buffer);

  if(size > max_size)
    size = max_size;

  uint16_t end = (start + size) % sizeof(rxBuffer.buffer);


  if(end < start) {
    // Copy between start and end of buffer
    uint16_t first_chunk_size = sizeof(rxBuffer.buffer) - start;
    memcpy(&rxBuffer.buffer[start], rxBuffer.usb_buffer, first_chunk_size);

    // then between begin of buffer and end
    memcpy(&rxBuffer.buffer[0], &rxBuffer.usb_buffer[first_chunk_size], end);
  } else {
    // Copy from start to end
    memcpy(&rxBuffer.buffer[start], rxBuffer.usb_buffer, end - start);
  }

  rxBuffer.write_index = end;

  USBD_CDC_ReceivePacket(usb_pdev);

  return (0);
}

/**
  * @brief  USB_CDC_IF_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t USB_CDC_IF_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);

  txBuffer.usb_busy = 0;

  USB_CDC_IF_sendPending();

  return (0);
}


void USB_CDC_IF_TX_write(const uint8_t *Buf, uint32_t Len)
{
  size_t i;
  for(i = 0; i < Len; i++) {
    USB_CDC_IF_BUFFER_write_char(&txBuffer, Buf[i]);
  }

  USB_CDC_IF_sendPending();
}

uint32_t USB_CDC_IF_RX_read(uint8_t *Buf, uint32_t max_len)
{
  size_t i = 0;
  while(i < max_len && USB_CDC_IF_BUFFER_read_char(&rxBuffer, &Buf[i])) {
    i++;
  }
  return i;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

