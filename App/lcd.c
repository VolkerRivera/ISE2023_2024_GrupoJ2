#include "main.h"
#include "Driver_SPI.h" /*Para poder manejar el SPI. "Driver_XXINTERFACEXX.h"*/
#include "Arial12x12.h"
#include "string.h"
#include "stdio.h"
#include "lcd.h"

ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
ARM_SPI_STATUS stat;
unsigned char buffer[512];
static uint32_t positionL1 = 0;
static uint32_t positionL2 = 0;

	void LCD_reset(void){
		
		static GPIO_InitTypeDef GPIO_InitStructure;
		
		/*Tendremos MOSI->PA7 ; RESET->PA6 ; SCK->PA5 ; A0->PF13 ; CS->PD14*/
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		
		/*Inicializamos el SPI*/
		
		SPIdrv->Initialize(NULL);
		SPIdrv->PowerControl(ARM_POWER_FULL);//nos da igual el consumo
		SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);
	
		/*Inicializamos los pines*/

		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		
		GPIO_InitStructure.Pin = GPIO_PIN_6;//RESET
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = GPIO_PIN_14;//CS
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = GPIO_PIN_12;//A0
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
		
		/*MODELAMOS EL RESET INICIAL*/
		
		HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6 , GPIO_PIN_RESET);
		delay(1);//Esperamos durante 1 us
		HAL_GPIO_WritePin (GPIOA, GPIO_PIN_6 , GPIO_PIN_SET);
		
		
		delay(1000);//Esperamos durante 1 ms
		//Pasado este tiempo ya podria realizarse la primera transaccion
		
	}
	
	void delay(uint32_t n_microsegundos){
		static TIM_HandleTypeDef htim7;
		__HAL_RCC_TIM7_CLK_ENABLE();
		__HAL_TIM_SET_COUNTER(&htim7,0);
		/*Timer7 pertenece a la APB1, le llegaran 84MHz*/
		htim7.Instance = TIM7;
		htim7.Init.Prescaler = 83; // 84MHz/84 = 1MHz => tics cada 1 us
		htim7.Init.Period = n_microsegundos-1;// 
		
		HAL_TIM_Base_Init(&htim7);
		HAL_TIM_Base_Start(&htim7);

		while(TIM7->CNT < n_microsegundos-1){
			/*Mientras no haya pasado el tiempo asignado, 
			se mantendra dentro del bucle*/
		}
		HAL_TIM_Base_Stop(&htim7);// Paramos el contador
		__HAL_TIM_SET_COUNTER(&htim7,0);// Ponemos el estado de cuenta a 0
	}	
	
  void LCD_wr_data(unsigned char data){
		//Nos servira para checkear el estado de la comunicacion
		
		//Seleccionar CS = 0 : Cuando baja el CS empieza la comunicacion
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		
		//Seleccionar A0 = 1
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_SET);
		
		//Escribir un dato (data) usando la funcion SPIdrv->Send()
		SPIdrv->Send(&data,sizeof(data));
		
		//Esperar a que se libere el bus SPI (comprobar estado)
		do{
			stat = SPIdrv->GetStatus();
			
		}while(stat.busy); /*busy = 1 cuando esta ocupado; */
		
		//Seleccionar CS = 1
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
		
	}
	
	void LCD_wr_cmd(unsigned char cmd){
		//Nos servira para checkear el estado de la comunicacion
	
		//Seleccionar CS = 0
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	
		//Seleccionar A0 = 0
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_RESET);
	
		//Escrbir un comando (cmd) usando la funcion SPIdrv->Send()
		SPIdrv->Send(&cmd,sizeof(cmd));

		//Esperar a que se libere el bus SPI
		do{
			stat = SPIdrv->GetStatus();

		}while(stat.busy);
	
		//Seleccionar CS = 1
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
		
}
	
	void LCD_init(void){
		LCD_wr_cmd(0xAE);
		LCD_wr_cmd(0xA2);
		LCD_wr_cmd(0xA0);
		LCD_wr_cmd(0xC8);
		LCD_wr_cmd(0x22);
		LCD_wr_cmd(0x2F);
		LCD_wr_cmd(0x40);
		LCD_wr_cmd(0xAF);
		LCD_wr_cmd(0x81);
		LCD_wr_cmd(0x17);//contraste en hexadecimal
		LCD_wr_cmd(0xA4);
		LCD_wr_cmd(0xA6);
	}
	
	void LCD_update(void){
			
		
		int i;

			LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
			LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
			LCD_wr_cmd(0xB0); // Página 0
			for(i=0;i<128;i++){
				LCD_wr_data(buffer[i]);
			}
			LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
			LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
			LCD_wr_cmd(0xB1); // Página 1
			for(i=128;i<256;i++){
				LCD_wr_data(buffer[i]);
			}
			LCD_wr_cmd(0x00);
			LCD_wr_cmd(0x10);
			LCD_wr_cmd(0xB2); //Página 2
			for(i=256;i<384;i++){
				LCD_wr_data(buffer[i]);
			}
			LCD_wr_cmd(0x00);
			LCD_wr_cmd(0x10);
			LCD_wr_cmd(0xB3); // Pagina 3
			for(i=384;i<512;i++){
				LCD_wr_data(buffer[i]);
			}
	}
	
	void LCD_symbolToLocalBuffer_L1(uint8_t symbol){
		uint8_t i, value1, value2;
		uint16_t offset = 0;
		//static uint32_t positionL1 = 0;
		
		offset=25*(symbol - ' ');
		
		for (i=0; i<12; i++){
			
			value1 = Arial12x12[offset+i*2+1];
			value2 = Arial12x12[offset+i*2+2];
			
			buffer[i+positionL1] = value1;//primera pagina
			buffer[i+128+positionL1] = value2;//segunda pagina?
		
		}
		positionL1=positionL1+Arial12x12[offset];
	}
	
	void LCD_symbolToLocalBuffer_L2(uint8_t symbol){
		uint8_t i, value1, value2;
		uint16_t offset = 0;
		//static uint32_t positionL2 = 0;
		
		offset=25*(symbol - ' ');
		
		for (i=0; i<12; i++){
			
			value1 = Arial12x12[offset+i*2+1];
			value2 = Arial12x12[offset+i*2+2];
			
			buffer[i+256+positionL2] = value1;//tercera pagina teoricamente
			buffer[i+384+positionL2] = value2;//cuarta pagina?
		
		}
		positionL2=positionL2+Arial12x12[offset];
	}
	
	void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
		if(line==1){
			LCD_symbolToLocalBuffer_L1(symbol);
			}else if(line==2){
			LCD_symbolToLocalBuffer_L2(symbol);
		}
	}
	
	void write(uint8_t line, char* msg){
		int i;
		int posc_letra = strlen(msg);
		
		for (i=0; i < posc_letra; i++){
		 symbolToLocalBuffer(line, msg[i]);//imprimira letra por letra en la linea que queramos segun su posicion
		}
		LCD_update();
		positionL1=0;//para que comience siempre a escribir la palabra al principio de la linea
		positionL2=0;
	}
	
	void erase_screen(void){
		memset(buffer,0x00,512);//resetea el buffer -> pantalla en blanco
		LCD_update();
		
		/*en lugar de meter manualmente el msg, se puede utilizar el la funcion sprintf() para escribir en un array de char y luego printear eso*/
	/*Asi se tendria solo una variable que va cambiando, pero bueno, proj pequenio asi que da un poco igual*/
	/*Util para pasar de uint a cadena de caracteres*/
	}
