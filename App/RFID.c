#include "RTE_Device.h"
#include "Driver_SPI.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "RFID.h"
#include <stdio.h>
#include "cmsis_os2.h"
#include <string.h> 
/*
MOSI (Master Out Slave In): PE6
MISO (Master In Slave Out): PE5
SCK : PE2
NSS (Slave Select): PE4 
*/
extern ARM_DRIVER_SPI Driver_SPI4;
ARM_DRIVER_SPI* SPIdrv4 = &Driver_SPI4;

 void RFID_InitPins(void);
 void RFID_WriteRegister(uint8_t addr, uint8_t val);
 uint8_t RFID_ReadRegister(uint8_t addr);
 void RFID_SetBitMask(uint8_t reg, uint8_t mask);
 void RFID_ClearBitMask(uint8_t reg, uint8_t mask);
 void RFID_AntennaOn(void);
 void RFID_AntennaOff(void);
 void RFID_Reset(void);
 RFID_Status_t RFID_Request(uint8_t reqMode, uint8_t* TagType);
 RFID_Status_t RFID_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
 RFID_Status_t RFID_Anticoll(uint8_t* serNum);
 void RFID_CalculateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
 void RFID_Halt(void);
 int Init_Th_rfid (void) ;
 static void SPI_Callback(uint32_t event);
 int Init_MsgQueue_RFID(void);
 osMessageQueueId_t get_id_MsgQueue_rfid(void);
 
osThreadId_t id_Th_rfid; 
 osMessageQueueId_t id_MsgQueue_rfid;
void Th_rfid_Function (void *argument);
void Init_Leds(void);
t_MSGQUEUE_OBJ_RFID msg_rfid;


int Init_Th_rfid (void) {
  id_Th_rfid = osThreadNew(Th_rfid_Function, NULL, NULL);
  if (id_Th_rfid == NULL) {
    return(-1);
  }
 
  return(0);
}


void Th_rfid_Function (void *argument) {
  Init_MsgQueue_RFID();
  RFID_Init();
	int ini=0;
	uint8_t UID[5];
  while (1) {
   osDelay(500U);
	 if(ini==0){
		 osThreadFlagsWait(READ_RFID,osFlagsWaitAny,osWaitForever);
		 ini =1;
	 }else{

	 if(MI_OK==RFID_Check(UID)){
		for(int i=0;i<5;i++){
			if(i<4){
			sprintf(msg_rfid.utag + i * 3, "%02X:", UID[i]);
			}else {
				sprintf(msg_rfid.utag + i * 3, "%02X", UID[i]);//El ultimo byte va sin el :
			}
			
		}
       ini=0;
     osMessageQueuePut(id_MsgQueue_rfid, &msg_rfid, 0U, 0U);
	 }
	 
	 }
		     osThreadYield();
  }
}

osMessageQueueId_t get_id_MsgQueue_rfid(void){
return id_MsgQueue_rfid;
}




	
	
//QUEUE
int Init_MsgQueue_RFID(void){
  id_MsgQueue_rfid = osMessageQueueNew(MSGQUEUE_OBJECTS_RFID, sizeof(t_MSGQUEUE_OBJ_RFID), NULL);
  if(id_MsgQueue_rfid == NULL)
    return (-1); 
  return(0);
}	



//Inicializa los registros del sensor 
void RFID_Init(void){
	RFID_InitPins();


	RFID_Reset();

	RFID_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
	RFID_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
	RFID_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);           
	RFID_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);

	/* 48dB gain */
	RFID_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
	
	RFID_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	RFID_WriteRegister(MFRC522_REG_MODE, 0x3D);

	RFID_AntennaOn();		//Open the antenna
}

//Inicializa los pines
void RFID_InitPins(void) {
	static GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
 	/*NSS*/
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
	
		/*SPI*/
	SPIdrv4->Initialize(SPI_Callback);
  SPIdrv4-> PowerControl(ARM_POWER_FULL);
  SPIdrv4-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0  | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),10000000);
}

//Callback del SPI
static void SPI_Callback(uint32_t event){
	    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        /* Success: Wakeup Thread */
        osThreadFlagsSet(id_Th_rfid, RFID_TRANSFER_FLAG);
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        //__breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
       // __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
	}
}

