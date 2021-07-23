/*
 * CLCD.h
 *
 * Created: 2019-10-09 오후 3:24:22
 *  Author: gigas
 */ 


#ifndef CLCD_H_
#define CLCD_H_

#define PORT_DATA      PORTF
#define PORT_CONTROL   PORTB
#define DDR_DATA       DDRF
#define DDR_CONTROL    DDRB
#define RS_PIN   0
#define RW_PIN   1
#define E_PIN    2
#define COMMAND_CLEAR_DISPLAY    0X01
#define COMMAND_8_BIT_MODE       0X38
#define COMMAND_DISPLAY_ON_OFF_BIT   2
#define COMMAND_CUSOR_ON_OFF_BIT     1
#define COMMAND_BLINK_ON_OFF_BIT     0
#define LCD_SETCGRAMDDR             0x40


void LCD_pulse_enable(void);
void LCD_write_data(uint8_t data);
void LCD_write_command(uint8_t command);
void LCD_clear(void);
void LCD_init(void);
void LCD_write_string(char *string);
void LCD_goto_XY(uint8_t row, uint8_t col);
void timer(int t[], int mode);
void setCLCD();
void tempCLCD(float temp);


#endif /* CLCD_H_ */