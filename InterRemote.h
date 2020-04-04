#ifndef INTERREMOTE_H
#define INTERREMOTE_H

#define NEC_AGC 	9000			// 9000 ys
#define NEC_SPACER	4500			// 4500 ys
#define NEC_BURST	560				//  560 ys
#define NEC_ONE		2250-NEC_BURST	// 1690 ys
#define NEC_ZERO	1120-NEC_BURST	//  560 ys
#define NEC_REPEAT	2250			// 2250 ys
#define TOLERANCE	0.3				// 10%

//you may increase this value on Arduinos with greater than 2k SRAM
#define IR_BUFF_LEN 150
#define IR_PIN 2 //pin D2 or D3 on standard arduinos. (other pins may be available on More mordern modules like MEga2560, DUE, ESP8266, ESP32)

typedef enum {
	S_INVALID,	// 0
	S_AGC,		// 1
	S_SPACER,	// 2
	S_CODE,		// 3
	S_DONE,		// 4
	S_REPEAT	// 5
} IRState;

typedef struct {
	uint16_t address;
	uint16_t command;
} IRPacket;

typedef struct {
	IRState state;
	uint8_t pos;
} IRRemote;

extern IRPacket irPacket;

void irInit();
bool irAvailable();
void irDecodeNEC();

#endif // INTERREMOTE_H
