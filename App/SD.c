#include "SD.h"


	/*	Drive					File System				 Drive Type
		"F:" or "F0:"				EFS					NOR Flash Drive 0
		"F1:" 							EFS					NOR Flash Drive 1
		"R:" or "R0:"				FAT					RAM Drive 
		"M:" or "M0:"				FAT					Memory Card Drive 0 << Selected in congif wizard
		"M1:"								FAT					Memory Card Drive 1
		"U:" or "U0:"				FAT					USB Memory Stick  0
		"U1:"       				FAT					USB Memory Stick  1
		"N:" or "N0:"				FAT					NAND Flash Drive 0
		"N1:" 							FAT					NAND Flash Drive 1
	*/

	/* Mounting means that it will be checked if the drive's media is attached 
		 and formatted. After a successful mount, drive is ready for file I/O operations*/
	
	// Update a log file on SD card.
	/*
	r: Read mode. Opens a file for reading.
	w: Write mode. Opens an empty file for writing. If the file exists, 
		 the content is destroyed. If the file does not exist, an empty file is opened for writing.
	a: Append mode. Opens a file for appending text. If the file already exists, data is appended. 
		 If the file does not exist, an empty file is opened for writing. Opening a file with append 
		 mode causes all subsequent writes to be placed at the then current end-of-file, regardless 
		 of interfering calls to fseek.
	b: Binary mode. Can be appended to any character above, but has no effect. The character is 
		 allowed for ISO C standard conformance.
	+: Update mode. Can be used with any character above as a second or third character. When a 
		 file is opened with update mode, both reading and writing can be performed. Programmers 
		 must ensure that reading is not directly followed by writing without an interfering call to 
		 fflush or to a file positioning function (fseek or rewind). Also, writing should not be 
		 followed directly by reading without an interfering call to a file positioning function, 
		 unless the writing operation encounters EOF.
	*/

	
/*
	Si esta funcion se llama dentro de otra funcion que monta y desmonta la unidad M:, 
	no hace falta hacerlo aqui. Si se llama individualmente si.
	return code
	0: error: no se ha cerrado bien el fichero tras escribir
	1: todo ok
	4: error file not found
*/

uint8_t registerPerson(char timestamp[], char DNI[], char InOut[]){ 
	printf("Se va a añadir un registro de entrada/salida.\n");
	fflush(stdout);
	const uint8_t size = 128; //Checkear la longitud maxima de cada linea
	char buffer [size];
	FILE* registerfile;
	uint8_t code = 0;
	
	registerfile = fopen ("M:\\REG_INOUT.TXT","a+"); //no puede ser r a no ser que el archivo YA este creado
	
	

	snprintf(buffer, size, "%s -> %s -> %s \n", timestamp, DNI, InOut);
	rewind(registerfile); //rewind solo funcionara con un a+ file para leer, pero nunca para escribir ya que esta implicito que a (append) es para escribir al final hagas rewinf o fseek
	fprintf(registerfile,"%s", buffer);
	fflush(registerfile);
	
	if(fclose(registerfile) == 0){
		code = 1;
	}
	
	return code;
}

uint8_t mount_unmount(bool mount){ // mount = true -> mount and init ;;;; mount = false -> unmount and unitit
	if(mount){
		if (finit ("M:") != fsOK) {
			//Error handling
			printf("No se ha podido inicializar la microSD.\n");
			fflush(stdout);
			return 2;
		}
		if (fmount ("M:") != fsOK) {
			//Error handling
			printf("No se ha podido montar la microSD.\n");
			fflush(stdout);
			funinit ("M:");
			return 3;
		}
	}else{
		funmount("M:");
    funinit("M:");
	}
	//if succesfully mount and init:
	return 0; //default code
}	


/* Añade un nuevo vecino si este aun no esta registrado
	Devuelve code:

	0: Vecino no añadido porque ya existe
	1: Vecino añadido correctamente
	2: Error al inicializar el disco M:
	3: Error al montar el disco M:
	4: Error al abrir el fichero REGVECINOS.TXT

*/

