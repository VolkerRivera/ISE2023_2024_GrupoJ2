
/****************TECLADO MATRICIAL*********************
         
   FILA/COLUMNA     1         2        3        4
                  (PE13)   (PF15)    (PG14)    (PG9)
  
    1(PF13)         '1'       '2'       '3'      'A'
    
    2(PE9)         '4'       '5'       '6'      'B'
     
    3(PE11)         '7'       '8'       '9'      'C'
  
    4(PF14)        '*'       '0'       '#'      'D'
  
  *****************************************************/
/* Includes ------------------------------------------------------------------*/
#include "teclado.h"
#include "altavoz.h"



/* Private typedef -----------------------------------------------------------*/
		
/* Private define ------------------------------------------------------------*/
#define MSGQUEUE_TECLADO 4                     // number of Message Queue Objects
#define Time_Rebotes_Tecla  200 						 // tiempo de muestreo tecla pulsada 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

osThreadId_t tid_ThreadFlags_rebotes;         //thread id, rebote de de la tecla pulsada
osThreadId_t tid_ThreadFlags_registroTecla;       //thread id, registro de la tecla pulsada
osTimerId_t tim_teclado;                     // timer id, rebote de la tecla pulsada 
	

	
osMessageQueueId_t mid_MsgQueue_Teclado;                // message queue id
osMessageQueueId_t mid_MsgQueue_Contrasena;                // message queue id
osMessageQueueId_t mid_MsgQueue_TeclaRegistrada;                // message queue id
	
MSGQUEUE_Teclado_t msg_teclaPulsada;      //tecla pulsada 	 

	 
char TeclasRegistradas[5] = {'\0', '\0', '\0', '\0', '\0'};


/* Private function prototypes -----------------------------------------------*/
void Config_teclado (void);  //Funcion para configuracion del teclado
void RegistrarContrasena(char tecla);  //Funcion para registrar constraseña 

/*Timer*/
static void TimerBarrido_Callback (void const *arg); // Timer: Barrido de FILAS Y COLUMNAS
int Init_Timer_Teclado (void);

/*Cola de mensaje*/
int Init_MsgQueue_Teclado (void); //Cola de mensaje registra Tecla Pulsada 
int Init_MsgQueue_Contrasena (void); //Cola de mensaje registra Contraseña Tecleada 
int Init_MsgQueue_TeclaRegistrada (void);

/*Thread*/
int Init_Thread_Rebotes_Tecla (void);
int Init_Thread_Registro_Tecla (void);
void ThreadFlags_Rebotes_Tecla (void *argument);
void ThreadFlags_Registro_Tecla (void *argument);

/* Private functions ---------------------------------------------------------*/

/*--------------------------Configuración del Teclado -----------------------*/
 void Config_teclado (void){                        //funcion que configura todos los relojes y puertos de los pulsadores del teclado
  GPIO_InitTypeDef GPIO_InitStruct_Teclado;
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  
 /********FILAS************/
  GPIO_InitStruct_Teclado.Pin = GPIO_PIN_13 | GPIO_PIN_14;
  GPIO_InitStruct_Teclado.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_Teclado.Pull = GPIO_NOPULL;
  GPIO_InitStruct_Teclado.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_Teclado);
  GPIO_InitStruct_Teclado.Pin =  GPIO_PIN_9 | GPIO_PIN_11;
  GPIO_InitStruct_Teclado.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_Teclado.Pull = GPIO_NOPULL;
  GPIO_InitStruct_Teclado.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_Teclado);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
 
 /******COLUMNAS********/
  GPIO_InitStruct_Teclado.Pin = GPIO_PIN_13;
  GPIO_InitStruct_Teclado.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct_Teclado.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_Teclado);
  GPIO_InitStruct_Teclado.Pin = GPIO_PIN_15;
  GPIO_InitStruct_Teclado.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct_Teclado.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_Teclado);
  GPIO_InitStruct_Teclado.Pin = GPIO_PIN_14 | GPIO_PIN_9;
  GPIO_InitStruct_Teclado.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct_Teclado.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct_Teclado);
}
 

/*----------------------------------------------------------------------------
 *                  Timer: Barrido de FILAS Y COLUMNAS 
 *---------------------------------------------------------------------------*/
