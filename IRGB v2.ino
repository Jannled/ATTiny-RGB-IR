/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <tiny_IRremote.h>

int RECV_PIN = 2;

#define LED_RED		0
#define LED_GREEN	1
#define LED_BLUE	4

#define B_ON			0xFFB04F
#define B_OFF			0xFFF807
#define B_UP			0xFF906F
#define B_DOWN			0xFFB847
#define B_FLASH			0xFFB24D
#define B_STROBE		0xFF00FF
#define B_FADE			0xFF58A7
#define B_SMOOTH		0xFF30CF

const uint32_t B_COLORS[]
{
	0xFF9867,  // B_RED
	0xFFE817,  // B_DARKORANGE
	0xFF02FD,  // B_ORANGE
	0xFF50AF,  // B_LIGHTORANGE
	0xFF38C7,  // B_YELLOW
	0xFFD827,  // B_GREEN
	0xFF48B7,  // B_GRASS
	0xFF32CD,  // B_TURQUOISE
	0xFF7887,  // B_BLUEGREEN
	0xFF28D7,  // B_GREEBBLUE
	0xFF8877,  // B_BLUE
	0xFF6897,  // B_SLATEBLUE
	0xFF20DF,  // B_VIOLET
	0xFF708F,  // B_PURPLE
	0xFFF00F,  // B_PINK
	0xFFA857   // B_WHITE
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

IRrecv irrecv(RECV_PIN);

decode_results results;

color_t currentColor = colors[NUM_COLORS-1];
float brightness = 1.0f;
uint8_t animation = 0;
bool on = true;

void setup()
{
	irrecv.enableIRIn(); // Start the receiver
	
	//Init LEDs
	pinMode(LED_RED,   OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE,  OUTPUT);

	//Wohoo doing stuff the manual way... (Thanks to http://www.technoblogy.com/show?LE0)
	//Timer 0
	//TCCR0A = 3<<COM0A0 | 3<<COM0B0 | 3<<WGM00; // Note: 3<<COM0A0 | 3<<COM0B0 inverts numbers (0 is 255 and vice versa)
	//TCCR0B = 0<<WGM02 | 3<<CS00; 				 // Optional; already set

	//Timer 1
	//GTCCR = 1<<PWM1B | 3<<COM1B0;
	//TCCR1 = 3<<COM1A0 | 7<<CS10;				 // Note: 3<<COM1A0 inverts numbers (0 is 255 and vice versa)
}

void loop() {
	if (irrecv.decode(&results)) 
	{
		received(&results);
		irrecv.resume(); // Receive the next value
	}
	//delay(100);

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
void received(decode_results* results)
{
	//Check if Button is a color button
	for(int i=0; i<NUM_COLORS; i++)
	{
		if(results->value == B_COLORS[i])
		{
			animation = 0;
			brightness = 1.0f;
			staticColor(colors[i], brightness);
			return;
		}
	}

	//Check if Button is one of the function buttons
	switch(results->value)
	{
		#define B_ON			0xFFB04F
		#define B_OFF			0xFFF807
		#define B_UP			0xFF906F
		#define B_DOWN			0xFFB847
		#define B_FLASH			0xFFB24D
		#define B_STROBE		0xFF00FF
		#define B_FADE			0xFF58A7
		#define B_SMOOTH		0xFF30CF
		
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
/*
	OCR0A = color.r*brightness;
	OCR0B = color.g*brightness;
	OCR1B = color.b*brightness; */
}