uint8_t addVecino(identificacion vecino){
	printf("Se va a añadir a un vecino.\n");
	fflush(stdout);
	const uint8_t size = 128; //Checkear la longitud maxima de cada linea
	char buffer [size];
	bool Existe = false;
	uint8_t code = 0;
	FILE *f; 

	code = mount_unmount(true); //se intenta montar e iniciar, si todo ok se mantiene code = 0
	
	f = fopen ("M:\\REGVECINOS.TXT","a+");
	
  if (f == NULL) {
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
  }
  else {//Si se encuentra el archivo
		rewind(f); //En caso de que se vuelva a abrir el archivo, empiece a leerse desde el principio siempre, no solo al crearlo.
		//Check if already exists
		while(fgets(buffer, sizeof(buffer), f) != NULL){
			if ((strstr(buffer, vecino.codigoPIN) != NULL) && (strstr(buffer, vecino.DNI)!= NULL)) {
				Existe = true;
				printf("Ya existe ese vecino.\n");
				fflush(stdout);
				break;
			}
		}
		
    // Write data to file
		if(!Existe){
			fseek(f, 0, SEEK_END); // Para añadir el nuevo vecino en una nueva linea al final y no donde se haya quedado fgets
			fprintf(f, "%s,%s,%s,%s,%s,%s\n", vecino.codigoPIN, vecino.tag, vecino.nombre, vecino.apellido, vecino.DNI, vecino.estaDentro);
			fflush(f);
			printf("Se ha añadido a %s %s.\n", vecino.nombre, vecino.apellido);
			fflush(stdout);
			code = 1;
		  for(int i=0;i<3;i++){
      LED_On(0);
      osDelay(100);
      LED_Off(0);
      osDelay(100);
			}
		}
  }
	// Close file
	fclose (f);
	mount_unmount(false); //unmount y uninit

	return code;
}



/* Funcion que se encarga de eliminar un vecino de la base de datos. Primero evalua si dicha
persona existe mediante la asociacion correcta de codigoPIN y DNI. Si la persona que se pretende 
borrar no esta en la lista, no hay nada que borrar.

Para eliminar la linea no se borra directamente sino que se copia linea a linea del txt original
al txt temporal, a excepcion de la linea que contiene el codigoPIN y DNI asociados a la persona que 
se quiere borrar. Una vez se ha hecho esto, se borra el txt original y al txt temporal se le asigna 
el nombre del otro, pasando a ser este el txt original de la proxima vez que se quiera eliminar un
vecino de la BBDD.

return code:

0: Vecino que se quiere eliminar de la BBDD no se encuentra en ella.
1: Vecino se ha eliminado correctamente.
2: Error al inicializar el disco M:
3: Error al montar el disco M:
4: Error al abrir el fichero REGVECINOS
5: Vecino que se quiere eliminar se encuentra en la BBDD pero ha habido un error al eliminar sus datos
		- Error en fdelete: fdelete borra el txt que contiene el registro de vecinos original
		- Error en frename: frename renombra el txt temporal con el nombre del txt original, quedando 
			actualizado el fichero con todos los datos anteriores a excepcion del vecino que ha sido borrado
		
6: Vecino no se encuentra en la lista y ha habido un problema al eliminar el txt temporal

*/

