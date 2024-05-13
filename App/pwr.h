/**
  ******************************************************************************
  * @file    PWR/PWR_CurrentConsumption/stm32f4xx_lp_modes.h
  * @author  MCD Application Team
  * @brief   Header for stm32f4xx_lp_modes.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_LP_MODES_H
#define __STM32F4xx_LP_MODES_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SLEEP 0x00000040
#define RICOCHET 0x00000080

#if !defined (SLEEP_MODE) && !defined (STOP_MODE) && !defined (STOP_UNDERDRIVE_MODE) && !defined (STANDBY_MODE)\
 && !defined (STANDBY_RTC_MODE) && !defined (STANDBY_RTC_BKPSRAM_MODE)
/* Uncomment the corresponding line to select the STM32F4xx Low Power mode */
#define SLEEP_MODE
#endif

#if !defined (SLEEP_MODE) && !defined (STOP_MODE) && !defined (STOP_UNDERDRIVE_MODE) && !defined (STANDBY_MODE)\
 && !defined (STANDBY_RTC_MODE) && !defined (STANDBY_RTC_BKPSRAM_MODE)
 #error "Please select first the target STM32F4xx Low Power mode to be measured (in stm32f4xx_lp_modes.h file)"
#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SleepMode_Measure(void);
int Init_Th_Sleep(void);

#endif
