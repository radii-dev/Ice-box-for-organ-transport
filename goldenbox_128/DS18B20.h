/*
 * DS18B20.h
 *
 * Created: 2019-10-09 오후 2:59:41
 *  Author: gigas
 */ 


#ifndef DS18B20_H_
#define DS18B20_H_

#define DQ_Input  DDRE &= ~(1<<7)           // PORTE7를 입력모드로 설정
#define DQ_Output DDRE |= (1<<7)            // PORTE7를 출력모드로 설정
#define DQ_High   PORTE |= 1<<7             // 전압을 5V로 올림
#define DQ_Low    PORTE &= ~(1<<7)          // 전압을 0v로 내림
#define DQ        ((PINE&(1<<7)) == 0x00)   // 5V 입력받으면 0, 0V 입력받으면 1


unsigned short int reset_ds18b20(void);
unsigned short int read_byte(void);
void write_byte(unsigned char data);
void bitodec(unsigned short int MSB, unsigned short int LSB, float *temp);


#endif /* DS18B20_H_ */