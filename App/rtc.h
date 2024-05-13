#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file


#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

typedef struct{
	char tiempo[35];
	char fecha[35];
}tiempoyfecha;

void init_RTC(void);
void set_dia_hora(uint8_t horas, uint8_t minutos, uint8_t segundos, uint8_t dia, uint8_t nombre_dia, uint8_t mes, uint8_t ano);
tiempoyfecha get_tiempo_fecha(void);
void set_alarm(uint8_t segundos, uint8_t minutos, uint8_t horas);

static void Timer500_Callback (void const *arg);
int Init_Timer(void);
void RTC_Alarm_IRQHandler(void);
