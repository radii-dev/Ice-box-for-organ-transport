#define F_CPU 16000000L
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "UART1.h"
#include "DS18B20.h"
#include "CLCD.h"
#include "u8g.h"
int optimer[6] = {48, 48, 48, 48, 48, 48};
int dotimer[6] = {48, 48, 48, 48, 48, 48};

u8g_t u8g;
int sec2 = 0;
int temp2 = 0;
int temper[129];

/* GPS Interrupt */
volatile uint8_t gindex = 0;
volatile uint8_t result_index = 0;
volatile uint8_t receive_complete = 0;
char compare[] = "GPGGA,";
volatile char result[100];

ISR(USART0_RX_vect)
{
	char data = UDR0;
	
	if(data == compare[gindex])
	gindex++;
	else if(data == 'E') {
		receive_complete = 1;
		result_index = 0;
		gindex = 0;
	}
	else if(gindex > 5) {
		result[result_index] = data;
		result_index++;
	}
	else
	gindex = 0;
}

void parsingGPS(float* coord){
	char gps[100] = {0,};
	memcpy(gps, (char*)result, 100);
	memset((char*)result, 0, 100);
	char *ptrlat = NULL, *ptrlon = NULL;
	char ptrlat_2[8], ptrlon_2[8];
	memset(ptrlat_2, 0, 8);
	memset(ptrlon_2, 0, 8);
	ptrlat = strtok(gps, ","); // 시간
	ptrlat = strtok(NULL, ","); // latitude
	ptrlon = strtok(NULL, ","); // N
	ptrlon = strtok(NULL, ","); // longitude
	memcpy(ptrlat_2, ptrlat + 2, 8);
	memcpy(ptrlon_2, ptrlon + 3, 8);
	for(int i = 2; ptrlat[i] != '\0'; i++) ptrlat[i] = '\0';
	for(int i = 3; ptrlon[i] != '\0'; i++) ptrlon[i] = '\0';
	float lat = atof(ptrlat);
	float lat_2 = atof(ptrlat_2);
	float lon = atof(ptrlon);
	float lon_2 = atof(ptrlon_2);
	coord[0] = lat + lat_2 / 60;
	coord[1] = lon + lon_2 / 60;
}

typedef union
{
	unsigned char ch[4];
	float fl;
} union_data;

void str_clean(char *str)
{
	for(int i = 0; str[i] != '\0'; i++)
	str[i] = '\0';
}

void u8g_setup(void)
{
	u8g_Init8Bit(&u8g, &u8g_dev_ks0108_128x64,
	PN(2, 0), PN(2, 1), PN(2, 2), PN(2, 3), PN(2, 4), PN(2, 5), PN(2, 6), PN(2, 7),
	PN(0, 2), PN(0, 3), PN(0, 4), PN(0, 0), PN(0, 1),
	U8G_PIN_NONE);
}

void draw1(void)
{
	for(int i=0; i<=128; i+=3)
	{
		u8g_DrawLine(&u8g, i, 20, i, 20);
	}                                    // 이상온도 축(10도)
	
	for(int j=0; j<=128; j+=3)
	{
		u8g_DrawLine(&u8g, j, 25, j, 25);         //이상온도 축(5도)
	}
	
	u8g_DrawLine(&u8g, 0, 63, 127, 63);               // x축
	
	u8g_DrawLine(&u8g, 127, 0, 127, 63);            // y축

	for(int j=0; j<=64; j+=10){
		u8g_DrawLine(&u8g, 124, j, 127, j);
	}                                       // y축 줄표시

	u8g_SetFont(&u8g, u8g_font_6x12);
	u8g_DrawStr(&u8g, 0, 60, "s");                  // 6x12 크기로 단위 s 입력
	
	u8g_SetFont(&u8g, u8g_font_6x12);
	u8g_DrawStr(&u8g, 118, 8, "C");                  // 6x12 크기로 단위 C 입력
	u8g_DrawPixel(&u8g, 117, 1);
}

