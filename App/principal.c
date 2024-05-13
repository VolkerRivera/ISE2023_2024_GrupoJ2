/*HILO PRINCIPAL GESTIONA TODA LA APLICACION */

#include "principal.h"
#include "teclado.h"
#include "Board_LED.h"
#include "SD.h"
#include "RFID.h"
#include "pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define totalUsuarios 5
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

osThreadId_t tid_Thread_Principal;         //thread id, rebote de de la tecla pulsada
osTimerId_t tim_inactividad;                     // timer id, inactividad
 
//Acceso
int numero_teclas = 0;
char tecla[5] = {'\0', '\0', '\0', '\0', '\0'};
extern MSGQUEUE_Teclado_t msg_teclaPulsada;

//NFC
t_MSGQUEUE_OBJ_RFID msg;

//Estados
char* modo_func="REPOSO";
int inactividad=0;

//Threads
extern osThreadId_t tid_ThreadAltavoz;
extern osThreadId_t tid_ThRGB;
 
//Temperatura
extern float temperatura;

extern uint8_t paginaActual;
extern identificacion Personas[5];
char IDGuardado[25];
char ContrasenaGuardada[25];
	
/* Private function prototypes -----------------------------------------------*/
//Principal
void Thread_Principal (void *argument);
int Init_Thread_Principal (void);

//Timer para los x segundos de inactividad
int Init_Timer_Inactividad (void);
static void Timer_Inactividad (void const *arg);


/* Private functions ---------------------------------------------------------*/
int Init_Timer_Inactividad (void) {
  osStatus_t status;           // function return status
  uint32_t exec1 = 1U;
  tim_inactividad = osTimerNew((osTimerFunc_t)&Timer_Inactividad, osTimerOnce, &exec1, NULL);
  if (tim_inactividad != NULL) {  // One-shot timer created 
    if (status != osOK){ 
      return -1;
    }
  }
  return NULL;
}

static void Timer_Inactividad (void const *arg){
  
  if(strcmp("REPOSO", modo_func)==0){
    
    SleepMode_Measure();
  }
	osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
	osMessageQueueReset(mid_MsgQueue_Contrasena);
	numero_teclas = 0;
	
	for(int i=0; i < 5; i++){
		tecla[i] = '\0';
	}
	
	sprintf(IDGuardado, " ");
  sprintf(ContrasenaGuardada, " ");
	inactividad=0;
  
	modo_func="REPOSO";
}

/*----------------------------------------------------------------------------
 *                 HILO PRINCIPAL 
 *---------------------------------------------------------------------------*/

int Init_Thread_Principal (void) {
  tid_Thread_Principal = osThreadNew(Thread_Principal, NULL, NULL);
  if (tid_Thread_Principal == NULL) {
    return(-1);
  }
  return(0);
}

