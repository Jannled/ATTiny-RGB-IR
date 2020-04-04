#include <Arduino.h>
#include "InterRemote.h"

//Big thanks to https://www.analysir.com/blog/2014/03/19/air-conditioners-problems-recording-long-infrared-remote-control-signals-arduino/

IRPacket irPacket = {0};
IRRemote irRemote = {0};

volatile unsigned int irBuffer[IR_BUFF_LEN]; //stores timings - volatile because changed by ISR
volatile unsigned int irBuffPos = 0; //Pointer thru irBuffer - volatile because changed by ISR

void handleInterupt()
{	
	if (irBuffPos < IR_BUFF_LEN)
		irBuffer[irBuffPos++] = micros(); //just continually record the time-stamp of signal transitions
}

bool lenMatch(int actualTime, int correctTime, float tolerance)
{
	return abs((int) (actualTime - correctTime)) <= (correctTime * tolerance);
}

void irInit()
{
	pinMode(IR_PIN, INPUT);
	//Serial.begin(9600);

	attachInterrupt(digitalPinToInterrupt(IR_PIN), handleInterupt, CHANGE);
}

void irDecodeNEC()
{
	detachInterrupt(digitalPinToInterrupt(IR_PIN)); // External Interrupt Request 0 Disable

	irRemote.state = S_INVALID;
	irRemote.pos = 0;
	irPacket = {0};

	uint8_t transmissionBegin = 0;

	for (int i = 1; i < irBuffPos; i++)
	{
		switch (irRemote.state)
		{
		case S_AGC:
			if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_SPACER, TOLERANCE))
			{
				transmissionBegin = i;
				irRemote.state = S_SPACER;
				break;
			}
			else if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_REPEAT, TOLERANCE))
			{
				irRemote.state = S_REPEAT;
				break;
			}

		case S_SPACER:
			if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_BURST, TOLERANCE))
			{
				irRemote.state = S_CODE;
				irRemote.pos = 0;
				irPacket = {0};
				break;
			}

		case S_CODE:
			if((i - transmissionBegin) % 2 == 0)
			{
				if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_ONE, TOLERANCE))
				{
					if (irRemote.pos < 16)
						irPacket.address |= (1UL << irRemote.pos++);
					else if (irRemote.pos < 32)
						irPacket.command |= (1UL << (irRemote.pos++ - 16));
					else
						irRemote.state = S_DONE;
					
					break;
				}
	
				else if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_ZERO, TOLERANCE))
				{
					if (irRemote.pos < 32)
						irRemote.pos++;
					else
						irRemote.state = S_DONE;
	
					break;
				}
			}
			else
			{
				if (lenMatch(irBuffer[i] - irBuffer[i - 1], NEC_BURST, TOLERANCE))
					break;
			}

		default:
			if(irRemote.pos < 32)
			{
				irRemote.state = S_INVALID;
				break;
			}

		case S_DONE:
			irRemote.state = S_DONE;
			goto done;

		case S_REPEAT:
			break;
		}

		//Search for begin of transmission
		if (abs((int) irBuffer[i] - irBuffer[i - 1] - NEC_AGC) <= (NEC_AGC * TOLERANCE))
			irRemote.state = S_AGC;

		//Serial.print(i);
		//Serial.print(": ");
		//Serial.print(abs((int) irBuffer[i] - irBuffer[i - 1]));
		//Serial.print(", State: ");
		//Serial.println(irRemote.state);
	}

done:
	//Serial.print("Done: ");
	//Serial.print(irPacket.address, HEX);
	//Serial.print(", ");
	//Serial.print(irPacket.command, HEX);
	//Serial.print("(");
	//Serial.print(irRemote.state);
	//Serial.println(")");

	irBuffPos = 0;
	attachInterrupt(digitalPinToInterrupt(IR_PIN), handleInterupt, CHANGE);	// External Interrupt Request 0 Enable
}

bool irAvailable()
{
	if (irBuffPos < 128)
		return false;

	irDecodeNEC();

	return true;
}
