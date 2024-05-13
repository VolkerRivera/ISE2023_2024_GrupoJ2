#include "rtc.h"
#include "main.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>
#include <string.h>


RTC_HandleTypeDef RtcHandle;
RTC_DateTypeDef sdatestructure;
RTC_TimeTypeDef stimestructure;
RTC_AlarmTypeDef alarmRtc;

extern osThreadId_t tid_ThLCD;

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  /*##-2- Configure LSE as RTC clock source ###################################*/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  { 
    //Error_Handler();
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    //Error_Handler();
  }
  
  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

//Función para la inicialización del RTC y los valores iniciales de este
void init_RTC(void){
  
  HAL_RTC_MspInit(&RtcHandle);
  
  //HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x00);
	
	//Inicio valores RTC
//	uint8_t horas = 23;
//	uint8_t minutos = 59;
//	uint8_t segundos = 30;
//	uint8_t dia = 31;
//	uint8_t nombre_dia = RTC_WEEKDAY_TUESDAY;
//	uint8_t mes = 12;
//	uint8_t ano = 99;
	
	__HAL_RCC_PWR_CLK_ENABLE(); //Habilita el reloj del módulo de administración de energía (PWR).
	HAL_PWR_EnableBkUpAccess(); //Habilita el acceso al dominio de backup para permitir la configuración del RTC.
  __HAL_RCC_RTC_ENABLE(); //Habilita el reloj del RTC.
	
	RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24; //Formato de la hora
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV; //Predivisor Asíncrono
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV; //Predivisor síncrono
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK){
		printf("ERROR: No se ha podido iniciar el RTC\n\r");
  }

}

//Función para configurar los valores del RTC
void set_dia_hora(uint8_t horas, uint8_t minutos, uint8_t segundos, uint8_t dia, uint8_t nombre_dia, uint8_t mes, uint8_t ano){
	
	//Configuración del día, mes y año
	sdatestructure.Year = ano;
  sdatestructure.Month = mes;
  sdatestructure.Date = dia;
  sdatestructure.WeekDay = nombre_dia;
	if (HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN) != HAL_OK)
  {
    printf("ERROR: No se ha establecido correctamente la fecha\n\r");
	}
	
	//Configuración de la hora, minutos y segundos
	stimestructure.Hours = horas;
  stimestructure.Minutes = minutos;
  stimestructure.Seconds = segundos;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
  {
    printf("ERROR: No se ha establecido correctamente la hora\n\r");
	}

}

//Función para obtener el tiempo y la fecha
tiempoyfecha get_tiempo_fecha(void){
	
	tiempoyfecha tiempo_fecha;
	//Obtención tiempo
	HAL_RTC_GetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN);
	sprintf(tiempo_fecha.tiempo,"%.2d:%.2d:%.2d",stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
	//Obtención fecha
	HAL_RTC_GetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN);
	sprintf(tiempo_fecha.fecha,"%02d/%02d/%02d",sdatestructure.Date,sdatestructure.Month,sdatestructure.Year);
	return tiempo_fecha;
}

void set_alarm(uint8_t segundos, uint8_t minutos, uint8_t horas){
	
	HAL_RTC_GetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN);
	
  uint8_t date = sdatestructure.Date;
  uint8_t sec_alarm=stimestructure.Seconds+segundos;
	uint8_t mins_alarm=stimestructure.Minutes+minutos; //Alarma en el siguiente minuto
	uint8_t horas_alarm=stimestructure.Hours+horas;
		
  if(sec_alarm>59){
    sec_alarm=sec_alarm-60;
    mins_alarm++;
  }
	
  if(mins_alarm>59){
    mins_alarm=mins_alarm-60;
    horas_alarm++;
  }
  
	alarmRtc.AlarmTime.Hours = horas_alarm;
  alarmRtc.AlarmTime.Minutes = mins_alarm;
  alarmRtc.AlarmTime.Seconds = sec_alarm;
  alarmRtc.AlarmTime.SubSeconds = 0x0;
  alarmRtc.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  alarmRtc.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  alarmRtc.AlarmMask =RTC_ALARMMASK_HOURS | RTC_ALARMMASK_DATEWEEKDAY;
  alarmRtc.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  alarmRtc.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  alarmRtc.AlarmDateWeekDay = date;
  alarmRtc.Alarm = RTC_ALARM_A;//Tipo de Alrma A
  HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);//Configuramos la prioridad de la interrupción del RTC
  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);//Habilitamos la interrupción por RTC
	
	if (HAL_RTC_SetAlarm_IT(&RtcHandle, &alarmRtc, RTC_FORMAT_BIN) != HAL_OK) {
		printf("ERROR: No se ha establecido la alarma correctamente\n\r");
  }
}

void RTC_Alarm_IRQHandler(void) {
  HAL_RTC_AlarmIRQHandler(&RtcHandle);//Configuramos nuestra interrupción para nuestro manejador
	
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
  if(hrtc->Instance==RTC){
		osThreadFlagsSet(tid_ThLCD, FLAG_THLCD);
    set_alarm(1,0,0);
  }
}
