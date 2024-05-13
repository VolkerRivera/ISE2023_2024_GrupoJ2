#include <stdio.h>                      /* Standard I/O .h-file               */
#include <ctype.h>                      /* Character functions                */
#include <string.h>                     /* String and memory functions        */
#include "rl_fs.h"
#include "Board_LED.h"                  // CMSIS RTOS header file
#include "cmsis_os2.h"

#ifndef __SD_H

typedef struct {
	char codigoPIN[5]; //< 4 para el codigo + 1 para el \0 que indica fin de array
	char tag[15];
	char nombre[20];
	char apellido[20];
	char DNI [4];	//< 3 para el DNI (2 numeros + 1 letra) + 1 para el \0 que indica fin de array
	char estaDentro[2];
} identificacion;

uint8_t tieneAcceso(char codigoPIN[], char ID[]); 
//uint8_t tieneAccesoRFID(char tag[]); //Unicamente comrobar que pertenece a la línea
uint8_t tieneAccesoRFID(char tag[], char IDVer[]);
uint8_t addVecino(identificacion vecino);
uint8_t deleteVecino(char codigoPIN[], char DNI[]);
uint8_t estaDentro(char timestamp[], char DNI[], bool cambiarEstado); //cambiarEstado = false solo en consulta (pagina web p.ej), cambiarEstado = true cuando la persona accede/sale. Devuelve el estado TRAS la actualizacion
//uint8_t RFID_Used(); //quedaria añadir este numero a la estructura de identificacion
//uint8_t PIN_Used(char DNI[],char codigoPIN[]);
uint8_t registerPerson(char timestamp[], char DNI[], char InOut[]); //hora de entrada o salida, persona que entra o sale, metodo de entrada o salida
uint8_t mount_unmount(bool mount);
uint8_t cuanta_gente(void);
void getPersonasFromThisLine(uint8_t numeroLinea, identificacion Personas[5]);
identificacion getPersona(char linea_n[]);
	
#endif
