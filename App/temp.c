#include "main.h"
#include "temp.h" 


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_temp;                        // thread id
 
 /* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
 
static volatile uint32_t I2C_Event; 
 
 
 /* I2C Signal Event function callback */
void I2C_SignalEvent (uint32_t event) {
  osThreadFlagsSet(tid_Thread_temp, event);
}
 
/* Initialize I2C connected EEPROM */
void I2C_Initialize (void) {
  I2Cdrv->Initialize (I2C_SignalEvent);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
}
 
void trans_registros(uint8_t comando[1]){
  uint32_t flags;
  comando[0]=0x01;
  I2Cdrv->MasterTransmit(0x48, comando, 1, true);
   flags = osThreadFlagsWait(S_FLAG_TEMP, osFlagsWaitAny, osWaitForever);
  comando[0]=0x00;
  I2Cdrv->MasterTransmit(0x48, comando, 1, true);
   flags = osThreadFlagsWait(S_FLAG_TEMP, osFlagsWaitAny, osWaitForever);
}

void recep_temp(uint8_t temp[2]){
  uint32_t flags;
  I2Cdrv->MasterReceive(0x48, temp, 2, false);
  flags = osThreadFlagsWait(S_FLAG_TEMP, osFlagsWaitAny, osWaitForever);
}

float temperatura = 0;

void conv_temp(uint8_t temp[2], int temp_i2c){
  
  temp_i2c=((temp[0])<<8 | temp[1])>>5;
	temperatura = temp_i2c*0.125;
  
}

void Thread_Temp (void *argument);                   // thread function

int Init_Temperatura (void) {
 
  tid_Thread_temp = osThreadNew(Thread_Temp, NULL, NULL);
  if (tid_Thread_temp == NULL) {
    return(-1);
  }
 
  return(0);
}

void Thread_Temp (void *argument) {
  
   int temp_i2c;
   uint8_t temp[2];
   uint8_t comando[1];
   I2C_Initialize();
  
  while (1) {
    ;// Insert thread code here...
    trans_registros(comando);
    recep_temp(temp);
    conv_temp(temp, temp_i2c);		
    osDelay(1000);
    osThreadYield();                            // suspend thread
  }
}
