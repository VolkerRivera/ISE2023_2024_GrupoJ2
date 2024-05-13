#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file

typedef struct {
	uint8_t ano;
	uint8_t mes;
	uint8_t dias;
	uint8_t horas;
	uint8_t minutos;
	uint8_t segundos;
}ntp;

void get_time (void);
void set_fecha_hora_ntp(uint32_t segundos);