uint8_t deleteVecino(char codigoPIN[], char DNI[]){
	printf("Se va a eliminar a un vecino.\n");
	fflush(stdout);
	char buffer [128];
	bool Existe = false;
	uint8_t code = 0;
	FILE *f, *fileTemporal;
	
	code = mount_unmount(true); //mount y init
	
	//a+ leo/escribo desde el final ;;; r+ escribo/leo desde el principio
	f = fopen ("M:\\REGVECINOS.TXT","a+");// si se pone "no permitira borrarlo luego", permite lectura desde cualquier parte del archivo
	fileTemporal = fopen("M:\\TEMP.TXT", "a+"); 
  
	if (f == NULL || fileTemporal == NULL) { // Si no se encuentra el archivo
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
		
  }else { //Si se encuentra el archivo
    rewind(f);
		//Checkeamos si existe dicho vecino
		while(fgets(buffer, sizeof(buffer), f) != NULL){
			if ((strstr(buffer, codigoPIN) != NULL) && (strstr(buffer, DNI) != NULL)) {
				Existe = true;
				
				printf("Existe ese vecino.\n"); //print ok
				fflush(stdout);
				rewind(f); //Vuelve a poner el puntero al inicio del archivo
				break;
			}
		}
		
		//Hasta aqui todo correcto porque el archivo REGVECINOS.TXT se crea, se escribe y se lee correctamente
		
		if(Existe){ //si existe lo podemos borrar
			// Actualizamos el fichero copiando todas las lineas a otro y renombrandolo, a excepcion de la linea correspondiente al vecino a borrar
			while (fgets(buffer, sizeof(buffer), f) != NULL) {
        // Si la línea no contiene el DNI y el PIN a borrar, se escribe en el archivo temporal
				printf("Buffer leido: %s, DNI: %s, PIN: %s \n",buffer, DNI, codigoPIN);
				fflush(stdout);
				//No copiamos unicamente cuando coincida DNI, que es lo que es unico.
				//Si evaluamos tambien el pin y se repite, dejara de cumplirse la condicion
        //if ((strstr(buffer, codigoPIN) == NULL) && (strstr(buffer, DNI) == NULL)) { // strstr  a null pointer if the sequence is not present in haystack
				if ((strstr(buffer, DNI) == NULL)) {
            //fputs(buffer, fileTemporal);
						fseek(fileTemporal, 0, SEEK_END); // Para añadir el nuevo vecino en una nueva linea al final y no donde se haya quedado fgets
						fprintf(fileTemporal, "%s", buffer);
						fflush(fileTemporal);
						printf("Buffer escrito en temp: %s\n", buffer);
						fflush(stdout);
        }
			}
			//Una vez copiado cerramos los archivos
			fflush(f);
			fflush(fileTemporal);
			fclose(f);
			fclose(fileTemporal);
			//Eliminamos el original
      
			if(fdelete("M:\\REGVECINOS.TXT", NULL) != fsOK){
				code = 5;
				mount_unmount(false); //unmount y uninit
        printf("Error 5: no se pudo eliminar REGVECINOS.TXT\n");
				fflush(stdout);
				return code;
			}
			if (frename ("M:\\TEMP.TXT", "REGVECINOS.TXT") != fsOK){
				code = 5;
				mount_unmount(false); //unmount y uninit
        printf("Error 5: no se pudo renombrar TEMP.TXT a REGVECINOS.TXT\n");
				fflush(stdout);
				return code;
			}
			printf("Vecino eliminado correctamente.\n");
			fflush(stdout);
			code = 1;
      for(int i=0;i<3;i++){
      LED_On(2);
      osDelay(100);
      LED_Off(2);
      osDelay(100);
			}
		}else{// Si el vecino no existe
			printf("Dicho vecino no está registrado.\n");
			/* Se cierran ambos archivos */
			fclose(f);
			fclose(fileTemporal);
			printf("Ficheros cerrados, borrando TEMP.TXT...\n");
			fflush(stdout);
			/* Eliminamos el archivo temporal*/
			if(fdelete("M:\\TEMP.TXT", NULL) != fsOK){
				code = 6;
				mount_unmount(false); //unmount y uninit
				return code;
			}
			
			code = 0;
		}
	
	}
	mount_unmount(false); //unmount y uninit
	
	return code;
}



/* Evalua el permiso de acceso comprobando si el codigoPIN se encuentra en el Registro de Vecinos.
	Devuelve code:

	0: Acceso no permitido
	1: Acceso permitido
	2: Error al inicializar el disco M:
	3: Error al montar el disco M:
	4: Error al abrir el fichero REGVECINOS.TXT
*/

