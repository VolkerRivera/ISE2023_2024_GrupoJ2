#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"
#include "main.h"
#include "rtc.h"
 
osThreadId_t tid_ThLCD;                        // thread id
 
void ThreadLCDFunction (void *argument);                   // thread function
 
int Init_ThreadLCD (void) {
 
  tid_ThLCD = osThreadNew(ThreadLCDFunction, NULL, NULL);
  if (tid_ThLCD == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadLCDFunction (void *argument) {

	tiempoyfecha datos;
  
  while (1) {
    // Insert thread code here...
		//osThreadFlagsWait(FLAG_THLCD, osFlagsWaitAny, osWaitForever);
		datos=get_tiempo_fecha();
    erase_screen();
    write(1,datos.fecha);
    write(2,datos.tiempo);
		osDelay(500);
    osThreadYield();                            // suspend thread
  }
}