void draw2(int itemp)
{
	
	sec2++;
	int temp2 = 20-itemp;
	temper[0] = temp2;
	
	for(int a=0; a<=128; a++)
	u8g_DrawPixel(&u8g, a, temper[a]);

	if(sec2==8)
	{
		memmove(temper + 1, temper, sizeof(int) * 128);
		sec2=0;
	}
}

int main(void)
{
	/* Init */
	UART0_init();
	UART1_init();
	LCD_init();
	setCLCD();
	u8g_setup();
	
	DDRE &= ~(1 << PE6);	// Door Init

	DDRD |= (1 << PD0);
	DDRE |= 0x80;   // PE7: 1-wire 통신 케이블
	float temp = 0;
	int itemp = 0;
	unsigned short int MSB=0x00, LSB=0x00;			//MSB레지스터를 저장할 MSB, LSB레지스터를 저장할 LSB선언
	unsigned short int tempavail;	// 온도센서 통신가능여부 확인
	bool read = false; // 문열림 변수
	char buffertemp[8] = {0,};
	char buffercoord[8] = {0,};
	union_data chlat, chlon;
	for(int n = 0; n <= 128; n++) temper[n] = 63;
	
	//DDRE |= 0b00111100;
	//DDRD |= 0b11110000;
	while(1) {
		/* LED test */
		if((PORTD & 0x01) == 0) PORTD |= (1 << PD0);
		else PORTD &= ~(1 << PD0);
		
		/* temperature */
		tempavail = reset_ds18b20();	//온도센서가 연결됬는지 여부를 변수 i에 저장
		if (tempavail == 1)  // 온도센서가 살아있으면
		{
			write_byte(0xCC);  // 슬레이브를 지정
			write_byte(0x44);  // 현재 온도를 측정하라고 명령
			_delay_ms(700);    // 온도 측정 시간 대기
			tempavail = reset_ds18b20();  // 초기화
			write_byte(0xCC);     // 슬레이브를 지정
			write_byte(0xBE);     // 온도센서에 현재온도를 전송하라고 명령
			LSB = read_byte();		// 전송받은 현재온도를 리드 후 LSB에 저장
			MSB = read_byte();		// 전송받은 현재온도를 리드 후 MSB에 저장
			buffertemp[0] = MSB;
			buffertemp[1] = LSB;
		}
		bitodec(MSB, LSB, &temp);
		
		/* timer */
		if(PINE & 0x40) read = true;
		else read = false;
		if(read)
		{
			timer(dotimer, 0);
		}
		timer(optimer, 1);
		
		/* temp --> CLCD */
		tempCLCD(temp);
		
		/* temp --> GLCD */
		itemp = temp;
		u8g_FirstPage(&u8g);
		do
		{
			draw1();
			draw2(itemp);
		}
		while(u8g_NextPage(&u8g));
		
		/* GPS */
		if(receive_complete == 1) { // gps 파싱
			float coord[2] = {0,};
			parsingGPS(coord);
			chlat.fl = coord[0];
			chlon.fl = coord[1];
			if(chlat.fl < 35 || chlat.fl > 39) chlat.fl = 37.542715;
			if(chlon.fl < 125 || chlon.fl > 129) chlon.fl = 127.059030;
			for(int i = 0; i < 4; i++) {
				sprintf(buffercoord, "%c", chlat.ch[i]);
				sprintf(&buffercoord[4], "%c", chlon.ch[i]);
			}
			receive_complete = 0;
		}
		
		//for(int i = 0; i < 8; i++) sprintf(&buffercoord[i], "%c", '\0');
		//for(int i = 2; i < 8; i++) sprintf(&buffertemp[i], "%c", '\0');
		
		
		/* UART TX */
		/*
		UART1_string_transmit(buffercoord);
		_delay_ms(5);
		*/
		UART1_string_transmit(buffertemp);
		_delay_ms(5);
		
	}
}