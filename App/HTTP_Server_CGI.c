/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.0.0
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "rtc.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "SD.h"

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

//debug
uint8_t debugAddVecino = 0xF;
uint8_t debugDeleteVecino = 0xF;

//modulo sd
extern uint8_t paginaActual;
extern uint8_t numMaxPaginas;
extern uint16_t aforo;
extern identificacion Personas[5];

// http_server.c
extern uint16_t AD_in (uint32_t ch);
extern uint8_t  get_button (void);

extern bool LEDrun;
extern char lcd_text[2][20+1];
extern osThreadId_t TID_Display;

// Local variables.
//static uint8_t ip_addr[NET_ADDR_IP6_LEN];


// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

/* EN ESTE CASO LO QUE REPRESENTAMOS EN EL SERVIDOR NO LO SOLICITAMOS CON GET SINO QUE USAMOS EL CGI SCRIPT */

// Process query string received by GET request. GET-> Solicita datos del servidor
/*
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) { //si var[0] no esta vacia y var[2] contiene un igual
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}
*/

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
	identificacion vecino; //cada vez que llega un submit creamos un objeto tipo identificacion
  memset(&vecino, 0, sizeof(vecino)); //se inicializa a 0 para que al escribir los valores no salgan caracteres raros
	char var[40];
	//char passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }

  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    printf("\n");
		fflush(stdout);
    return;
  }
  //passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
		
    if (var[0] != 0) {
			printf("%s\n", var);
			fflush(stdout);
      // First character is non-null, string exists
			
      if (strcmp (var, "nuevaPaginaActual=<<") == 0) {
        if(paginaActual == 0 || numMaxPaginas == 0){
          paginaActual = paginaActual;
        }else{
          paginaActual--;
					memset(&Personas, 0, sizeof(Personas)); //formateamos
					getPersonasFromThisLine(5*(paginaActual), Personas); //Leemos las 5 personas correspondientes a la pagina actual
        }
      }
      else if (strcmp (var, "nuevaPaginaActual=>>") == 0) {
        if(paginaActual == numMaxPaginas-1 || numMaxPaginas == 0){
          paginaActual = paginaActual;
        }else{
          paginaActual++;
					memset(&Personas, 0, sizeof(Personas)); //formateamos
					getPersonasFromThisLine(5*(paginaActual), Personas); //Leemos las 5 personas correspondientes a la pagina actual
        }
      }
			else if (strncmp(var, "nombre=", 7) == 0) { //comprobamos si var contiene el valor "nombre" del formulario
            strncpy(vecino.nombre, var + 7, sizeof(vecino.nombre) - 1); //Si es asi, copiamos vecino.nombre el valor de var a partir del caracter 7 
      }																																	// y hasta un maximo del sizeof - 1, esto es para dejar siempre espacio para el \0 que finaliza el array de caracteres
			else if (strncmp(var, "apellidos=", 10) == 0) {
            strncpy(vecino.apellido, var + 10, sizeof(vecino.apellido) - 1);
      }
			else if (strncmp(var, "dni=", 4) == 0) {
            strncpy(vecino.DNI, var + 4, sizeof(vecino.DNI) - 1);
      }
			else if (strncmp(var, "password=", 9) == 0) {
            strncpy(vecino.codigoPIN, var + 9, sizeof(vecino.codigoPIN) - 1);
      }
			else if (strncmp(var, "rfid=", 5) == 0) {
            strncpy(vecino.tag, var + 5, sizeof(vecino.tag) - 1);
      }
			else if (strncmp(var, "dentro=", 7) == 0) {
				strncpy(vecino.estaDentro, var + 7, sizeof(vecino.codigoPIN) - 1);
				
				if (vecino.nombre[0] && vecino.apellido[0] && vecino.DNI[0] && vecino.codigoPIN[0] && vecino.tag[0] && vecino.estaDentro[0]){ //si se han llenado todos los campos
					debugAddVecino = addVecino(vecino);
          if(debugAddVecino == 1){
            aforo++; //Si se añade correctamente incrementamos, si no se queda como esta
            numMaxPaginas = (aforo%5 == 0) ? aforo/5 : (aforo/5)+1; //actualizamos numero de paginas
						memset(&Personas, 0, sizeof(Personas)); //formateamos
						getPersonasFromThisLine(5*(paginaActual), Personas); //Leemos las 5 personas correspondientes a la pagina actual
          }
				}else{
					printf("Error añadiendo: No se han podido asignar todos los valores del formulario\n");
					fflush(stdout);
				}
      }else if (strncmp(var, "dni_delete=", 11) == 0) {
            strncpy(vecino.DNI, var + 11, sizeof(vecino.DNI) - 1);
      }
			else if (strncmp(var, "codigoPIN_delete=", 17) == 0) {
            strncpy(vecino.codigoPIN, var + 17, sizeof(vecino.codigoPIN) - 1);
				if (vecino.DNI[0] && vecino.codigoPIN[0]){
					debugDeleteVecino = deleteVecino(vecino.codigoPIN, vecino.DNI);
          if(debugDeleteVecino == 1){
            aforo--; //Si se añade correctamente incrementamos, si no se queda como esta
            numMaxPaginas = (aforo%5 == 0) ? aforo/5 : (aforo/5)+1; //actualizamos numero de paginas
						memset(&Personas, 0, sizeof(Personas)); //formateamos
						getPersonasFromThisLine(5*(paginaActual), Personas); //Leemos las 5 personas correspondientes a la pagina actual
          }
				}else{
					printf("Error eliminando: No se han podido asignar todos los valores del formulario\n");
					fflush(stdout);
				}
      }
			
    }
  } while (data);
}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {

  uint32_t len = 0U;

  switch (env[0]) {//Se evalua env[0], que es un caracter. Segun la letra que sea, se evaluan otros indices de env[]

    case 'h':
      len = (uint32_t)sprintf (buf, &env[1], paginaActual);
    break;
		
		case 'i':
			if(numMaxPaginas>0){
				len = (uint32_t)sprintf (buf, &env[1], numMaxPaginas-1);
			}else{
				len = (uint32_t)sprintf (buf, &env[1], numMaxPaginas);
			}
    break;
		
		case 'z': //primera fila de la tabla
			switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[1], Personas[0].nombre);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[1], Personas[0].apellido);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[1], Personas[0].DNI);
          break;
        case '4':
          if(strcmp(Personas[0].estaDentro, "1") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "Si");
          }
					if(strcmp(Personas[0].estaDentro, "0") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "No");
          }
					if(strcmp(Personas[0].estaDentro, "") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "");
          }
          break;
				}
		break;
				
		case 'm': //segunda fila de la tabla
			switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[1], Personas[1].nombre);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[1], Personas[1].apellido);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[1], Personas[1].DNI);
          break;
        case '4':
          if(strcmp(Personas[1].estaDentro, "1") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "Si");
          }
					if(strcmp(Personas[1].estaDentro, "0") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "No");
          }
					if(strcmp(Personas[1].estaDentro, "") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "");
          }
          break;
				}
		break;
				
		case 'n': //tercera fila de la tabla
			switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[1], Personas[2].nombre);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[1], Personas[2].apellido);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[1], Personas[2].DNI);
          break;
        case '4':
          if(strcmp(Personas[2].estaDentro, "1") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "Si");
          }
					if(strcmp(Personas[2].estaDentro, "0") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "No");
          }
					if(strcmp(Personas[2].estaDentro, "") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "");
          }
          break;
				}
		break;
				
		case 'o': //cuarta fila de la tabla
			switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[1], Personas[3].nombre);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[1], Personas[3].apellido);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[1], Personas[3].DNI);
          break;
        case '4':
          if(strcmp(Personas[3].estaDentro, "1") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "Si");
          }
					if(strcmp(Personas[3].estaDentro, "0") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "No");
          }
					if(strcmp(Personas[3].estaDentro, "") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "");
          }
          break;
				}
		break;
				
		case 'q': //quinta fila de la tabla
			switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[1], Personas[4].nombre);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[1], Personas[4].apellido);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[1], Personas[4].DNI);
          break;
        case '4':
          if(strcmp(Personas[4].estaDentro, "1") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "Si");
          }
					if(strcmp(Personas[4].estaDentro, "0") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "No");
          }
					if(strcmp(Personas[4].estaDentro, "") == 0){
            len = (uint32_t)sprintf (buf, &env[1], "");
          }
          break;
				}
		break;

		
  }
  return (len); // devuelve la longitud del mensaje
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