uint8_t tieneAcceso(char codigoPIN[], char ID[]){ //codigoPIN es unico para cada persona
	printf("Comprobando permiso de acceso...\n");
	fflush(stdout);
	FILE *f;
	uint8_t code = 0;
	char buffer[128]; //< Buffer donde se almacenan los 128 caracteres leidos
	
	code = mount_unmount(true); //mount y init
	
	f = fopen ("M:\\REGVECINOS.TXT","r"); //< Puesto que unicamente vamos a leer este archivo
	
  if (f == NULL) {
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
  }
  // Comprobacion validez del Codigo PIN
	/* fgets lee hasta que se hayan leido los 128 caracteres, hasta que se cambie de linea o 
	hasta final del archivo => En ningun momento el codigoPIN de f se quedara a medias.*/
	while(fgets(buffer, sizeof(buffer), f) != NULL){ //Mientras no se llegue al final del archivo se leen 128 caracteres de f y se almacenan en buffer
	//Cada vez que se lee una linea:
    //char *token = strtok(buffer, ","); // Utiliza la coma como delimitador para dividir la cadena
    if((strstr(buffer, codigoPIN) != NULL) && (strstr(buffer, ID)!= NULL)){ //Si se encuentra en buffer dicha cadena
    //TODO: con strs verificar que SOLO reconozca el codigo. Ejemplo:
    // Ahora mismo si quiere coomprobar el codigo 111B pero hay una persona registrada con codigo 983A pero DNI 51023111B, tambien va a devolver 1
    //if (token != NULL && strcmp(token, codigoPIN) == 0) { // Compara el primer token con el código PIN  
      code = 1;
      for(int i=0;i<3;i++){
      LED_On(1);
      osDelay(200);
      LED_Off(1);
      osDelay(200);
			}
      break; // Sale del bucle en cuanto lo encuentra.
      }
    }
  
	// Close file
  fclose (f);
  mount_unmount(false); //unmount y uninit
	
	//if(code == 1) printf("Tiene acceso.\n");
	fflush(stdout);
	return code;
}

uint8_t tieneAccesoRFID(char tag[], char IDVer[]){
	
	printf("Se ha leido este ID: %s y este tag: %s\n", IDVer, tag);
	fflush(stdout);
	FILE *f;
	uint8_t code = 0;
	char buffer[128]; //< Buffer donde se almacenan los 128 caracteres leidos
	identificacion persona;
	
	code = mount_unmount(true); //mount y init
	
	f = fopen ("M:\\REGVECINOS.TXT","r"); //< Puesto que unicamente vamos a leer este archivo
	
  if (f == NULL) {
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
  }
  // Comprobacion validez del Codigo PIN
	/* fgets lee hasta que se hayan leido los 128 caracteres, hasta que se cambie de linea o 
	hasta final del archivo => En ningun momento el codigoPIN de f se quedara a medias.*/
	while(fgets(buffer, sizeof(buffer), f) != NULL){ //Mientras no se llegue al final del archivo se leen 128 caracteres de f y se almacenan en buffer
	//Cada vez que se lee una linea:
    //char *token = strtok(buffer, ","); // Utiliza la coma como delimitador para dividir la cadena
    if((strstr(buffer, tag) != NULL)){ //Si se encuentra en buffer dicha cadena
    //TODO: con strs verificar que SOLO reconozca el codigo. Ejemplo:
    // Ahora mismo si quiere coomprobar el codigo 111B pero hay una persona registrada con codigo 983A pero DNI 51023111B, tambien va a devolver 1
    //if (token != NULL && strcmp(token, codigoPIN) == 0) { // Compara el primer token con el código PIN  
      sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]", persona.codigoPIN, persona.tag,persona.nombre, persona.apellido, persona.DNI, persona.estaDentro);
			strcpy(IDVer, persona.DNI);
			code = 1;
      for(int i=0;i<3;i++){
      LED_On(1);
      osDelay(100);
      LED_Off(1);
      osDelay(100);
			}
      break; // Sale del bucle en cuanto lo encuentra.
      }
    }
  
	// Close file
  fclose (f);
  mount_unmount(false); //unmount y uninit
	
	//if(code == 1) printf("Tiene acceso.\n");
	fflush(stdout);
	return code;
	
}




