#ifndef __LCD_H
#define __LCD_H

#include  "Driver_SPI.h"
extern ARM_DRIVER_SPI Driver_SPI1;
extern unsigned char buffer[512];
void delay(uint32_t n_microsegundos);/*No podemos usar el Delay() porque funciona con el
SysTick, el cual va con ticks de 1ms, no podria hacer el de 1us*/
void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init(void);
void LCD_update(void);
void LCD_symbolToLocalBuffer_L1(uint8_t symbol);
void LCD_symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);//basicamente elige en que linea se escribira el simbolo del mensaje
void write(uint8_t line, char* msg);
void erase_screen(void);

#endif /* _LCD_H*/

