/**
  ******************************************************************************
  * @file    PWR/PWR_CurrentConsumption/stm32f4xx_lp_modes.c 
  * @author  MCD Application Team
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the STM32F4xx Low Power Modes:
  *           - Sleep Mode
  *           - STOP mode with RTC
  *           - Under-Drive STOP mode with RTC
  *           - STANDBY mode without RTC and BKPSRAM
  *           - STANDBY mode with RTC
  *           - STANDBY mode with RTC and BKPSRAM
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Board_LED.h"
#include "pwr.h"
#include "lcd.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_CurrentConsumption
  * @{
  */ 

#define PHY_PWR_DOWN       (1<<11)
#define PHY_ADDRESS        0x00 /* default ADDR for PHY: LAN8742 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* RTC handler declaration */
RTC_HandleTypeDef RTCHandle;

/* Private function prototypes -----------------------------------------------*/
static void SYSCLKConfig_STOP(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function configures the system to enter Sleep mode for
  *         current consumption measurement purpose.
  *         Sleep Mode
  *         ==========  
  *            - System Running at PLL (100MHz)
  *            - Flash 3 wait state
  *            - Instruction and Data caches ON
  *            - Prefetch ON   
  *            - Code running from Internal FLASH
  *            - All peripherals disabled.
  *            - Wakeup using EXTI Line (user Button)
  * @param  None
  * @retval None
  */
  
  
osThreadId_t TID_Th_Sleep;
osThreadId_t TID_Th_Ricochet;
osTimerId_t Sleep_timer;
osTimerId_t Ricochet_timer; //timer rebotes
uint32_t exec;
uint8_t Ir;
static void Th_Sleep (void *arg);
static void Th_Ricochet (void *arg);


void SleepMode_Measure(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Disable USB Clock */
  __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

  /* Disable Ethernet Clock */
  __HAL_RCC_ETH_CLK_DISABLE();

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_GPIOD_CLK_ENABLE();
//  __HAL_RCC_GPIOE_CLK_ENABLE();
//  __HAL_RCC_GPIOF_CLK_ENABLE();
//  __HAL_RCC_GPIOG_CLK_ENABLE();
//  __HAL_RCC_GPIOH_CLK_ENABLE();
//  __HAL_RCC_GPIOI_CLK_ENABLE();
//  __HAL_RCC_GPIOJ_CLK_ENABLE();
//  __HAL_RCC_GPIOK_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_All;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
//  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct); 
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct); 
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
//  __HAL_RCC_GPIOB_CLK_DISABLE();
//  __HAL_RCC_GPIOC_CLK_DISABLE();
//  __HAL_RCC_GPIOD_CLK_DISABLE();
//  __HAL_RCC_GPIOE_CLK_DISABLE();
//  __HAL_RCC_GPIOF_CLK_DISABLE();
//  __HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
  __HAL_RCC_GPIOJ_CLK_DISABLE();
  __HAL_RCC_GPIOK_CLK_DISABLE();
  
  erase_screen();
  LED_Off(3);
	LED_Off(4);
	LED_Off(5);
  LED_On(1);
  
  Ir=0;

  /* Suspend Tick increment to prevent wakeup by Systick interrupt. 
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  HAL_SuspendTick();

  /* Request to enter SLEEP mode */
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* Resume Tick interrupt if disabled prior to sleep mode entry */
  HAL_ResumeTick();
  
  /* Exit Ethernet Phy from LowPower mode */
  //ETH_PhyExitFromPowerDownMode();
  __HAL_RCC_ETH_CLK_ENABLE();
	__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
  
  LED_Off(1);
}




//Sleep timer callback:
static void SleepTimer_Callback (void const *arg) {
	//Enter sleep mode
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	SleepMode_Measure();
}

//Ricochet timer callback:
static void Ricochet_Callback (void const *arg) {
	
      if (Ir == 0){
        Ir = 1;
      }else{
        Ir = 0;
      }
  
}

//Sleep Timer funcion declarations:
int Init_SleepTimer (void) {
  osStatus_t status;                            // function return status
 
  // Create one-shoot timer
  exec = 1U;
  Sleep_timer = osTimerNew((osTimerFunc_t)&SleepTimer_Callback, osTimerOnce, &exec, NULL);
  if (Sleep_timer != NULL) {  // One-shot timer created
    // start timer with delay 50ms
    status = osTimerStart(Sleep_timer, 50U); 
    if (status != osOK) {
      return -1;
    }
  }
  return NULL;
}

//Ricochet Timer funcion declarations:
int Init_RicochetTimer (void) {
  osStatus_t status;                            // function return status
 
  // Create one-shoot timer
  exec = 1U;
  Ricochet_timer = osTimerNew((osTimerFunc_t)&Ricochet_Callback, osTimerOnce, &exec, NULL);
  if (Ricochet_timer != NULL) {  // One-shot timer created
    // start timer with delay 50ms
    status = osTimerStart(Ricochet_timer, 50U); 
    if (status != osOK) {
      return -1;
    }
  }
  return NULL;
}


//Sleep init thread declaration
int Init_Th_Sleep (void) {
  
  Ir=0;
  TID_Th_Sleep = osThreadNew(Th_Sleep, NULL, NULL);
  if (TID_Th_Sleep == NULL) {
    return(-1);
  }
 
  return(0);
}

//Ricochet init thread declaration
int Init_Th_Ricochet (void) {
  
  Ir = 0;
 
  TID_Th_Ricochet = osThreadNew(Th_Ricochet, NULL, NULL);
  if (TID_Th_Ricochet == NULL) {
    return(-1);
  }
 
  return(0);
}


//Sleep thread
static __NO_RETURN void Th_Sleep(void *arg){
	Init_SleepTimer();
	osTimerStart(Sleep_timer,500);
  while(1){
    osThreadFlagsWait(SLEEP, osFlagsWaitAny, osWaitForever);
    osTimerStart(Sleep_timer,500);
    
    osThreadYield();
  }
}

//Ricochet thread
static __NO_RETURN void Th_Ricochet(void *arg){
  
  uint8_t IR2 = 0;
	Init_RicochetTimer();
  
  while(1){

    osThreadFlagsWait(RICOCHET, osFlagsWaitAny, osWaitForever);
    osTimerStart(Ricochet_timer,100);

    osThreadYield();
  }
}
