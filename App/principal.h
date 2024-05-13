/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINCIPAL_H
#define __PRINCIPAL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"
#include <string.h>
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
#include "lcd.h"
#include "teclado.h"
#include "rtc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ROJO 0X00000001U //Acceso denegado
#define MORADO 0X00000003U //Estado Teclado 1--->Verificar ID
#define VERDE 0x00000002U //Estado Teclado 2--->Verificar password
#define AZUL 0x00000004U //Estado NFC
#define REPOSO 0x00000008U //Estado Reposo
#define ACCESO 0x00000005U //Estado Acceso

#define PULSACION 0X00000001U
#define DENEGADO 0x00000002U
#define ACCSAL 0x00000004U
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern int Init_Thread_Principal (void);

#endif /* __PRINCIPAL_H */
