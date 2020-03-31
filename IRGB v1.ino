/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 9;

#define LED_RED		10
#define LED_GREEN	5
#define LED_BLUE	6

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
bool on = false;

void setup()
{
	Serial.begin(9600);
	// In case the interrupt driver crashes on setup, give a clue
	// to the user what's going on.
	Serial.println("Enabling IRin");
	irrecv.enableIRIn(); // Start the receiver
	Serial.println("Enabled IRin");
	
	//Init LEDs
	pinMode(LED_RED,   OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE,  OUTPUT);
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
			Serial.println("On");
			switchOn();
			break;

		case B_OFF:
			Serial.println("Off");
			switchOff();
			break;

		case B_UP:
			up();
			Serial.print("Up: ");
			Serial.println(brightness);
			break;

		case B_DOWN:
			Serial.print("Down: ");
			Serial.println(brightness);
			down();
			break;

		case B_FLASH:
			Serial.println("Flash");
			brightness = 1.0f;
			animation = 1;
			flash();
			break;

		case B_STROBE:
			Serial.println("Strobe");
			brightness = 1.0f;
			animation = 2;
			strobe();
			break;

		case B_FADE:
			Serial.println("Fade");
			brightness = 0.1f;
			animation = 3;
			pulse();
			break;

		case B_SMOOTH:
			Serial.println("Smooth");
			brightness = 0.1f;
			animation = 4;
			smooth();
			break;
		
		default: 
			Serial.print("Unknown code: ");
			Serial.println(results->value, HEX);
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