/*Funcion que permite saber si una persona se encuentra dentro del edificio.
	cambiarEstado = false cuando solo se quiere hacer una consulta, timestamp not used
	cambiarEstado = true  cuando se quiere dejar registro de esta consulta. Cuando alguien entra/sale del edificio. timestamp used

	0: no está, pero tiene acceso
	1: está y tiene acceso
	2,3,4: error handling file
	5: error cambiando estado
	6: code not modified
*/

uint8_t estaDentro(char timestamp[], char DNI[], bool cambiarEstado){
	printf("Comprobando si dicha persona se encuentra dentro del edificio...\n");
	fflush(stdout);
	FILE *f, *fileTemporal;
	uint8_t code = 6;
	char isInside;
	char buffer[128]; //< Buffer donde se almacenan los 128 caracteres leidos
	
	code = mount_unmount(true); //mount y init
	code = (code == 0) ? 6 : code; //si el valor que devuelve mount_unmout es 0, todo ok asi que continua code = 6, 
																 //si devuelve otro valor significa que hubo error al hacer mount o init, asi que adquiere el codigo del error
	
	f = fopen ("M:\\REGVECINOS.TXT","r+"); //< Puesto que unicamente vamos a leer este archivo
	
	if(cambiarEstado){ //solo se crea un fichero si es necesario
		fileTemporal = fopen ("M:\\TEMP.TXT","a+");
		if (fileTemporal == NULL) {
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
		}
	}
	
  if (f == NULL) {
    //Error handling
    printf("File not found!\n"); 
		fflush(stdout);
		mount_unmount(false); //unmount y uninit
		code = 4;
		return code;
  }
	
	while(fgets(buffer, sizeof(buffer), f) != NULL){
		if(strstr(buffer, DNI) != NULL){ //Si se encuentra el DNI en esa linea
			
			size_t length = strlen(buffer); //longitud de la linea
			isInside = buffer[length - 2]; // El índice length - 2 es el último carácter antes del '\n'
			if(isInside == '1'){ //Si se lee 1 quiere decir que la persona esta dentro
				
				if(cambiarEstado){//Si se debe cambiar estado
					//Hemos detectado que esta dentro, asi que cambiamos a que ya no esta y devolvemos esto ultimo
					buffer[length - 2] = '0'; //editamos el buffer que almacena la linea leida
					printf("Estaba dentro del edificio pero ha salido.\n");
					fflush(stdout);
					code = 0;
					registerPerson(timestamp, DNI, "Ha salido");//Once the drive is successfully initialized, all subsequent calls 
																											//of finit will return execution status fsOK, leaving internal drive state unchanged.
				}else{//si no se deve cambiar estado
					printf("Esta dentro del edificio.\n");
					fflush(stdout);
					code = 1;
					break; //salimos del while si SOLO queremos consultar si la persona esta dentro o no.
				}
				
				
			}else{ // La persona no esta dentro
				
				if(cambiarEstado){
					buffer[length - 2] = '1'; //editamos el buffer que almacena la linea leida
					printf("Estaba fuera del edificio pero ha entrado.\n");
					fflush(stdout);
					code = 1;
					registerPerson(timestamp, DNI, "Ha entrado");
				}else{
					printf("No esta dentro del edificio.\n");
					fflush(stdout);
					code = 0;
					break;
				} 
				
				
			}
		}
		
		//SOLO si cambiarEstado = true, vamos copiando en el archivo temporal
		if(cambiarEstado){
			fseek(fileTemporal, 0, SEEK_END);
			fprintf(fileTemporal, "%s", buffer);
			fflush(fileTemporal);
		}
	}
	
	fclose(f); //se cierra si o si porque esta abierto si o si, independientemente del valor de cambiarEstado
	
	if(cambiarEstado){
		//Se cierran ambos ficheros
		fclose(fileTemporal);
		//Se elimina f y se renombra fileTemporal con el nombre de f
		      
		if(fdelete("M:\\REGVECINOS.TXT", NULL) != fsOK){
			code = 5;
			mount_unmount(false); //unmount y uninit
      printf("Error 5: no se pudo eliminar REGVECINOS.TXT\n");
			fflush(stdout);
			return code;
		}
		if (frename ("M:\\TEMP.TXT", "REGVECINOS.TXT") != fsOK){
			code = 5;
			mount_unmount(false); //unmount y uninit
      printf("Error 5: no se pudo renombrar TEMP.TXT a REGVECINOS.TXT\n");
			fflush(stdout);
			return code;
		}
	}
	
	mount_unmount(false); //unmount y uninit
  return code;
}