void Thread_Principal (void *argument) {
  
	
/*-------Configuarcion Teclado------*/
	/*Inicializacion Teclado*/
	Init_Thread_Rebotes_Tecla();
	MSGQUEUE_TeclaRegistrada_t msg_teclaRegistrada;
	MSGQUEUE_Contrasena_t msg_contrasena;
	
	osStatus_t status_teclado;
	int numero[4];
	
	char IDVer[4]; //Para la verificación del ID
	char PassVer[5]; //Para la verificación de la Contraseña
	uint8_t comp;
  bool correct=false;
  bool verificar = false;
/*---------------------------------*/	
	
/*Timer Inactividad*/
	Init_Timer_Inactividad();
  
/*-------RTC------*/	
	tiempoyfecha datos;
	
	char linea1[30];
	char linea2[30];
	char timestamp[30];
	 
	osStatus_t status;
	
	osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
	osMessageQueueReset(mid_MsgQueue_Contrasena);
			
	for(int i=0; i < 5; i++){
		tecla[i] = '\0';
	}
	
	osDelay(500);
	
  while (1) {
   
		if(strcmp("REPOSO", modo_func)==0){
      
      if(inactividad==0){
        inactividad=1;
        osTimerStart(tim_inactividad, 5*60000); //Si no se realiza nada en el modo reposo durante x minutos se duerme el micro
      }

			osThreadFlagsSet(tid_ThRGB,REPOSO);
			datos=get_tiempo_fecha();
			sprintf(linea1, "%s  Temp:%.2fC", datos.fecha, temperatura);
			sprintf(linea2, "      %s", datos.tiempo);
			erase_screen();
			write(1,linea1);
			write(2,linea2);
			
			status=osMessageQueueGet(mid_MsgQueue_TeclaRegistrada, &msg_teclaRegistrada, NULL, 0U);
			if(status == osOK){
				osMessageQueueGet(mid_MsgQueue_Contrasena, &msg_contrasena, NULL, 0U);
        tecla[numero_teclas] = msg_contrasena.contrasena;
          if((strcmp("Pulsado", msg_teclaRegistrada.teclaPulsada) == 0)){
						osDelay(250);
						if(msg_teclaPulsada.tecla=='*'){
              osThreadFlagsSet(tid_ThreadAltavoz, PULSACION);
							osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
							osMessageQueueReset(mid_MsgQueue_Contrasena);
							for(int i=0; i < 5; i++){
								tecla[i] = '\0';
							}
							osTimerStart(tim_inactividad, 10000);
  						osThreadFlagsSet(id_Th_rfid,READ_RFID);
							modo_func="NFC";
						}else{
              osThreadFlagsSet(tid_ThreadAltavoz, PULSACION);
							osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
							osMessageQueueReset(mid_MsgQueue_Contrasena);
							for(int i=0; i < 5; i++){
								tecla[i] = '\0';
							}
							osTimerStart(tim_inactividad, 10000);
							modo_func="TECLADO1";
						}
					}						
      } 
			osDelay(500);
						
		}else if(strcmp("TECLADO1", modo_func)==0){
			erase_screen();
			osThreadFlagsSet(tid_ThRGB,MORADO);
			write(1, "     Teclee ID: ");
			
     /*Verificacion ID*/
      if(verificar){
        verificar = false;
				if(correct){
					correct =false;
          sprintf(IDGuardado, "ID Correcto");
					printf("ID Correcto\n");
					fflush(stdout);
					for(int i=0; i < 5; i++){
							tecla[i] = '\0';
					}
					osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
					osMessageQueueReset(mid_MsgQueue_Contrasena);
					modo_func="TECLADO2";
				}else{
					sprintf(IDGuardado, "ID Incorrecto");	
					osThreadFlagsSet(tid_ThreadAltavoz,DENEGADO);
					osThreadFlagsSet(tid_ThRGB,ROJO);
					osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
					osMessageQueueReset(mid_MsgQueue_Contrasena);
					modo_func="TECLADO1";
       }
       numero_teclas = 0;
       for (int j=0;j<3;j++){
				 numero[j]=0;
			 }
			 for(int i=0; i < 4; i++){
				tecla[i] = '\0';
			}
			 write(2, IDGuardado);
			 osDelay(1000);
			 
			 erase_screen();
			 write(1, "     Teclee ID: ");
			 sprintf(IDGuardado, " ");
			  			 
			}
			status_teclado = osMessageQueueGet(mid_MsgQueue_Contrasena, &msg_contrasena, NULL, 0U);
			 
      if(status_teclado == osOK){
        tecla[numero_teclas] = msg_contrasena.contrasena; 
				
				osTimerStart(tim_inactividad, 10000);
				
				
					if(tecla[0] == '#' || tecla[1] == '#' || tecla[2] == '#' || tecla[numero_teclas] == '*' || tecla[0] == 'A' || tecla[1] == 'A'|| tecla[0] == 'B' || tecla[1] == 'B' || tecla[0] == 'C' || tecla[1] == 'C'|| tecla[0] == 'D' || tecla[1] == 'D' || tecla[2] == '0' || tecla[2] == '1' || tecla[2] == '2' || tecla[2] == '3' || tecla[2] == '4' || tecla[2] == '5'|| tecla[2] == '6' || tecla[2] == '7' || tecla[2] == '8' || tecla[2] == '9'){ 
						
						sprintf(IDGuardado,"No valida esta tecla: %c",  tecla[numero_teclas]);
									
					}else if (numero_teclas < 3 ){
            
            osThreadFlagsSet(tid_ThreadAltavoz, PULSACION);
						
							if(numero_teclas == 0){
								numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
								sprintf(IDGuardado, "    %c",  tecla[0]);
				
							}else if (numero_teclas == 1){
								numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
								sprintf(IDGuardado, "    %c   %c",  tecla[0], tecla[1]);
				 
							}else if (numero_teclas == 2){
								numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
								sprintf(IDGuardado, "    %c   %c   %c ",  tecla[0], tecla[1], tecla[2]);
								
							}
				numero_teclas++;
			 
				}else if (numero_teclas == 3){
				
          if(tecla[3] != '#'){
						
						sprintf(IDGuardado, "    %c   %c   %c",  tecla[0], tecla[1], tecla[2]);
						   		
					}else{
						
						sprintf(IDVer, "%c%c%c\0",  tecla[0], tecla[1], tecla[2]);
						
						sprintf(IDGuardado, "Verificando ID...");
						verificar = true;
						
						comp=estaDentro("",IDVer,false);
						printf("Verificacion id, comp vale: %d", comp);
            fflush(stdout);
						if(comp == 0 || comp == 1){
							correct=true; //Comprobamos si el ID introducido está dentro de la base de datos
						}else{
              printf("Error en TECLADO 1, comp vale: %d", comp);
              fflush(stdout);
            }	
						
						write(2, IDGuardado);	
					}			 
				}
			}
			
			write(2, IDGuardado);
		  osDelay(300);
			
			//FIN Estado Teclado1-->ID 
		}else if(strcmp("TECLADO2", modo_func)==0){
			erase_screen();
			osThreadFlagsSet(tid_ThRGB,VERDE);
			write(1, "  Teclee Password: ");
			
     /*Verificacion Contraseña*/
      if(verificar){
        verificar = false; //< Se ha empezado a verificar => el flag vuelve a false
				if(correct){ //< El id introducido se encuentra en la lista de acceso REGVECINOS.TXT
					correct =false; //< El flag vuelve a su estado original >> Checkear si este estado influye en el ELSE 
          sprintf(ContrasenaGuardada, "Password Valida");
					printf("Password Correcto\n");
					fflush(stdout);
					osThreadFlagsSet(tid_ThreadAltavoz,ACCSAL);
					modo_func="ACCSAL";
				}else{
					sprintf(ContrasenaGuardada, "Password Incorrecta");	
					osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
					osMessageQueueReset(mid_MsgQueue_Contrasena);
					osThreadFlagsSet(tid_ThreadAltavoz,DENEGADO);
					osThreadFlagsSet(tid_ThRGB,ROJO);
       }
				
       numero_teclas = 0;
			 
       for (int j=0;j<4;j++){
				 numero[j]=0;
			 }
			 
			 for(int i=0; i < 5; i++){
				tecla[i] = '\0';
			 }
			 
			 write(2, ContrasenaGuardada);
			 osDelay(500);
			 
			 erase_screen();
			 write(1, "  Teclee Password: ");
			 sprintf(ContrasenaGuardada, " ");
			  			 
			}
			status_teclado = osMessageQueueGet(mid_MsgQueue_Contrasena, &msg_contrasena, NULL, 0U);
			 
      if(status_teclado == osOK){
        tecla[numero_teclas] = msg_contrasena.contrasena; 
				osTimerStart(tim_inactividad, 10000);
				
					if(tecla[0] == '#' || tecla[1] == '#' || tecla[2] == '#' || tecla[3] == '#' || tecla[numero_teclas] == '*' ||tecla[numero_teclas] == 'A' || tecla[numero_teclas] == 'B' || tecla[numero_teclas] == 'C' || tecla[numero_teclas] == 'D'){ 
						sprintf(ContrasenaGuardada,"No valida esta tecla: %c",  tecla[numero_teclas]);
						
			
					}else if (numero_teclas < 4 ){
            
            osThreadFlagsSet(tid_ThreadAltavoz, PULSACION);
						
							if(numero_teclas == 0){
									numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
									sprintf(ContrasenaGuardada, "  %c",  tecla[0]);
				
							}else if (numero_teclas == 1){
								numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
								sprintf(ContrasenaGuardada, "  %c   %c",  tecla[0], tecla[1]);
				 
							}else if (numero_teclas == 2){
								numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
								sprintf(ContrasenaGuardada, "  %c   %c   %c ",  tecla[0], tecla[1], tecla[2]);
								
							}else if (numero_teclas == 3){
                numero[numero_teclas] =  numero[numero_teclas] + tecla[numero_teclas] - '0';
                sprintf(ContrasenaGuardada, "  %c   %c   %c   %c",  tecla[0], tecla[1], tecla[2], tecla[3]);
								
							}
				numero_teclas++;
			 
				}else if (numero_teclas == 4){
				
          if(tecla[4] != '#'){
							sprintf(ContrasenaGuardada, "  %c   %c   %c   %c ",  tecla[0], tecla[1], tecla[2], tecla[3]);
						   		
					}else{
						sprintf(PassVer, "%c%c%c%c\0",  tecla[0], tecla[1], tecla[2], tecla[3]);
						
//							sprintf(claveUsuario, "%d",  contrasena);
						sprintf(ContrasenaGuardada, "Verificando password...");
						verificar = true;
              
            printf("Datos introducidos a tieneAcceso: codigoPIN-> %s, id->%s\n",PassVer, IDVer);
            fflush(stdout);
            uint8_t debug = tieneAcceso(PassVer, IDVer);
            printf("return de tieneAcceso: %d", debug);
            fflush(stdout);
							if(debug == 1){
                
								correct=true; //Comprobamos si el ID introducido está dentro de la base de datos
								sprintf(timestamp, "%s  %s", datos.tiempo, datos.fecha);
								comp=estaDentro(timestamp, IDVer, true);
								getPersonasFromThisLine(5*(paginaActual), Personas);
							}
						
						write(2, ContrasenaGuardada);
						osDelay(500);	
							
					}			 
				}
			}
			write(2, ContrasenaGuardada);
		  osDelay(300);
			//FIN Estado Teclado2-->Contraseña 
	} else if(strcmp("NFC", modo_func)==0){ 
	
		erase_screen();
		write(1, "     APROXIMA");
		write(2,"     CHIP NFC");
		osThreadFlagsSet(tid_ThRGB,AZUL);
		osDelay(300);
		
		if(osMessageQueueGet(id_MsgQueue_rfid, &msg, NULL, 0U)==osOK){
			
			memset(IDVer, 0x00, sizeof(IDVer));
			if(tieneAccesoRFID(msg.utag, IDVer)==1){
				sprintf(timestamp, "%s  %s", datos.tiempo, datos.fecha);
        comp=estaDentro(timestamp,IDVer,true);
        printf("Verificacion nfc, comp vale: %d, tag: %s, id tras comprobacion: %s", comp, msg.utag, IDVer); //< print importante
        fflush(stdout);
				getPersonasFromThisLine(5*(paginaActual), Personas);
				erase_screen();
				write(1, "     TAG"); 
				write(2,"     CORRECTO");	
				osDelay(1000);	
				osThreadFlagsSet(tid_ThreadAltavoz,ACCSAL);
				modo_func="ACCSAL";
				
			}else {
				  erase_screen();
					write(1, "     CHIP NFC");
		      write(2,"     INCORRECTO");
	      	modo_func="REPOSO";
					osThreadFlagsSet(tid_ThreadAltavoz,DENEGADO);
					osThreadFlagsSet(tid_ThRGB,ROJO);
		      osDelay(1000);

			}
		}
		
	} else if(strcmp("ACCSAL", modo_func)==0){ 	
		
		osThreadFlagsSet(tid_ThRGB,ACCESO);
		erase_screen();
		
		if(comp==1){
			write(1, "    ACCESO");
			write(2,"   VERIFICADO");

		}else if(comp==0){
			write(1, "    SALIDA");
			write(2,"   VERIFICADA");
      
		}
		
		for(int i=0; i < 5; i++){
			tecla[i] = '\0';
		}
		osMessageQueueReset(mid_MsgQueue_TeclaRegistrada);
		osMessageQueueReset(mid_MsgQueue_Contrasena);
    
    inactividad=0;

		modo_func="REPOSO";
		osDelay(1000);
	}
  osThreadYield();
	
  } //while
} //end Thread 
