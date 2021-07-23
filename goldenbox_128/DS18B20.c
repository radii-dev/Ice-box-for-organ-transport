/*
 * DS18B20.c
 *
 * Created: 2019-10-09 오후 2:59:28
 *  Author: gigas
 */ 

#define F_CPU 16000000L
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "DS18B20.h"

unsigned short int reset_ds18b20(void)		// 통신 이전 초기화 함수(장치 연결 인식)
{
	unsigned short int existence;		//장치가 연결되었음을 표시하는 변수

	// 마스터가 Tx를 리셋

	DQ_Low;						//전압을 0V로 내림
	DQ_Output;					//PORT E를 출력모드로 설정
	_delay_us(500);				//통신을 위한 딜레이, 480us 이상
	DQ_High;					//전압을 다시 5V로 올림
	
	// 마스터 Rx모드로 변환

	DQ_Input;					//PORT E를 입력모드로 설정
	_delay_us(50);				//슬레이브의 신호를 기다리는 시간 15-60us

	// 마스터가 슬레이브(온도센서)의 존재를 확인

	existence = DQ;				//슬레이브의 입력신호를 변수 existence 에 저장
	_delay_us(500);				//슬레이브의 신호를 수신하는 시간

	// 초기화 종료 이후 다시 마스터가 슬레이브에게 명령을 보내기 위해 출력모드로 설정

	DQ_Output;					//PORT E를 출력모드로 설정
	return existence;			//슬레이브의 존재여부를 리턴
}

unsigned short int read_byte(void)			//마스터가 슬레이브로 부터 정보를 읽는 함수
{
	unsigned short int data = 0x00;			//마스터가 읽은 온도가 저장될 변수
	unsigned short int i;					//포문 돌리려고 선언한 변수
	for (i=0; i<8; i++)
	{
		DQ_Low;   // 전압을 0V로 내림
		_delay_us(2);
		DQ_Input; // PORT E를 입력모드로 전환
		_delay_us(10);

		if (DQ == 0x00)  // 입력받은 값이 1일때(5V일때)
		{
			data |= 1<<i;	// data변수의 i번째 비트에 1을 넣어라
		}
		_delay_us(55);  // 슬레이브의 신호 유지 시간

		DQ_High;	// 전압을 5V로 올림
		DQ_Output;      // PORT E를 출력모드로 전환
		_delay_us(2);   // 다음 비트를 전송받기 이전에 대기시간
	}
	return data;	// 읽어온 온도가 2진수로 저장된 변수를 리턴
}

void write_byte(unsigned char data)
{
	unsigned short int i;	// 포문을 돌리기 위한 변수
	for (i=0; i<8; i++)  // 8비트 전송을 위한 포문
	{
		if (data & (1<<i))  // data의 i번째 비트가 1일때
		{
			DQ_Low;   // 전압을 0V로 내림
			_delay_us(2);	//2us 동안 0V 유지
			DQ_High;  // 전압을 5V로 올림
		}
		else           //data의 i번째 비트가 0일때
		{
			DQ_Low;    // 전압을 0V로 내림
		}
		_delay_us(60);
		DQ_High;         // 다음 비트를 받기 전에 5V로 변경
	}
}

void bitodec(unsigned short int MSB, unsigned short int LSB, float *temp){
	*temp = 0;
	float tmp = 0;
	float a[12];			//온도를 더하는데 사용할 배열 a
	float b[4];			//소수점 이하 부를 더하는데 사용할 배열 b
	if((MSB & 0x08) == 0){   //양수온도
		if((MSB & 0x04) == 0){	// MSB의 2번째 비트가 0이면
			a[0] = 0;		//배열 a[0]에 0을 넣음
		}
		else{ a[0] = 64; }	// MSB의 2번째 비트가 1이면 a[0]에 64(2의 6승)을 넣음
		
		if((MSB & 0x02) == 0){
			a[1] = 0;
		}
		else{ a[1] = 32; }
		
		if((MSB & 0x01) == 0){
			a[2] = 0;
		}
		else{ a[2] = 16; }
		
		if((LSB & 0x80) == 0){
			a[3] = 0;
		}
		else{ a[3] = 8; }
		
		if((LSB & 0x40) == 0){
			a[4] = 0;
		}
		else{ a[4] = 4; }
		
		if((LSB & 0x20) == 0){
			a[5] = 0;
		}
		else{ a[5] = 2; }
		
		if((LSB & 0x10) == 0){
			a[6] = 0;
		}
		else{ a[6] = 1; }
		
		if((LSB & 0x08) == 0){
			a[7] = 0;
		}
		else{ a[7] = 0.5; }
		
		if((LSB & 0x04) == 0){
			a[8] = 0;
		}
		else{ a[8] = 0.25; }
		
		if((LSB & 0x02) == 0){
			a[9] = 0;
		}
		else{ a[9] = 0.125; }
		
		if((LSB & 0x01) == 0){
			a[10] = 0;
		}
		else{ a[10] = 0.0625; }
		
		for(int i = 0; i <= 10; i++){
			*temp += a[i];
		}									//배열 a의 값을 모두 더해서 temp에 저장
	}
	else{   //음수온도
		if((MSB & 0x04) == 0){	// MSB의 2번째 비트가 0이면
			a[0] = 64;		// 64를 저장
		}
		else{ a[0] = 0; }		// MSB의 2번째 비트가 1이면 0을 저장
		
		if((MSB & 0x02) == 0){
			a[1] = 32;
		}
		else{ a[1] = 0; }
		
		if((MSB & 0x01) == 0){
			a[2] = 16;
		}
		else{ a[2] = 0; }
		
		if((LSB & 0x80) == 0){
			a[3] = 8;
		}
		else{ a[3] = 0; }
		
		if((LSB & 0x40) == 0){
			a[4] = 4;
		}
		else{ a[4] = 0; }
		
		if((LSB & 0x20) == 0){
			a[5] = 2;
		}
		else{ a[5] = 0; }
		
		if((LSB & 0x10) == 0){
			a[6] = 1;
		}
		else{ a[6] = 0; }
		
		if((LSB & 0x08) == 0){		// LSB의 3번째 비트가 0이면
			b[0] = 0;		//배열 b에 0을저장
		}
		else{ b[0] = 0.5; }		// LSB의 3번째 비트가 1이면 배열에 0.5를 저장
		
		if((LSB & 0x04) == 0){
			b[1] = 0;
		}
		else{ b[1] = 0.25; }
		
		if((LSB & 0x02) == 0){
			b[2] = 0;
		}
		else{ b[2] = 0.125; }
		
		if((LSB & 0x01) == 0){
			b[3] = 0;
		}
		else{ b[3] = 0.0625; }

		for(int i = 0; i<=6; i++){
			*temp -= a[i];
		}								// 배열 a의 모든 성분을 더하여 온도변수에 넣음
		for(int j = 0; j<=3; j++){
			tmp += b[j];				// 배열 b의 모든 성분을 더하여 임시변수에 넣음
		}
		*temp -= (1-tmp);			// 온도변수에 1에서 임시변수를 뺀 값을 더해서 넣음
	}
	
}