uint8_t cuanta_gente (void){ //mientras se consulta esto no se puede añadir a gente asi que siempre devolvera un valor real
	uint8_t contador = 0;
	char linea[128]; // Asumiendo que ninguna línea excede los 128 caracteres
	FILE *f;
	mount_unmount(true);
	
	f = fopen ("M:\\REGVECINOS.TXT","r"); //leeremos linea por linea
	if (f == NULL) {
        printf("Error al abrir el archivo.\n");
				fflush(stdout);
				//return 0; asignar un valor para el codigo de errores
    }
	
		while (fgets(linea, sizeof(linea), f)) {
        // Verifica si la línea no está vacía (ignorando posibles líneas solo con '\n')
        if (linea[0] != '\n' && linea[0] != '\0') {
            contador++;
        }
    }
		
	fclose(f); // Cierra el archivo
	
	mount_unmount(false);
	return contador; // Retorna el número de líneas con información
}



void getPersonasFromThisLine(uint8_t numeroLinea, identificacion Personas[5]){
	FILE *f;
	static char linea[128];
	uint16_t contador = 0;
	uint16_t index = 0;
	mount_unmount(true);
	
	f = fopen("M:\\REGVECINOS.TXT", "r"); // Abre el archivo en modo de lectura
    if (f == NULL) {
      printf("Error al abrir el archivo");
			fflush(stdout);
    }
    
		//contador empieza en 0 y linea es la 5 por lo que no entrara , asi que hace else y sale deleteVecino bucle sin seguir leyendo
	while (fgets(linea, sizeof(linea), f)) { //buscamos la linea
    if (contador >= numeroLinea && index < 5) { //aumentamos contador hasta encontrar la linea,cuando hacen match:
			printf("Linea: %s, Contandor:%d \n",linea, contador);
			fflush(stdout);
      Personas[index] = getPersona(linea);
      //fclose(f); // Cierra el archivo
			//mount_unmount(false);
      //return; //linea; // Retorna la línea deseada
			index++;
			if(index == 5) break;
		}
		contador++;
  }
	fclose(f); // Cierra el archivo
	mount_unmount(false);
	return;// "Empty"; //RETORNA "Empty" SI LA LINEA NO EXISTE
}

identificacion getPersona(char linea_n[]){
  identificacion persona;
  memset(&persona, 0, sizeof(persona)); //formatea los componentes de la estructura
  if(strcmp(linea_n, "Empty") == 0){
    strcpy(persona.codigoPIN, "");
		strcpy(persona.tag, "");
    strcpy(persona.nombre, "");
    strcpy(persona.apellido, "");
    strcpy(persona.DNI, "");
    strcpy(persona.estaDentro, "");
    return persona; //retornamos struct con campos vacios
  }
  sscanf(linea_n, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]", persona.codigoPIN, persona.tag,persona.nombre, persona.apellido, persona.DNI, persona.estaDentro);
  return persona; //estaDentro = 0
}
