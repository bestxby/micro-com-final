#ifndef __SD_H
#define __SD_H

#include "stm32f10x.h"

/* SD Card SPI pin mapping */
#define SD_CS_PORT       GPIOB
#define SD_CS_PIN        GPIO_Pin_4

#define SD_MOSI_PORT     GPIOB
#define SD_MOSI_PIN      GPIO_Pin_3

#define SD_MISO_PORT     GPIOA
#define SD_MISO_PIN      GPIO_Pin_11      /* PA11 (原 PA6，与 I2C SCL 冲突，将其移至闲置的 PA11) */

#define SD_SCK_PORT      GPIOB
#define SD_SCK_PIN       GPIO_Pin_5

/* SD Card R1 response flags & error codes */
typedef enum
{
  SD_RESPONSE_NO_ERROR      = (0x00),
  SD_IN_IDLE_STATE          = (0x01),
  SD_ERASE_RESET            = (0x02),
  SD_ILLEGAL_COMMAND        = (0x04),
  SD_COM_CRC_ERROR          = (0x08),
  SD_ERASE_SEQUENCE_ERROR   = (0x10),
  SD_ADDRESS_ERROR          = (0x20),
  SD_PARAMETER_ERROR        = (0x40),
  SD_RESPONSE_FAILURE       = (0xFF),

  /* Data response tokens */
  SD_DATA_OK                = (0x05),
  SD_DATA_CRC_ERROR         = (0x0B),
  SD_DATA_WRITE_ERROR       = (0x0D),
  SD_DATA_OTHER_ERROR       = (0xFF)
} SD_Error;

/* Public API */
SD_Error SD_Init(void);
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize);
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize);

#endif /* __SD_H */
