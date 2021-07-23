/*
 * CLCD.c
 *
 * Created: 2019-10-09 오후 3:24:03
 *  Author: gigas
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "CLCD.h"

void LCD_pulse_enable(void){                    // 하강에지에서 동작
	PORT_CONTROL |= (1<<E_PIN);
	_delay_ms(1);
	PORT_CONTROL &= ~(1<<E_PIN);
	_delay_ms(1);
}

void LCD_write_data(uint8_t data){             //문자 한글자 쓰기
	PORT_CONTROL |=(1<<RS_PIN);            //출력모드
	PORT_DATA = data;                      //문자데이터
	LCD_pulse_enable();                    // 출력실행
	_delay_ms(2);
}

void LCD_write_command(uint8_t command){
	PORT_CONTROL &= ~(1<<RS_PIN);           //명령어 실행모드
	PORT_DATA = command;                    //명령어 전달
	LCD_pulse_enable();                      //명령어 실행
	_delay_ms(2);
}

void LCD_clear(void){
	LCD_write_command(COMMAND_CLEAR_DISPLAY);
	_delay_ms(2);										// LCD 지우기
}

void LCD_init(void){
	_delay_ms(50);
	DDR_DATA=0xFF;													  // PORTD를 출력으로 설정
	PORT_DATA =0x00;
	DDR_CONTROL |= (1<<RS_PIN) | (1<<RW_PIN) | (1<<E_PIN);
	PORT_CONTROL &= ~(1<<RW_PIN);									  // 쓰기모드
	LCD_write_command(COMMAND_8_BIT_MODE);							  //8비트 모드
	uint8_t command = 0x08 | (1<<COMMAND_DISPLAY_ON_OFF_BIT);
	LCD_write_command(command);
	LCD_clear();
	LCD_write_command(0x06);
}

void LCD_write_string(char *string){
	uint8_t i;
	for(i=0;string[i];i++)
	LCD_write_data(string[i]);					 //문자열 출력
}

void LCD_goto_XY(uint8_t row, uint8_t col){
	col %= 16;
	row %= 2;
	uint8_t address = (0x40 * row) + col;
	uint8_t command = 0x80 + address;
	LCD_write_command(command);					//LCD 좌표설정
}

void timer(int t[], int mode) {
	t[0]++;
	if(t[0] == 58) {
		t[0] = 48;
		t[1]++;
	}
	if(t[1] == 54) {
		t[0] = 48;
		t[1] = 48;
		t[2]++;
	}
	if(t[2] == 58) {
		t[2] = 48;
		t[3]++;
	}
	if(t[3] == 54){
		t[2] = 48;
		t[3] = 48;
		t[4]++;
	}
	if(t[4]>=58){
		t[4]=48;
		t[5]++;
	}
	LCD_goto_XY(mode, 8);
	LCD_write_data(t[5]);
	LCD_goto_XY(mode, 9);
	LCD_write_data(t[4]);
	LCD_goto_XY(mode, 11);
	LCD_write_data(t[3]);
	LCD_goto_XY(mode, 12);
	LCD_write_data(t[2]);
	LCD_goto_XY(mode, 14);
	LCD_write_data(t[1]);
	LCD_goto_XY(mode, 15);
	LCD_write_data(t[0]);
}

void setCLCD() {
	LCD_goto_XY(1,8);
	LCD_write_string("00:00:00");
	LCD_goto_XY(0,8);
	LCD_write_string("00:00:00");
}

void tempCLCD(float temp) {
	char s1[10] = {0,};
	sprintf(s1, "%.1f", temp);
	if(temp == 0) {
		LCD_goto_XY(0, 0);
		LCD_write_string("      ");
		LCD_goto_XY(0, 0);
		LCD_write_string("0");
		LCD_goto_XY(0, 2);
		LCD_write_string("C");
	}
	else if(temp > 0 && temp < 10) {
		LCD_goto_XY(0, 0);
		LCD_write_string("      ");
		LCD_goto_XY(0, 2);
		LCD_write_string(s1);
		LCD_goto_XY(0, 4);
		LCD_write_string("C");
	}
	else if((temp < 0 && temp > -10) || (temp >= 10 && temp < 100)) {
		LCD_goto_XY(0, 0);
		LCD_write_string("      ");
		LCD_goto_XY(0, 0);
		LCD_write_string(s1);
		LCD_goto_XY(0, 5);
		LCD_write_string("C");
	}
	else if((temp <= -10 && temp > -100) || (temp >= 100)) {
		LCD_goto_XY(0, 0);
		LCD_write_string("      ");
		LCD_goto_XY(0, 0);
		LCD_write_string(s1);
		LCD_goto_XY(0, 6);
		LCD_write_string("C");
	}
	if(temp >= 10){
		LCD_goto_XY(1, 0);
		LCD_write_string("Danger  ");
	}
	else if(temp >= 5 && temp < 10){
		LCD_goto_XY(1, 0);
		LCD_write_string("Warning ");
	}
	else if(temp < 5){
		LCD_goto_XY(1, 0);
		LCD_write_string("OK      ");
	}
}