static void TimerBarrido_Callback (void const *arg) { // One-Shoot Timer Function
  
 /***  Matrix Keyboard ****** 
   FILA/COLUMNA     1         2        3        4
                  (PE13)   (PF15)    (PG14)    (PG9)
  
    1(PF13)         '1'       '2'       '3'      'A'
    
    2(PE9)         '4'       '5'       '6'      'B'
     
    3(PE11)         '7'       '8'       '9'      'C'
  
    4(PF14)        '*'       '0'       '#'      'D'
  
  *******************************************/
  char matrizTeclas [4][4] ={ { '1' , '2' , '3' , 'A'} ,
                              {'4', '5',  '6' , 'B' },
                              { '7', '8' , '9' , 'C' },
                              {'*', '0', '#', 'D' } }; 
  
  MSGQUEUE_TeclaRegistrada_t msg_teclaRegistrada;

                             
  for(int i = 0; i<4; i++){  //Bucle de barrido FILAS Y COLUMNAS 
    
/*---------------------- MUESTREO FILA 1--- ----------------*/    
    if(i==0){
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);   //FILA  1 muestreada 
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_SET);
      
/********************* MUESTREO DE COLUMNAS *************************/
      if((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)) == 0){  //COLUMNA 1 muestrada 
        msg_teclaPulsada.tecla = matrizTeclas[0][0];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
        
     }else if ((HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15)) == 0){ //COLUMNA 2 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[0][1];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14)) == 0){ //COLUMNA 3 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[0][2];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_9)) == 0){ //COLUMNA 4 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[0][3];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
   }
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_RESET);  
 
/*--------------------------------------------------------------------------*/ 
/*----------------- MUESTREO FILA 2 ----------------------------------------*/    
    }else if(i==1){
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET); //FILA 2 muestreada 
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
      
/********************* MUESTREO DE COLUMNAS *************************/
      if((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)) == 0){ //COLUMNA 1 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[1][0];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
        
     }else if ((HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15)) == 0){ //COLUMNA 2 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[1][1];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14)) == 0){ //COLUMNA 3 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[1][2];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_9)) == 0){ //COLUMNA 4 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[1][3];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
     }
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_RESET);

/*--------------------------------------------------------------------------*/      
/*----------------- MUESTREO FILA 3 ----------------*/  
    }else if(i==2){
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
    
/********************* MUESTREO DE COLUMNAS *************************/
      if((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)) == 0){ //COLUMNA 1 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[2][0];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
        
     }else if ((HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15)) == 0){ //COLUMNA 2 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[2][1];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14)) == 0){ //COLUMNA 3 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[2][2];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_9)) == 0){ //COLUMNA 4 muestrada
        msg_teclaPulsada.tecla = matrizTeclas[2][3];
        osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       
     }
     
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_RESET);
     
/*--------------------------------------------------------------------------*/      
/*----------------- MUESTREO FILA 4 ----------------*/      
    }else if(i==3){
       HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_SET);
       HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
      
/********************* MUESTREO DE COLUMNAS *************************/
       if((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)) == 0){ //COLUMNA 1 muestrada
         msg_teclaPulsada.tecla = matrizTeclas[3][0];
         osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
         
       }else if ((HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15)) == 0){ //COLUMNA 2 muestrada
         msg_teclaPulsada.tecla = matrizTeclas[3][1];
         osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
         
       }else if ((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14)) == 0){ //COLUMNA 3 muestrada
         msg_teclaPulsada.tecla = matrizTeclas[3][2]; 
         osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
         
       }else if((HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_9)) == 0){ //COLUMNA 4 muestrada
         msg_teclaPulsada.tecla = matrizTeclas[3][3];
         osMessageQueuePut(mid_MsgQueue_Teclado, &msg_teclaPulsada, 0U, 0U);
       }
    
      HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_RESET);
			 
			  
    }
  }
  
  msg_teclaRegistrada.teclaPulsada = "Pulsado";
			 
	osMessageQueuePut(mid_MsgQueue_TeclaRegistrada, &msg_teclaRegistrada, 0U, 0U); //Envia mensaje de tecla pulsada 
  
	
}


