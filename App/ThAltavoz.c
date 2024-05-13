#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "tim.h"
#include "altavoz.h"
#include "principal.h"

/* BELONG TO THIS THREAD */
osThreadId_t tid_ThreadAltavoz; //THREAD ALTAVOZ ID
void FuncionThreadAltavoz (void *argument);// thread function

int Init_ThreadAltavoz (void) {
 
  tid_ThreadAltavoz = osThreadNew(FuncionThreadAltavoz, NULL, NULL);
  if (tid_ThreadAltavoz == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void FuncionThreadAltavoz (void *argument) {
	
	uint32_t flag_recibida=0;

  while (1) {
		
    //Dependiendo de la flag suena un tono u otro
		flag_recibida=osThreadFlagsWait(PULSACION | DENEGADO | ACCSAL, osFlagsWaitAny, osWaitForever);

		if(flag_recibida==PULSACION){
			tono_tecla();
			printf("Se ha transmitido el tono tecla\n");
		}else if(flag_recibida==DENEGADO){
			tono_fallo();
			printf("Se ha transmitido el tono fallo\n");
		}else if(flag_recibida==ACCSAL){
			tono_entrada();
			printf("Se ha transmitido el tono entrada\n");
		}	
//		tono_entrada();
//		printf("Se ha transmitido el tono entrada\n");
//		

		
    osThreadYield();                            // suspend thread
  }
}
