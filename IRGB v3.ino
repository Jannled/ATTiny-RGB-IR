/**
 * Wiring guide:
 * LED Red		PB0
 * LED Green	PB1
 * LED Blue		PB4
 * IR-Receiver	PB2
 * 
 * Spence Kondes ATTinyCore
 * Frequency: 		16MHz (PLL)
 * Timer1 Clock:	CPU
 * millis/micros:	Enabled
 */

#include "InterRemote.h"

#define LED_RED		0
#define LED_GREEN	1
#define LED_BLUE	4

#define B_ON			0xF20D
#define B_OFF			0xE01F
#define B_UP			0xF609
#define B_DOWN			0xE21D
#define B_FLASH			0xB24D
#define B_STROBE		0xFF00
#define B_FADE			0xE51A
#define B_SMOOTH		0xF30C

const uint32_t B_COLORS[]
{
	0xE619,  // B_RED
	0xE817,  // B_DARKORANGE
	0xBF40,  // B_ORANGE
	0xF50A,  // B_LIGHTORANGE
	0xE31C,  // B_YELLOW
	0xE41B,  // B_GREEN
	0xED12,  // B_GRASS
	0xB34C,  // B_TURQUOISE
	0xE11E,  // B_BLUEGREEN
	0xEB14,  // B_GREEBBLUE
	0xEE11,  // B_BLUE
	0xE916,  // B_SLATEBLUE
	0xFB04,  // B_VIOLET
	0xF10E,  // B_PURPLE
	0xF00F,  // B_PINK
	0xEA15   // B_WHITE
};

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color_t;

#define NUM_COLORS 16
const color_t colors[] = 
{
	color_t{255,   0,   0}, // RED
	color_t{255,  85,   0},
	color_t{255, 170,   0},
	color_t{170, 255,   0},
	color_t{ 85, 255,   0},
	color_t{  0, 255,   0}, // GREEN
	color_t{  0, 255,  85},
	color_t{  0, 255, 170},
	color_t{  0, 170, 255},
	color_t{  0,  85, 255}, 
	color_t{  0,   0, 255}, // BLUE
	color_t{ 85,   0, 255},
	color_t{170,   0, 255},
	color_t{255,   0, 170},
	color_t{255,   0,  85},
	color_t{255, 255, 255}  // WHITE
};

color_t currentColor = colors[NUM_COLORS-1];
float brightness = 1.0f;
uint8_t animation = 0;
bool on = true;

void setup()
{
	irInit(); // Start the receiver
	
	//Init LEDs
	pinMode(LED_RED,   OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE,  OUTPUT);
}

void loop() {
	if (irAvailable()) 
		received(irPacket.command);

	if(!on)
		return;

	//Update current animation
	switch(animation)
	{
		case 0:
			staticColor(currentColor, brightness); 
			break;
			
		case 1: 
			flash(); 
			break;
		
		case 2: 
			strobe(); 
			break;
		
		case 3: 
			pulse(); 
			break;
			
		case 4: 
			smooth(); 
			break;
			
		default: 
			break;
	}
}

//Handle IRRemote-Message
void received(uint32_t command)
{
	//Serial.print("Command: ");
	//Serial.println(command, HEX);
	
	//Check if Button is a color button
	for(int i=0; i<NUM_COLORS; i++)
	{
		if(command == B_COLORS[i])
		{
			animation = 0;
			brightness = 1.0f;
			staticColor(colors[i], brightness);
			return;
		}
	}

	//Check if Button is one of the function buttons
	switch(command)
	{
		case B_ON: 
			switchOn();
			break;

		case B_OFF:
			switchOff();
			break;

		case B_UP:
			up();
			break;

		case B_DOWN:
			down();
			break;

		case B_FLASH:
			brightness = 1.0f;
			animation = 1;
			flash();
			break;

		case B_STROBE:
			brightness = 1.0f;
			animation = 2;
			strobe();
			break;

		case B_FADE:
			brightness = 0.1f;
			animation = 3;
			pulse();
			break;

		case B_SMOOTH:
			brightness = 0.1f;
			animation = 4;
			smooth();
			break;
		
		default: 
			break;
	}
}

void switchOn()
{
	on = true;
}

void switchOff()
{
	on = false;
	digitalWrite(LED_RED,   HIGH);
	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_BLUE,  HIGH);
}

void up()
{
	brightness += 0.1;
	brightness = min(1.0f, brightness);
}

void down()
{
	brightness -= 0.1;
	brightness = max(0.1f, brightness);
}

void flash()
{
	const int minSpeed = 50;
	const int speedMult = (minSpeed/brightness);
	
	switchOn();
	float brightness = (sin((float) millis() / speedMult) > 0) ? 1.0f : 0.0f;
	staticColor(currentColor, brightness);
}

void strobe()
{
	const int minSpeed = 100;
	const int speedMult = (minSpeed/brightness);
	
	switchOn();
	staticColor(colors[((millis() / speedMult)) % (NUM_COLORS*2)], brightness);
}

void pulse()
{
	const int minSpeed = 150;
	const int speedMult = (minSpeed/brightness);
	
	switchOn();
	float brightness = (1+sin((float) millis() / speedMult))/2;
	staticColor(currentColor, brightness);
}

void smooth()
{	
	const int minSpeed = 200;
	const int speedMult = (minSpeed/brightness);
	
	switchOn();
	const float phaseShift = (2*PI)/3;
	
	color_t color = {
		(1 + sin((float) millis()/speedMult))                * 128,
		(1 + sin((float) millis()/speedMult + phaseShift))   * 128,
		(1 + sin((float) millis()/speedMult + 2*phaseShift)) * 128
	};
	
	staticColor(color, 1.0f);
}

void staticColor(color_t color, float brightness)
{
	switchOn();
	currentColor = color;
	analogWrite(LED_RED,   255-(color.r*brightness));
	analogWrite(LED_GREEN, 255-(color.g*brightness));
	analogWrite(LED_BLUE,  255-(color.b*brightness));
}
