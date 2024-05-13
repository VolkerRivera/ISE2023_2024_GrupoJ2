#include "stm32f4xx_hal.h"
#include <string.h>
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "sntp.h"
#include "rtc.h"
#include "rl_net.h"
#include <time.h>
#include <stdio.h>

int status = 0;
time_t     now;
//const  NET_ADDR4 ntp_server = { NET_ADDR_IP4 , 0, 217, 79, 179, 106 };
static void time_callback (uint32_t seconds, uint32_t seconds_fraction);

void get_time (void) {
  if (netSNTPc_GetTime (NULL, time_callback) == netOK) {
    printf ("SNTP request sent.\n");
		status=1;
  }
  else {
		status=2;
    printf ("SNTP not ready or bad parameters.\n");
  }
}

static void time_callback (uint32_t seconds, uint32_t seconds_fraction) {
  if (seconds == 0) {
		status=3;
    printf ("Server not responding or bad response.\n");
  }
  else {
		status=4;
    printf ("%d seconds elapsed since 1.1.1970\n", seconds);
		set_fecha_hora_ntp(seconds);
  }
}

void set_fecha_hora_ntp(uint32_t segundos){
	
	now=segundos+3600+3600;
  struct tm  ts;
  char       buf[80];

  // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
  ts = *localtime(&now);
  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
  //printf("%s\n", buf);
	
	uint8_t anos= ts.tm_year-100; //years since 1900
	uint8_t mes= ts.tm_mon+1; //	months since January
	set_dia_hora(ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_mday, ts.tm_wday, mes, anos);
  printf ("HORA SINCRONIZADA\n");
	
}