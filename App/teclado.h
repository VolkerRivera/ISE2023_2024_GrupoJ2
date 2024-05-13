/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TECLADO_H
#define __TECLADO_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"
#include <string.h>
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
#include "lcd.h"


/* Exported types ------------------------------------------------------------*/
 typedef struct { 
    char* teclaPulsada;
  } MSGQUEUE_TeclaRegistrada_t;
	
 typedef struct { 
    char contrasena;
  } MSGQUEUE_Contrasena_t;
		
 typedef struct {
    char tecla;
  } MSGQUEUE_Teclado_t;
 
	

/* Exported constants --------------------------------------------------------*/
#define REBOTES 0x00000020

	
extern osThreadId_t tid_ThreadFlags_rebotes;         //thread id, rebote de de la tecla pulsada
extern osMessageQueueId_t mid_MsgQueue_TeclaRegistrada;                // message queue id
extern osMessageQueueId_t mid_MsgQueue_Contrasena; 

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern int Init_Thread_Rebotes_Tecla (void);

#endif /* __TECLADO_H */

	