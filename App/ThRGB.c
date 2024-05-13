#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "principal.h" 
#include "Board_LED.h"
/*----------------------------------------------------------------------------
 *      ThRGB 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThRGB;                        // thread id
 
void ThRGB (void *argument);                   // thread function
 
int Init_ThRGB (void) {
 
  tid_ThRGB = osThreadNew(ThRGB, NULL, NULL);
  if (tid_ThRGB == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThRGB (void *argument) {
 
	uint32_t flag_recibida=0;
	LED_Off(0);
	LED_Off(1);
	LED_Off(2);
	LED_Off(3);
	LED_Off(4);
	LED_Off(5);
	
  while (1) {
    ; // Insert thread code here...
		flag_recibida=osThreadFlagsWait(ROJO | VERDE | AZUL | REPOSO | ACCESO, osFlagsWaitAny, osWaitForever);
		if(flag_recibida==ROJO){
			LED_Off(4);
			LED_Off(5);
			LED_On(3);
			osDelay(250);
			LED_Off(3);
			osDelay(250);
			LED_On(3);
			osDelay(250);
			LED_Off(3);
			osDelay(250);
			LED_On(3);
			osDelay(250);
			LED_Off(3);
			osDelay(250);
		}else if(flag_recibida==MORADO){
			LED_On(3);
			LED_On(5);
			LED_Off(4);			
		}else if(flag_recibida==VERDE){
			LED_Off(3);
			LED_Off(5);
			LED_On(4);			
		}else if(flag_recibida==AZUL){
			LED_Off(3);
			LED_Off(4);
			LED_On(5);	
		}else if(flag_recibida==REPOSO){
			LED_Off(3);
			LED_On(4);
			LED_On(5);
		}else if(flag_recibida==ACCESO){
			LED_On(3);
			LED_On(4);
			LED_On(5);
			osDelay(200);
			LED_Off(3);
			LED_Off(4);
			LED_Off(5);
			osDelay(200);
			LED_Off(3);
			LED_Off(4);
			LED_Off(5);
			osDelay(200);
			LED_On(3);
			LED_On(4);
			LED_On(5);
			osDelay(200);
			LED_Off(3);
			LED_Off(4);
			LED_Off(5);
			osDelay(200);
			LED_Off(3);
			LED_Off(4);
			LED_Off(5);
		}
    osThreadYield();                            // suspend thread
  }
}
