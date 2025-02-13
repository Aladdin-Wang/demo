/**
 ******************************************************************************
  * @file    bsp_driver_sdram.h (based on stm32469i_eval_sdram.h)
  * @brief   This file contains the common defines and functions prototypes for  
  *          the bsp_driver_sdram.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4_SDRAM_H
#define __STM32F4_SDRAM_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sdram.h"

/* USER CODE BEGIN 0 */

/** 
  * @brief  SDRAM status structure definition  
  */     
#define   SDRAM_OK         ((uint8_t)0x00)
#define   SDRAM_ERROR      ((uint8_t)0x01)

/* Exported constants --------------------------------------------------------*/ 

#define SDRAM_DEVICE_ADDR  ((uint32_t)0xD0000000)
#define SDRAM_DEVICE_SIZE  ((uint32_t)0x800000)  /* SDRAM device size in MBytes */

 #define SDRAM_MEMORY_WIDTH FMC_SDRAM_MEM_BUS_WIDTH_32 */
 #define SDCLOCK_PERIOD     FMC_SDRAM_CLOCK_PERIOD_2 */

#define REFRESH_COUNT       ((uint32_t)0x0569)       /* SDRAM refresh counter (90 MHz SD clock) */
#define  SDRAM_TIMEOUT      ((uint32_t)0xFFFF)

/* DMA definitions for SDRAM DMA transfer */

#define __DMAx_CLK_ENABLE     __HAL_RCC_DMA2_CLK_ENABLE
#define __DMAx_CLK_DISABLE    __HAL_RCC_DMA2_CLK_DISABLE
#define SDRAM_DMAx_CHANNEL    DMA_CHANNEL_0
#define SDRAM_DMAx_STREAM     DMA2_Stream0
#define SDRAM_DMAx_IRQn       DMA2_Stream0_IRQn
#define SDRAM_DMAx_IRQHandler DMA2_Stream0_IRQHandler

   
/**
  * @brief  FMC SDRAM Mode definition register defines
  */

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)


/* Exported functions --------------------------------------------------------*/   
uint8_t BSP_SDRAM_Init(void);
uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd);
void    BSP_SDRAM_DMA_IRQHandler(void);
void SDRAM_InitSequence(void);
/* USER CODE END 0 */
   
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4_SDRAM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
