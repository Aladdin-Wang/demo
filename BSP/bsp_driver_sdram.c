/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    bsp_driver_sdram.c (based on stm32469i_eval_sdram.c)
  * @brief   This file includes a generic SDRAM driver.
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
/* USER CODE END Header */
  

#ifdef OLD_CODE
/* Kept to avoid issue when migrating old projects (as some user sections were renamed/changed). */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
#else

/* Includes ------------------------------------------------------------------*/
#include "bsp_driver_sdram.h"

/* Extern variables ----------------------------------------------------------*/
extern SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN Init */
/**
  * @brief  Initializes the SDRAM device 
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_Init(void)
{
  uint8_t sdramstatus = SDRAM_OK;
  
  /* place for custom code */
  
  return sdramstatus;
}
/* USER CODE END Init */

/* USER CODE BEGIN BeforeReadSection */
/* can be used to modify / undefine following code or add code */
/* USER CODE END BeforeReadSection */

/**
  * @brief  Reads an mount of data from the SDRAM memory in polling mode. 
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status : SDRAM_OK or SDRAM_ERROR.
  */
uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{ 
  uint8_t sdramstatus = SDRAM_OK;
  
  if(HAL_SDRAM_Read_32b(&hsdram1, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }

  return sdramstatus;
}

/**
  * @brief  Reads an mount of data from the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status : SDRAM_OK or SDRAM_ERROR.
  */
uint8_t BSP_SDRAM_ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{ 
  uint8_t sdramstatus = SDRAM_OK;
  
  if(HAL_SDRAM_Read_DMA(&hsdram1, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }
   
  return sdramstatus;
}

/* USER CODE BEGIN BeforeWriteSection */
/* can be used to modify / undefine following code or add code */
/* USER CODE END BeforeWriteSection */

/**
  * @brief  Writes an mount of data to the SDRAM memory in polling mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status : SDRAM_OK or SDRAM_ERROR.
  */
uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{ 
  uint8_t sdramstatus = SDRAM_OK;
  
  if(HAL_SDRAM_Write_32b(&hsdram1, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }

  return sdramstatus;
}

/**
  * @brief  Writes an mount of data to the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status : SDRAM_OK or SDRAM_ERROR.
  */
uint8_t BSP_SDRAM_WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{ 
  uint8_t sdramstatus = SDRAM_OK;
  
  if(HAL_SDRAM_Write_DMA(&hsdram1, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }

  return sdramstatus;
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  SdramCmd: Pointer to SDRAM command structure
  * @retval SDRAM status : SDRAM_OK or SDRAM_ERROR.
  */
uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd)
{ 
  uint8_t sdramstatus = SDRAM_OK;
  
  if(HAL_SDRAM_SendCommand(&hsdram1, SdramCmd, SDRAM_TIMEOUT) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }

  return sdramstatus;
}

/**
  * @brief  Handles SDRAM DMA transfer interrupt request.
  */
void BSP_SDRAM_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hsdram1.hdma); 
}

/* USER CODE BEGIN AdditionalCode */
/* user code can be inserted here */
/**
  * @brief  ��SDRAMоƬ���г�ʼ������
  * @param  None. 
  * @retval None.
  */
void SDRAM_InitSequence(void)
{
  FMC_SDRAM_CommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;
  
/* Step 3 --------------------------------------------------------------------*/
  /* ������������ṩ��SDRAM��ʱ�� */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 0;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;

  /* ������������*/
  BSP_SDRAM_Sendcmd(&FMC_SDRAMCommandStructure);  
  
/* Step 4 --------------------------------------------------------------------*/
  /* ��ʱ */
  HAL_Delay(10);
    
/* Step 5 --------------------------------------------------------------------*/
  /* ������������е�bankԤ��� */ 
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_PALL;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 0;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;

   /* ������������*/
  BSP_SDRAM_Sendcmd(&FMC_SDRAMCommandStructure);
  
/* Step 6 --------------------------------------------------------------------*/
  /* ��������Զ�ˢ�� */   
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 2;			//2���Զ�ˢ������
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;

   /* �����Զ�ˢ������*/
  BSP_SDRAM_Sendcmd(&FMC_SDRAMCommandStructure);
  
 
 /* Step 7 --------------------------------------------------------------------*/
  /* ����sdram�Ĵ������� */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_4          |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                   SDRAM_MODEREG_CAS_LATENCY_2           |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  
  /* �����������SDRAM�Ĵ��� */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = tmpr;

   /* ������������*/
  BSP_SDRAM_Sendcmd(&FMC_SDRAMCommandStructure);
  
/* Step 8 --------------------------------------------------------------------*/

  /* ����ˢ�¼����� */
	/*ˢ������ = (COUNT + 1) x SDRAM Ƶ��ʱ��
		COUNT =�� SDRAM ˢ������/����) - 20*/
  /* 64ms/4096=15.62us  (15.62 us x FSDCLK) - 20 =1386 */
  HAL_SDRAM_ProgramRefreshRate(&hsdram1,1386);

}
/* USER CODE END AdditionalCode */

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
