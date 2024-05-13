#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "tim.h"
#include "principal.h"

/* FROM TIM */
extern TIM_HandleTypeDef htim3;

void cambiar_frec500(void){
	
	htim3.Init.Period = 1999;
	HAL_TIM_Base_Init(&htim3);
	
}

void cambiar_frec200(void){
	
	htim3.Init.Period = 4999;
	HAL_TIM_Base_Init(&htim3);
	
}

void cambiar_frec100(void){
	
	htim3.Init.Period = 9999;
	HAL_TIM_Base_Init(&htim3);
	
}

uint8_t value = 0;  // the value for the duty cycle

void tono(int delay, int maxvalor, int incremento){
	
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	
	while (value<maxvalor){
		htim3.Instance->CCR1 = value;  // vary the duty cycle
		value += incremento;  // increase the duty cycle
		osDelay (delay);  // wait for x ms
	}

	value = 0;
	
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);

}

void tono_dec(int delay, int minvalor, int decremento, int inicial){
	
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	
	while (inicial>minvalor){
		htim3.Instance->CCR1 = inicial;  // vary the duty cycle
		inicial -= decremento;  // decrease the duty cycle
		osDelay (delay);  // wait for x ms
	}

	value = 0;
	
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);

}

void tono_tecla(void){
	
	cambiar_frec200();
	tono(300, 50, 25);
	
}

void tono_entrada(void){
	
	cambiar_frec500();
	tono(100, 100, 10);
	tono_dec(100, 1, 10, 100);
	
}

void tono_fallo(void){
	
	cambiar_frec100();
	tono(100, 50, 25);
	osDelay(200);
	tono(100, 50, 25);
	osDelay(200);
	tono(100, 50, 25);
	osDelay(200);
	
}