//Escritura de 1 registro(1 byte) un dato de 1 byte
void RFID_WriteRegister(uint8_t addr, uint8_t val) {
  int8_t tx_data_wr[2];
	//ARM_SPI_STATUS status_spi;
  //Formar el dato a enviar al sensor
  tx_data_wr[0] = (addr << 1)& 0x7E;  // Escribir la dirección del registro (7 bits) y el bit de escritura (0)
  tx_data_wr[1] = val;           
	
  // Selección del dispositivo esclavo (NSS bajo)
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
  // Envío de datos al sensor RFID RC522
  
  SPIdrv4->Send(&tx_data_wr, sizeof(tx_data_wr));
  osThreadFlagsWait(RFID_TRANSFER_FLAG,osFlagsWaitAny,osWaitForever);

  // Deselección del dispositivo esclavo (NSS alto)
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
}

//Lectura de un registro (1 byte)
uint8_t RFID_ReadRegister(uint8_t addr) {
  uint8_t tx_data = ((addr << 1)&0x7E) | 0x80;
  uint8_t rx_data; // Para almacenar los datos recibidos
	//ARM_SPI_STATUS status_spi;
	
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
	
  
	SPIdrv4->Send(&tx_data, sizeof(tx_data));
  osThreadFlagsWait(RFID_TRANSFER_FLAG,osFlagsWaitAny,osWaitForever);

		
  
		SPIdrv4->Receive(&rx_data, sizeof(rx_data));
  osThreadFlagsWait(RFID_TRANSFER_FLAG,osFlagsWaitAny,osWaitForever);
	
	 // Deselección del dispositivo esclavo (NSS alto)
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
  return rx_data; // El dato recibido está en el segundo byte
}

//Funciones para cambiar bits especificos de un registro
void RFID_SetBitMask(uint8_t reg, uint8_t mask) {
	RFID_WriteRegister(reg, RFID_ReadRegister(reg) | mask);
}

void RFID_ClearBitMask(uint8_t reg, uint8_t mask){
	RFID_WriteRegister(reg, RFID_ReadRegister(reg) & (~mask));
}

//Activar/desactivar antena
void RFID_AntennaOn(void) {
	uint8_t temp;

	temp = RFID_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) {
		RFID_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
	}
}

void RFID_AntennaOff(void) {
	RFID_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

//Soft reset
void RFID_Reset(void) {
	RFID_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

//Peticion de lectura de una tarjeta
RFID_Status_t RFID_Request(uint8_t reqMode, uint8_t* TagType) {
	RFID_Status_t status;  
	uint16_t backBits;			//The received data bits

	RFID_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		//TxLastBists = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = RFID_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {    
		status = MI_ERR;
	}

	return status;
}

//Realizacion de operaciones de comunicación con la tarjeta RFID
RFID_Status_t RFID_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	RFID_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	RFID_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	RFID_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
	RFID_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);

	RFID_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {   
		RFID_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);    
	}

	//Execute the command
	RFID_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) {    
		RFID_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts  
	}   

	//Waiting to receive data to complete
	i = 2500;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = RFID_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	RFID_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);			//StartSend=0

	if (i != 0)  {
		if (!(RFID_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {
			status = MI_OK;
			if (n & irqEn & 0x01) {   
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE) {
				n = RFID_ReadRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = RFID_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits) {   
					*backLen = (n - 1) * 8 + lastBits;   
				} else {   
					*backLen = n * 8;   
				}

				if (n == 0) {   
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN) {   
					n = MFRC522_MAX_LEN;   
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++) {   
					backData[i] = RFID_ReadRegister(MFRC522_REG_FIFO_DATA);    
				}
			}
		} else {   
			status = MI_ERR;  
		}
	}

	return status;
}

//Anticolision
RFID_Status_t RFID_Anticoll(uint8_t* serNum) {
	RFID_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	RFID_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);		//TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = RFID_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		//Check card serial number
		for (i = 0; i < 4; i++) {   
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {   
			status = MI_ERR;    
		}
	}
	return status;
} 

//Calculo CRC (Para errores)
void RFID_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	RFID_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	RFID_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
	for (i = 0; i < len; i++) {   
		RFID_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));   
	}
	RFID_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = RFID_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = RFID_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = RFID_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

//Poner en inactivo la tarjeta
void RFID_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	RFID_CalculateCRC(buff, 2, &buff[2]);

	RFID_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

//Comprobar si hay una tarjeta en el sensor
RFID_Status_t RFID_Check(uint8_t* id) {
	RFID_Status_t status;
	//Find cards, return card type
	status = RFID_Request(PICC_REQIDL, id);	
	if (status == MI_OK) {
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = RFID_Anticoll(id);	
	}
	RFID_Halt();			//Command card into hibernation 

	return status;
}