int Init_Timer_Teclado (void) {
  osStatus_t status;           // function return status
  uint32_t exec1 = 1U;
  tim_teclado = osTimerNew((osTimerFunc_t)&TimerBarrido_Callback, osTimerOnce, &exec1, NULL);
  if (tim_teclado != NULL) {  // One-shot timer created 
    if (status != osOK){ 
      return -1;
    }
  }
  return NULL;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *                       Cola de mensajes Tecla Pulsada 
 *---------------------------------------------------------------------------*/
int Init_MsgQueue_Teclado (void) {
  mid_MsgQueue_Teclado = osMessageQueueNew(MSGQUEUE_TECLADO, sizeof(MSGQUEUE_Teclado_t), NULL);

  if (mid_MsgQueue_Teclado == NULL) {
    return(-1); 
  }
   return(0);
}

/*----------------------------------------------------------------------------
 *                       Cola de mensajes del Tecla registrada
 *---------------------------------------------------------------------------*/

int Init_MsgQueue_Contrasena (void) {
  mid_MsgQueue_Contrasena = osMessageQueueNew(MSGQUEUE_TECLADO, sizeof(MSGQUEUE_Contrasena_t), NULL);
  if (mid_MsgQueue_Contrasena == NULL) {
    return(-1); 
  }
  return(0);
}

/*----------------------------------------------------------------------------
 *                       Cola de mensajes del Tecla registrada
 *---------------------------------------------------------------------------*/

int Init_MsgQueue_TeclaRegistrada (void) {
	
  mid_MsgQueue_TeclaRegistrada = osMessageQueueNew(MSGQUEUE_TECLADO, sizeof(MSGQUEUE_TeclaRegistrada_t), NULL);
  if (mid_MsgQueue_TeclaRegistrada == NULL) {
    return(-1); 
  }
  return(0);
}


void RegistrarContrasena(char tecla) {
  
  MSGQUEUE_Contrasena_t msg_contrasena;
  int i;
  int j;
  bool registrado = false;

        for(i = 0; i < 5 && !registrado; i++){  //recorro todo mi array de claves
          if (TeclasRegistradas[i] == '\0' && TeclasRegistradas[4] != '#' ) { //si mi array es distinto a estar vacio y no he pulsado # en la cuarta posicion
              
              if( tecla == '#' || tecla == '*' || tecla == 'A' || tecla == 'B' || tecla == 'C' || tecla == 'D'){ //si he pulsado alguna de las teclas # * A B C D
                   msg_contrasena.contrasena = tecla;                                                //guardame la tecla en la cola
                  osMessageQueuePut(mid_MsgQueue_Contrasena, &msg_contrasena, 0U, 0U);              //enviamela
                
              }else{
                TeclasRegistradas[i] = tecla;             //si no es ninguna de esas registramela
                
                msg_contrasena.contrasena = tecla;         //guardame la tecla en la cola
                osMessageQueuePut(mid_MsgQueue_Contrasena, &msg_contrasena, 0U, 0U); //enviamela
              }
              registrado = true;
          
          }else if (TeclasRegistradas[i] != '\0' && TeclasRegistradas[i] != '#' && i == 4) {  //si  para esa posicion de tecla registrada no esta vacia y no es # y la posicion es 4
            
               if(tecla  == '*' && tecla == 'A' && tecla == 'B' && tecla == 'C' && tecla == 'D'){ //al ser la ultima posicion, si es A,B,C,D entonces
                  msg_contrasena.contrasena = tecla;   //guarda en cola
                  osMessageQueuePut(mid_MsgQueue_Contrasena, &msg_contrasena, 0U, 0U); //enviamela
                 
               }else{  //si no lo es, registrala, es decir, si es #
                  TeclasRegistradas[i] = //tecla; registra esa tecla
              
                    msg_contrasena.contrasena = tecla; //metela en cola 
                    osMessageQueuePut(mid_MsgQueue_Contrasena, &msg_contrasena, 0U, 0U); //enviala
                 
                   if (TeclasRegistradas[i] == '#') {   //si estando en la posicion 4 la tecla ultima que se ha registrado es 4, borrame todo el array de teclas y dejamelo limpio
                      for (int i = 0; i < 5; i++) {
                              TeclasRegistradas[i] = '\0';
                       }
                   }
               }   
               registrado = true;
             }
        }
}

/*----------------------------------------------------------------------------
 *           HILO: Gestiona los rebotes de la pulsacion de una tecla 
 *---------------------------------------------------------------------------*/

int Init_Thread_Rebotes_Tecla (void) {
  tid_ThreadFlags_rebotes = osThreadNew(ThreadFlags_Rebotes_Tecla, NULL, NULL);
  if (tid_ThreadFlags_rebotes == NULL) {
    return(-1);
  }
  return(0);
}

void ThreadFlags_Rebotes_Tecla (void *argument) {
  
	/*Configuracion teclado*/
	Config_teclado();

	/*Inicializacion Colas de Mensaje*/
	Init_MsgQueue_Teclado(); 
	Init_MsgQueue_Contrasena(); 
	Init_MsgQueue_TeclaRegistrada ();
	
	/*Inicializacion Timer*/
	Init_Timer_Teclado ();
	
	/*Inicializacion Hilo de registro*/
	Init_Thread_Registro_Tecla();
	
	osStatus_t status_rebotes;
	osStatus_t status_teclaRegistrada;
	
   
	
  while (1) {
		//Espera a una interrupcion del teclado
     osThreadFlagsWait(REBOTES, osFlagsWaitAny, osWaitForever);
		
			osTimerStart(tim_teclado, Time_Rebotes_Tecla);    //Arranca timer para la gestion de los rebotes
			
			
    osThreadYield();
  }
}


/*----------------------------------------------------------------------------
 *           HILO: Registra la tecla pulsada en la contraseña  
 *---------------------------------------------------------------------------*/
int Init_Thread_Registro_Tecla (void) {
  tid_ThreadFlags_registroTecla = osThreadNew(ThreadFlags_Registro_Tecla, NULL, NULL);
  if (tid_ThreadFlags_rebotes == NULL) {
    return(-1);
  }
  return(0);
}


void ThreadFlags_Registro_Tecla (void *argument) {
 
	osStatus_t status_teclaRegistrada;
	char teclaP;
	
	
  while (1) {
			
			status_teclaRegistrada = osMessageQueueGet(mid_MsgQueue_Teclado, &msg_teclaPulsada, NULL, 0U);
			if(status_teclaRegistrada == osOK){
				teclaP = msg_teclaPulsada.tecla;
				RegistrarContrasena(teclaP);
			}	
			osDelay(300);
    osThreadYield();
  }
}




