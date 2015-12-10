#include <Arduino.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <RH_NRF24.h>
#include <Adafruit_NeoPixel.h>
#include <ubberFrame.h>
#include <avr/pgmspace.h>

//#define DEBUG

typedef struct opto_capture
{
	unsigned long time;
	unsigned char val;
} opto_capture_t;

/*Port Definitions*/
// Digital pins
#define RF433_RX_PIN        2
#define RF433_TX_PIN        3
#define RF433_TT_PIN        4
#define BOUTONSW_PIN        5
#define PIXELPIN            6
#define MAX7219_CLK_PIN     7
#define MAX7219_DIN_PIN     8
#define MAX7219_CS_PIN      9
#define NRF24_CSN_PIN       10
#define NRF24_MOSI_PIN      11
#define NRF24_MISO_PIN      12
#define NRF24_SCK_PIN       13
#define NRF24_CE_PIN        4




// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      5

// Analog Pins
#define PHOTORP_PIN 4
#define PHOTOR1_PIN 0
#define PHOTOR2_PIN 1
#define PHOTOR3_PIN 2
#define PHOTOR4_PIN 3
#define BOUTONX_PIN 5
#define BOUTONY_PIN 6

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel (NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
RH_ASK RH_433 = RH_ASK (1000, 2, 3, 4, false);
RH_NRF24 RH_24 = RH_NRF24 (NRF24_CE_PIN, NRF24_CSN_PIN);

unsigned char i;
unsigned char j;

typedef enum state_machine
{
	STATE_RECEIVE = 0,
	STATE_TRANSMIT_DEST,
	STATE_TRANSMIT_EVENT,
	STATE_TRANSMIT_EXTENSION,
	STATE_TRANSMIT_EMIT,
	STATE_ABORT,
	STATE_MAX,
} state_machine_t;

uint8_t code_dest, code_event;
UbberFrame UF;
state_machine_t sm = STATE_RECEIVE;

typedef enum disp_symbol
{
	SYMBOL_0 = 0,
	SYMBOL_1,
	SYMBOL_2,
	SYMBOL_3,
	SYMBOL_4,
	SYMBOL_5,
	SYMBOL_6,
	SYMBOL_7,
	SYMBOL_8,
	SYMBOL_9,
	SYMBOL_A,
	SYMBOL_B,
	SYMBOL_C,
	SYMBOL_D,
	SYMBOL_E,
	SYMBOL_F,
	SYMBOL_G,
	SYMBOL_H,
	SYMBOL_I,
	SYMBOL_J,
	SYMBOL_K,
	SYMBOL_L,
	SYMBOL_M,
	SYMBOL_N,
	SYMBOL_O,
	SYMBOL_P,
	SYMBOL_Q,
	SYMBOL_R,
	SYMBOL_S,
	SYMBOL_T,
	SYMBOL_U,
	SYMBOL_V,
	SYMBOL_W,
	SYMBOL_X,
	SYMBOL_Y,
	SYMBOL_Z,
	SYMBOL_CLOPE,
	SYMBOL_CAFE,
	SYMBOL_URGENCE,
	SYMBOL_REPAS,
	SYMBOL_CLOCHE,
	SYMBOL_SMILEY_GOOD,
	SYMBOL_SMILEY_MMM,
	SYMBOL_SMILEY_BAD,
	SYMBOL_DOOR,
	SYMBOL_TARGET,
	SYMBOL_SPIRAL,
	SYMBOL_MONTAIN,
	SYMBOL_CAR,
	SYMBOL_PISCINE,
	SYMBOL_POUCE,	
	SYMBOL_EMPTY,
	SYMBOL_FULL,
	SYMBOL_MAX
} disp_symbol_t;

/* Font Generator */
/* http://blog.riyas.org/2013/12/online-led-matrix-font-generator-with.html */
const unsigned char disp1[SYMBOL_MAX][8] PROGMEM = {
	/*0 */ {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C},
	//0
	/*1 */ {0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x3e},
	//1
	/*2 */ {0x7E, 0x2, 0x2, 0x7E, 0x40, 0x40, 0x40, 0x7E},
	//2
	/*3 */ {0x3e, 0x02, 0x02, 0x1e, 0x02, 0x02, 0x02, 0x3e},
	//3
	/*4 */ {0x8, 0x18, 0x28, 0x48, 0xFE, 0x8, 0x8, 0x8},
	//4
	/*5 */ {0x7e, 0x40, 0x40, 0x7c, 0x02, 0x02, 0x02, 0x7c},
	//5
	/*6 */ {0x7e, 0x40, 0x40, 0x7e, 0x42, 0x42, 0x42, 0x7e},
	//6
	/*7 */ {0x7e, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x10},
	//7
	/*8 */ {0x7e, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x7e},
	//8
	/*9 */ {0x7e, 0x42, 0x42, 0x7e, 0x02, 0x02, 0x02, 0x7e},
	//9
	/*10*/ {0x8, 0x14, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x22},
	//A
	/*11*/ {0x3C, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x3C, 0x0},
	//B
	/*12*/ {0x3C, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3C, 0x0},
	//C
	/*13*/ {0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x0},
	//D
	/*14*/ {0x7C, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x7C},
	//E
	/*15*/ {0x7C, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x40},
	//F
	/*16*/ {0x3C, 0x40, 0x40, 0x40, 0x40, 0x44, 0x44, 0x3C},
	//G
	/*17*/ {0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x44},
	//H
	/*18*/ {0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C},
	//I
	/*19*/ {0x3C, 0x8, 0x8, 0x8, 0x8, 0x8, 0x48, 0x30},
	//J
	/*20*/ {0x0, 0x24, 0x28, 0x30, 0x20, 0x30, 0x28, 0x24},
	//K
	/*21*/ {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C},
	//L
	/*22*/ {0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81},
	//M
	/*23*/ {0x0, 0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x0},
	//N
	/*24*/ {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C},
	//O
	/*25*/ {0x3C, 0x22, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x20},
	//P
	/*26*/ {0x1C, 0x22, 0x22, 0x22, 0x22, 0x26, 0x22, 0x1D},
	//Q
	/*27*/ {0x3C, 0x22, 0x22, 0x22, 0x3C, 0x24, 0x22, 0x21},
	//R
	/*28*/ {0x0, 0x1E, 0x20, 0x20, 0x3E, 0x2, 0x2, 0x3C},
	//S
	/*29*/ {0x0, 0x3E, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8},
	//T
	/*30*/ {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x22, 0x1C},
	//U
	/*31*/ {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18},
	//V
	/*32*/ {0x0, 0x49, 0x49, 0x49, 0x49, 0x2A, 0x1C, 0x0},
	//W
	/*33*/ {0x0, 0x41, 0x22, 0x14, 0x8, 0x14, 0x22, 0x41},
	//X
	/*34*/ {0x41, 0x22, 0x14, 0x8, 0x8, 0x8, 0x8, 0x8},
	//Y
	/*35*/ {0x0, 0x7F, 0x2, 0x4, 0x8, 0x10, 0x20, 0x7F},
	//Z
	/*36*/ {0x02, 0x04, 0x04, 0x02, 0x00, 0xfd, 0xfd, 0x00},
	//clope
	/*37*/ {0x20, 0x10, 0x10, 0x47, 0x45, 0x47, 0x44, 0x7c},
	//cafe
	/*38*/ {0x00, 0x54, 0xd6, 0x92, 0x92, 0xd6, 0x44, 0x10},
	//urgence
	/*39*/ {0x3c, 0x7e, 0x7e, 0x00, 0x7e, 0x00, 0x7e, 0x7e},
	//repas
	/*40*/ {0x38, 0x44, 0x44, 0x92, 0x92, 0x92, 0xaa, 0x38},
	//cloche
	/*41*/ {0x3c, 0x42, 0xa5, 0x81, 0xa5, 0x99, 0x42, 0x3c},
	//smiley :-)
	/*42*/ {0x3c, 0x42, 0xa5, 0x81, 0xbd, 0x81, 0x42, 0x3c},
	//smiley :-|
	/*43*/ {0x3c, 0x42, 0xa5, 0x81, 0x99, 0xa5, 0x42, 0x3c},
	//smiley :-(
	/*44*/ {0xe7, 0x81, 0x81, 0x81, 0x99, 0x81, 0x81, 0xe7},
	// [-]
	/*45*/ {0x81, 0x3c, 0x42, 0x5a, 0x5a, 0x42, 0x3c, 0x81},
	// target
	/*46*/ {0xff, 0x81, 0xbd, 0xa5, 0xad, 0xa1, 0xbf, 0x80},
	// spiral
	/*47*/ {0x00, 0x3c, 0x00, 0x00, 0x18, 0x24, 0x42, 0x81},
	// montain
	/*48*/ {0x00, 0x38, 0x44, 0xc3, 0xff, 0x44, 0x00, 0x00},
	// car
	/*49*/ {0x00, 0x44, 0xaa, 0x11, 0x00, 0x44, 0xaa, 0x11},
	// piscine
	/*50*/ {0x0c, 0x18, 0x30, 0xf6, 0xe1, 0xe6, 0xe1, 0x1e},
	// pouce	
	/*51*/ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	// Empty
	/*52*/ {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	// Full
};

#define ERR_U 0xFF
#define ERR_D 0x80

// Hamming 8/4 Encoding Table[SEND_NIBBLE]
const uint8_t H4_8[16] PROGMEM = {
	0x15, // 0x0
	0x2,  // 0x1
	0x49, // 0x2
	0x5E, // 0x3
	0x64, // 0x4
	0x73, // 0x5
	0x38, // 0x6
	0x2F, // 0x7
	0xD0, // 0x8
	0xC7, // 0x9
	0x8C, // 0xA
	0x9B, // 0xB
	0xA1, // 0xC
	0xB6, // 0xD
	0xFD, // 0xE
	0xEA  // 0xF
};

// Hamming 8/4 Decoding/Correcting Table [MSB_RECV_NIBBLE][LSB_RECV_NIBBLE]
const uint8_t H8_4[16][16] PROGMEM = {
0x1|ERR_D,ERR_U,0x1,0x1|ERR_D,ERR_U,0x0|ERR_D,0x1|ERR_D,ERR_U,ERR_U,0x2|ERR_D,0x1|ERR_D,ERR_U,0xA|ERR_D,ERR_U,ERR_U,0x7|ERR_D, // 0x0
ERR_U,0x0|ERR_D,0x1|ERR_D,ERR_U,0x0|ERR_D,0x0,ERR_U,0x0|ERR_D,0x6|ERR_D,ERR_U,ERR_U,0xB|ERR_D,ERR_U,0x0|ERR_D,0x3|ERR_D,ERR_U, // 0x1
ERR_U,0xC|ERR_D,0x1|ERR_D,ERR_U,0x4|ERR_D,ERR_U,ERR_U,0x7|ERR_D,0x6|ERR_D,ERR_U,ERR_U,0x7|ERR_D,ERR_U,0x7|ERR_D,0x7|ERR_D,0x7, // 0x2
0x6|ERR_D,ERR_U,ERR_U,0x5|ERR_D,ERR_U,0x0|ERR_D,0xD|ERR_D,ERR_U,0x6,0x6|ERR_D,0x6|ERR_D,ERR_U,0x6|ERR_D,ERR_U,ERR_U,0x7|ERR_D, // 0x3
ERR_U,0x2|ERR_D,0x1|ERR_D,ERR_U,0x4|ERR_D,ERR_U,ERR_U,0x9|ERR_D,0x2|ERR_D,0x2,ERR_U,0x2|ERR_D,ERR_U,0x2|ERR_D,0x3|ERR_D,ERR_U, // 0x4
0x8|ERR_D,ERR_U,ERR_U,0x5|ERR_D,ERR_U,0x0|ERR_D,0x3|ERR_D,ERR_U,ERR_U,0x2|ERR_D,0x3|ERR_D,ERR_U,0x3|ERR_D,ERR_U,0x3,0x3|ERR_D, // 0x5
0x4|ERR_D,ERR_U,ERR_U,0x5|ERR_D,0x4,0x4|ERR_D,0x4|ERR_D,ERR_U,ERR_U,0x2|ERR_D,0xF|ERR_D,ERR_U,0x4|ERR_D,ERR_U,ERR_U,0x7|ERR_D, // 0x6
ERR_U,0x5|ERR_D,0x5|ERR_D,0x5,0x4|ERR_D,ERR_U,ERR_U,0x5|ERR_D,0x6|ERR_D,ERR_U,ERR_U,0x5|ERR_D,ERR_U,0xE|ERR_D,0x3|ERR_D,ERR_U, // 0x7
ERR_U,0xC|ERR_D,0x1|ERR_D,ERR_U,0xA|ERR_D,ERR_U,ERR_U,0x9|ERR_D,0xA|ERR_D,ERR_U,ERR_U,0xB|ERR_D,0xA,0xA|ERR_D,0xA|ERR_D,ERR_U, // 0x8
0x8|ERR_D,ERR_U,ERR_U,0xB|ERR_D,ERR_U,0x0|ERR_D,0xD|ERR_D,ERR_U,ERR_U,0xB|ERR_D,0xB|ERR_D,0xB,0xA|ERR_D,ERR_U,ERR_U,0xB|ERR_D, // 0x9
0xC|ERR_D,0xC,ERR_U,0xC|ERR_D,ERR_U,0xC|ERR_D,0xD|ERR_D,ERR_U,ERR_U,0xC|ERR_D,0xF|ERR_D,ERR_U,0xA|ERR_D,ERR_U,ERR_U,0x7|ERR_D, // 0xA
ERR_U,0xC|ERR_D,0xD|ERR_D,ERR_U,0xD|ERR_D,ERR_U,0xD,0xD|ERR_D,0x6|ERR_D,ERR_U,ERR_U,0xB|ERR_D,ERR_U,0xE|ERR_D,0xD|ERR_D,ERR_U, // 0xB
0x8|ERR_D,ERR_U,ERR_U,0x9|ERR_D,ERR_U,0x9|ERR_D,0x9|ERR_D,0x9,ERR_U,0x2|ERR_D,0xF|ERR_D,ERR_U,0xA|ERR_D,ERR_U,ERR_U,0x9|ERR_D, // 0xC
0x8,0x8|ERR_D,0x8|ERR_D,ERR_U,0x8|ERR_D,ERR_U,ERR_U,0x9|ERR_D,0x8|ERR_D,ERR_U,ERR_U,0xB|ERR_D,ERR_U,0xE|ERR_D,0x3|ERR_D,ERR_U, // 0xD
ERR_U,0xC|ERR_D,0xF|ERR_D,ERR_U,0x4|ERR_D,ERR_U,ERR_U,0x9|ERR_D,0xF|ERR_D,ERR_U,0xF,0xF|ERR_D,ERR_U,0xE|ERR_D,0xF|ERR_D,ERR_U, // 0xE
0x8|ERR_D,ERR_U,ERR_U,0x5|ERR_D,ERR_U,0xE|ERR_D,0xD|ERR_D,ERR_U,ERR_U,0xE|ERR_D,0xF|ERR_D,ERR_U,0xE|ERR_D,0xE,ERR_U,0xE|ERR_D, // 0xF
};

uint16_t Encode_8b_H4_8 (uint8_t input)
{
	uint16_t lsb = pgm_read_byte_near( &(H4_8[input&0xF]) );
	uint16_t msb = ((pgm_read_byte_near( &(H4_8[((input&0xF0)>>4)]) )) << 8) | lsb;
	return msb; 
}

int8_t Decode_8b_H8_4 (uint16_t input, uint8_t *output)
{
	uint8_t recovery=0;
	uint8_t lsb = pgm_read_byte_near( &(H8_4[(input&0xF0)>>4][input&0xF]) );
	uint8_t msb = pgm_read_byte_near( &(H8_4[(input&0xF000)>>12][(input&0xF00)>>8]) );

#if DEBUG
	Serial.print ("\n*");
	Serial.print (input, HEX);
	Serial.print ("\n*");
	Serial.print (lsb, HEX);
	Serial.print ("==");
	Serial.print (msb, HEX);
	Serial.print ("\n");
#endif
	if ((lsb == ERR_U) || (msb == ERR_U))
		return -1;
	if (lsb & ERR_D)
		recovery++;
	if (msb & ERR_D)
		recovery++;

#ifdef DEBUG
	Serial.print ("*\n");
	Serial.print (recovery, HEX);
#endif

	*output = (((msb & 0xF)<<4) | (lsb & 0xF));
	return recovery;
}

void Write_Max7219_byte (unsigned char DATA);
void Write_Max7219 (unsigned char address, unsigned char dat);
void Show_Max7219 (unsigned char symbol);
void Init_MAX7219 (void);
int Photo_code (uint16_t * val);

	int 
freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

	void
Write_Max7219_byte (unsigned char DATA)
{
	unsigned char i;
	digitalWrite (MAX7219_CS_PIN, LOW);
	for (i = 8; i >= 1; i--)
	{
		digitalWrite (MAX7219_CLK_PIN, LOW);
		digitalWrite (MAX7219_DIN_PIN, DATA & 0x80);	// Extracting a bit data
		DATA = DATA << 1;
		digitalWrite (MAX7219_CLK_PIN, HIGH);
	}
}


	void
Write_Max7219 (unsigned char address, unsigned char dat)
{
	digitalWrite (MAX7219_CS_PIN, LOW);
	Write_Max7219_byte (address);	//address，code of LED
	Write_Max7219_byte (dat);	//data，figure on LED 
	digitalWrite (MAX7219_CS_PIN, HIGH);
}

	void
Show_Max7219 (unsigned char symbol)
{
	for (int i = 1; i < 9; i++)
	{
		//Write_Max7219 (i, disp1[(symbol % SYMBOL_MAX)][i - 1]);
		Write_Max7219 (i, pgm_read_byte_near( &(disp1[(symbol % SYMBOL_MAX)][i - 1]) ));
	}
}

	void
Init_MAX7219 (void)
{
	Write_Max7219 (0x09, 0x00);	//decoding ：BCD
	Write_Max7219 (0x0a, 0x03);	//brightness 
	Write_Max7219 (0x0b, 0x07);	//scanlimit；8 LEDs
	Write_Max7219 (0x0c, 0x01);	//power-down mode：0，normal mode：1
	Write_Max7219 (0x0f, 0x00);	//test display：1；EOT，display：0
}

	void
Disable_Max7219 (void)
{
	Write_Max7219 (0x0c, 0x00);	//power-down mode：0，normal mode：1
}

	void
Enable_Max7219 (void)
{
	Write_Max7219 (0x0c, 0x01);	//power-down mode：0，normal mode：1
}

char Read_Button_PB(int debounce, char * exiting)
{
	char BoutonSW = 1;
	BoutonSW = digitalRead (BOUTONSW_PIN);
	if ((BoutonSW == 0) && exiting && (*exiting == 0)) {
		*exiting = 1;
		return BoutonSW;
	}
	delay(debounce);
	return BoutonSW;
}


#define PHOTO_SENSIBILITYP 820
#define PHOTO_SENSIBILITY1 820
#define PHOTO_SENSIBILITY2 820
#define PHOTO_SENSIBILITY3 820
#define PHOTO_SENSIBILITY4 820

//#define PHOTO_CAPTURE_CALIBRATION
#ifdef PHOTO_CAPTURE_CALIBRATION
opto_capture_t oc[80];
#endif

	int
Photo_code (uint8_t * val)
{
	int i;
	unsigned char err_x[5];
	unsigned char err_y[4];
	unsigned char err_x_total = 0;
	unsigned char err_y_total = 0;
	unsigned int Photo_RP, Photo_R1, Photo_R2, Photo_R3, Photo_R4;
	unsigned int Photo_RP_tmp, Photo_R1_tmp, Photo_R2_tmp, Photo_R3_tmp,
		     Photo_R4_tmp;
	unsigned long time1, time2a[5], time2b[5], time3;
	unsigned int local_val[5];
	uint16_t val16;
	int8_t err_rec;
	char BoutonSW = 1;
	BoutonSW = Read_Button_PB (0, 0);


	Photo_RP = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 1;
	/* Waiting for carte insertion all to zero */
	while ((Photo_RP != 0) || (Photo_R1 != 0) || (Photo_R2 != 0)
			|| (Photo_R3 != 0) || (Photo_R4 != 0))
	{
		Photo_RP = ((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITYP) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("Photo1\n");
		Serial.print ("P:");
		Serial.print (Photo_RP);
		Serial.print ("\t");
		Serial.print ("1:");
		Serial.print (Photo_R1);
		Serial.print ("\t");
		Serial.print ("2:");
		Serial.print (Photo_R2);
		Serial.print ("\t");
		Serial.print ("3:");
		Serial.print (Photo_R3);
		Serial.print ("\t");
		Serial.print ("4:");
		Serial.print (Photo_R4);
		Serial.print ("\n");
#endif
		if ((BoutonSW = Read_Button_PB(0,0)) == 0)
		{
			goto early_exit;
		}

	}
	time1 = micros ();
#ifdef PHOTO_CAPTURE_CALIBRATION
	//delay (1);
	for (int i = 0; i < 80; i++)
	{
		Photo_RP = ((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITYP) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
		oc[i].time = micros ();
		oc[i].val =
			Photo_RP | Photo_R1 << 1 | Photo_R2 << 2 | Photo_R3 << 3 | Photo_R4 <<
			4;
		delayMicroseconds (500);
	}

	for (int i = 0; i < 100; i++)
	{
		Serial.print ("PhotoCap\n");
		Serial.print ("\tindex:");
		Serial.print (i);
		Serial.print ("\ttime:");
		Serial.print (oc[i].time);
		Serial.print ("\tval:");
		Serial.print (oc[i].val, BIN);
		Serial.print ("\n");
	}
#endif

	for (int i = 0; i < 5; i++)
	{
		Photo_RP = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 0;
		local_val[i] = 0;
		/* Waiting At least one hole found */
		while ((Photo_RP == 0) && (Photo_R1 == 0) && (Photo_R2 == 0)
				&& (Photo_R3 == 0) && (Photo_R4 == 0))
		{
			Photo_RP =
				((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITYP) ? 0 : 1);
			Photo_R1 =
				((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
			Photo_R2 =
				((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
			Photo_R3 =
				((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
			Photo_R4 =
				((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
			Serial.print ("Photo2\n");
			Serial.print ("P:");
			Serial.print (Photo_RP);
			Serial.print ("\t");
			Serial.print ("1:");
			Serial.print (Photo_R1);
			Serial.print ("\t");
			Serial.print ("2:");
			Serial.print (Photo_R2);
			Serial.print ("\t");
			Serial.print ("3:");
			Serial.print (Photo_R3);
			Serial.print ("\t");
			Serial.print ("4:");
			Serial.print (Photo_R4);
			Serial.print ("\n");
#endif
			if ((BoutonSW = Read_Button_PB (0, 0)) == 0)
			{
				goto early_exit;
			}

		}
		time2a[i] = micros ();
		delayMicroseconds (2700);
		/* Sampling Values */
		Photo_RP = ((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITYP) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("PhotoSampling\n");
		Serial.print ("P:");
		Serial.print (Photo_RP);
		Serial.print ("\t");
		Serial.print ("1:");
		Serial.print (Photo_R1);
		Serial.print ("\t");
		Serial.print ("2:");
		Serial.print (Photo_R2);
		Serial.print ("\t");
		Serial.print ("3:");
		Serial.print (Photo_R3);
		Serial.print ("\t");
		Serial.print ("4:");
		Serial.print (Photo_R4);
		Serial.print ("\n");
#endif
		//delay (1);
		/* Waiting back all to zero for next acquisition */
		Photo_RP_tmp = Photo_R1_tmp = Photo_R2_tmp = Photo_R3_tmp =
			Photo_R4_tmp = 1;
		while ((Photo_RP_tmp != 0) || (Photo_R1_tmp != 0) || (Photo_R2_tmp != 0)
				|| (Photo_R3_tmp != 0) || (Photo_R4_tmp != 0))
		{
			Photo_RP_tmp =
				((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITYP) ? 0 : 1);
			Photo_R1_tmp =
				((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
			Photo_R2_tmp =
				((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
			Photo_R3_tmp =
				((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
			Photo_R4_tmp =
				((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
			Serial.print ("Photo2b\n");
			Serial.print ("P:");
			Serial.print (Photo_RP_tmp);
			Serial.print ("\t");
			Serial.print ("1:");
			Serial.print (Photo_R1_tmp);
			Serial.print ("\t");
			Serial.print ("2:");
			Serial.print (Photo_R2_tmp);
			Serial.print ("\t");
			Serial.print ("3:");
			Serial.print (Photo_R3_tmp);
			Serial.print ("\t");
			Serial.print ("4:");
			Serial.print (Photo_R4_tmp);
			Serial.print ("\n");
#endif

			if ((BoutonSW = Read_Button_PB (0, 0)) == 0)
			{
				goto early_exit;
			}

		}

		time2b[i] = micros ();
		err_x[i] =
			((((Photo_RP + Photo_R1 + Photo_R2 + Photo_R3 + Photo_R4) % 2) ==
			  0) ? 1 : 0);
		err_x_total += err_x[i];
		local_val[i] =
			(((Photo_R1 & 0x1) << 0) | ((Photo_R2 & 0x1) << 1) |
			 ((Photo_R3 & 0x1) << 2) | ((Photo_R4 & 0x1) << 3));

	}

	//delay (1);
	Photo_RP = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 0;
	/* Waiting for carte exit all to one */
	while ((Photo_RP != 1) || (Photo_R1 != 1) || (Photo_R2 != 1)
			|| (Photo_R3 != 1) || (Photo_R4 != 1))
	{
		Photo_RP = ((analogRead (PHOTORP_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("Photo3\n");
		Serial.print ("P:");
		Serial.print (Photo_RP);
		Serial.print ("\t");
		Serial.print ("1:");
		Serial.print (Photo_R1);
		Serial.print ("\t");
		Serial.print ("2:");
		Serial.print (Photo_R2);
		Serial.print ("\t");
		Serial.print ("3:");
		Serial.print (Photo_R3);
		Serial.print ("\t");
		Serial.print ("4:");
		Serial.print (Photo_R4);
		Serial.print ("\n");
#endif

		if ((BoutonSW = Read_Button_PB (0 , 0)) == 0)
		{
			goto early_exit;
		}

	}
	time3 = micros ();
#ifdef DEBUG
	Serial.print ("\nTime:\n");
	Serial.print ((time3 - time1), DEC);
	Serial.print ("\nTime 0:\n");
	Serial.print ((time2a[0] - time1), DEC);
	Serial.print ("\nTimediff 0:\n");
	Serial.print ((time2b[0] - time2a[0]), DEC);
	Serial.print ("\nVal 0:\n");
	Serial.print (local_val[0], BIN);
	Serial.print ("\nTime 1:\n");
	Serial.print ((time2a[1] - time1), DEC);
	Serial.print ("\nTimediff 1:\n");
	Serial.print ((time2b[1] - time2a[1]), DEC);
	Serial.print ("\nVal 1:\n");
	Serial.print (local_val[1], BIN);
	Serial.print ("\nTime 2:\n");
	Serial.print ((time2a[2] - time1), DEC);
	Serial.print ("\nTimediff 2:\n");
	Serial.print ((time2b[2] - time2a[2]), DEC);
	Serial.print ("\nVal 2:\n");
	Serial.print (local_val[2], BIN);
	Serial.print ("\nTime 3:\n");
	Serial.print ((time2a[3] - time1), DEC);
	Serial.print ("\nTimediff 3:\n");
	Serial.print ((time2b[3] - time2a[3]), DEC);
	Serial.print ("\nVal 3:\n");
	Serial.print (local_val[3], BIN);
	Serial.print ("\nTime 4:\n");
	Serial.print ((time2a[4] - time1), DEC);
	Serial.print ("\nTimediff 4:\n");
	Serial.print ((time2b[4] - time2a[4]), DEC);
	Serial.print ("\nVal 4:\n");
	Serial.print (local_val[4], BIN);
	Serial.print ("\n-----\n");
#endif

	for (int i = 0; i < 4; i++)
	{
		err_y[i] =
			((((((local_val[0] & (0x1 << i)) >> i) +
			    ((local_val[1] & (0x1 << i)) >> i) +
			    ((local_val[2] & (0x1 << i)) >> i) +
			    ((local_val[3] & (0x1 << i)) >> i) +
			    ((local_val[4] & (0x1 << i)) >> i)) % 2) == 0) ? 1 : 0);
		err_y_total += err_y[i];
	}

	// only one Bit error recovery if not on x recovery err line
	if ((err_x_total == err_y_total) && (err_x_total == 1) && (err_x[4] == 0))
	{
		for (int i = 0; i < 4; i++)
		{
			if (err_x[i] == 1)
			{
				for (int j = 0; j < 4; j++)
				{
					if (err_y[j] == 1)
					{
						Serial.print ("\nECC!!\n");
						local_val[i] ^= (1 << j);
						err_y_total = err_x_total = 0;
					}
				}
			}
		}
	}

#ifdef DEBUG
	Serial.print ("=\n");
	for (int i = 0; i < 5; i++)
	{
		Serial.print (err_x[i]);
		Serial.print (",");
	}
	Serial.print (err_x_total);
#endif
#ifdef DEBUG
	Serial.print ("-\n");
	for (int i = 0; i < 4; i++)
	{
		Serial.print (err_y[i]);
		Serial.print (",");
	}
	Serial.print (err_y_total);
#endif

	val16 =
		((((unsigned long) (local_val[3])) << 0) |
		 (((unsigned long) (local_val[2])) << 4) |
		 (((unsigned long) (local_val[1])) << 8) |
		 (((unsigned long) (local_val[0])) << 12));

	err_rec = Decode_8b_H8_4(val16, val);

#ifdef DEBUG
	Serial.print ("\n");
	Serial.print (val16, HEX);
	Serial.print ("-");
	Serial.print (*val, HEX);
	Serial.print ("-");
	Serial.print (err_rec, HEX);
	Serial.print ("\n");

	Serial.println(freeRam());
#endif

	if (err_rec >= 0) {
		return 0;
	}
	else
		return -2;

early_exit:
	return -1;
}

typedef enum led_mask
{
	LED0 = 0x1,
	LED1 = 0x2,
	LED2 = 0x4,
	LED3 = 0x8,
	LED4 = 0x10,
	LED_ALL = 0x1F
} led_mask_t;

	int
led_colors(led_mask_t lm, unsigned char r, unsigned char g, unsigned char b ) {
	for (int i = 0; i < NUMPIXELS; i++)
	{
		if ((1<<i)&lm)
			pixels.setPixelColor (i, pixels.Color (r, g, b));
		else
			pixels.setPixelColor (i, pixels.Color (0, 0, 0));

	}
	pixels.show ();
	return 0;
}

	void
setup ()
{
	uint8_t i;
	uint16_t elem16;
	UbberFrame *uf = &UF;

	Serial.begin (115200);	// Initialisation du port série pour avoir un retour sur le serial monitor

	pinMode (MAX7219_CLK_PIN, OUTPUT);
	pinMode (MAX7219_CS_PIN, OUTPUT);
	pinMode (MAX7219_DIN_PIN, OUTPUT);
	Serial.println ("Radiohead 433 initialization");
	if (!RH_433.init ())
		Serial.println ("Radiohead 433 init failed");
	if (!RH_24.init())
		Serial.println("Radiohead NRF24 init failed");
	// Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
	if (!RH_24.setChannel(1))
		Serial.println("setChannel failed");
	if (!RH_24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");

	pixels.begin ();		// This initializes the NeoPixel library.

	Serial.println ("LED Matrix initialization");
	Init_MAX7219 ();
	Show_Max7219 (SYMBOL_EMPTY);
	Serial.println ("RGB LED initialization");
	led_colors(LED_ALL, 0, 0, 0);
	led_colors(LED_ALL, 255, 0, 0);
	led_colors(LED0, 64, 0, 0);
	
#if DEBUG
	for (i =1 ; i < 8; i++) {
		UF.setDestID(i);
		Serial.print ("\nDest : ");
		Serial.print (i, HEX);
		Serial.print (" = ");
		Serial.print (uf->getDestIDString());
		Serial.print (" => ");
		elem16 = Encode_8b_H4_8(i);
		Serial.print (elem16, HEX);
		Serial.print ("\n");
	}
	for (i =0 ; i < 10; i++) {
		UF.setType(i);
		Serial.print ("\nEvent : ");
		Serial.print (i, HEX);
		Serial.print (" = ");
		Serial.print (uf->getTypeString());
		Serial.print (" => ");
		elem16 = Encode_8b_H4_8(i);
		Serial.print (elem16, HEX);
		Serial.print ("\n");
	}
#endif

}

	void 
Show_UbberFrame(UbberFrame * uf)
{
	int i = uf->getPayloadSize();
	char * str = (char *)(uf->getPayload());
	str = (char *)(uf->getPayload());
	i = uf->getPayloadSize();
	Serial.print ("\nSource\n");
	Serial.print (uf->getSourceIDString());
	Serial.print ("\nDestination\n");
	Serial.print (uf->getDestIDString());
	Serial.print ("\nType\n");
	Serial.print (uf->getTypeString());
	Serial.print ("\nLength\n");
	Serial.print (uf->getLength());
	Serial.print ("\nPayload\n");
	while (i-- != 0)
		Serial.print(*(str++));
	Serial.print ("\n");

}


	void
loop ()
{
	int BoutonX, BoutonY;
	int beautiful_loop=0;
	char BoutonSW;
	uint8_t buf[/*RH_ASK_MAX_MESSAGE_LEN*/16];
	uint8_t buflen = sizeof (buf);


	/*****************/
	/* TRANSMIT EMIT */
	/*****************/
	if (sm == STATE_TRANSMIT_EMIT)
	{
		Show_Max7219 (SYMBOL_X);
		delay (1000);
		Disable_Max7219();
		Serial.print ("STATE_TRANSMIT_EMIT\t");
		Show_UbberFrame(&UF);
		RH_433.send (UF.frameToChar(), UF.getLength());
		RH_433.waitPacketSent ();
		RH_24.send(UF.frameToChar(), UF.getLength());
		RH_24.waitPacketSent();
		Enable_Max7219();
		sm = STATE_RECEIVE;
	}
	/*************/
	/*  RECEIVE  */
	/*************/
	else if (sm == STATE_RECEIVE)
	{
		int x = 1;
		int old_led = -1;
		Show_Max7219 (SYMBOL_R);
		delay (1000);

#ifdef DEBUG
		Serial.print ("STATE_RECEIVE\n");
#endif

		while (1)
		{
			int sel_led;
			RH_24.waitAvailableTimeout(0);
			if ((RH_433.recv (buf, &buflen)) || (RH_24.recv(buf, &buflen))) // Non-blocking
			{
				int i,j;
				int err;

				Serial.print ("RECEIVING !\n");
				led_colors(LED_ALL, 0, 0, 255);

				err = UF.frameFromChar(buf,buflen);
				Show_UbberFrame(&UF);
				Serial.print ("err\n");
				Serial.print (err);

				if (err == 0) {
					char * str;
					const char * source;
					uint8_t source_id;
					uint8_t dest_id;
					uint8_t type;
					char exiting = 0;
					char wait_ack = 1;
					unsigned long time, time2;
					str = (char *)(UF.getPayload());
					i = UF.getPayloadSize();
					source = UF.getSourceIDString();
					source_id = UF.getSourceID();
					dest_id = UF.getDestID();
					if (dest_id != UbberFrame::DAMS) {
						if (dest_id != UbberFrame::ALL)
							goto not_for_me;
					}

					type = UF.getType();
					time = millis();
					time2 = 0;

					while (!exiting && (wait_ack == 1) && (time2<60000)) {
						i = 0;
						time2 = (millis() - time);
						switch (type) {
							case UbberFrame::PAUSE :
								Show_Max7219(SYMBOL_P);
								break;
							case UbberFrame::RENCONTRE :
								Show_Max7219(SYMBOL_CLOCHE);
								break;
							case UbberFrame::REPAS :
								Show_Max7219(SYMBOL_REPAS);
								break;
							case UbberFrame::CLOPE :
								Show_Max7219(SYMBOL_CLOPE);
								break;
							case UbberFrame::CAFE :
								Show_Max7219(SYMBOL_CAFE);
								break;
							case UbberFrame::PISCINE :
								Show_Max7219(SYMBOL_PISCINE);
								break;
							case UbberFrame::EPICURIA :
								Show_Max7219(SYMBOL_CAR);
								break;
							case UbberFrame::VELO :
								Show_Max7219(SYMBOL_DOOR);
								break;
							case UbberFrame::ACQUITTEMENT :
								Show_Max7219(SYMBOL_POUCE);
								wait_ack = 0;
								break;
							case UbberFrame::MAJ :
								Show_Max7219(SYMBOL_SPIRAL);
								break;
							default :
								Show_Max7219(SYMBOL_SMILEY_MMM);
								break;
						}

						for (j = 0 ; j < 20 ; j++) {
							Read_Button_PB(100, &exiting);
							if (exiting == 1) {
								break;
							}
						}

						while ((exiting == 0) && (source[i] != 0)) {
							if ((source[i] >= 'A') && (source[i] <= 'Z')) {
								Show_Max7219(SYMBOL_A + source[i] - 'A');
							}
							else if ((source[i] >= 'a') && (source[i] <= 'z')) {
								Show_Max7219(SYMBOL_A + source[i] - 'a');
							}
							else {
								Show_Max7219(SYMBOL_EMPTY);
							}
							for (j = 0 ; j < 3 ; j++) {
								Read_Button_PB(100, &exiting);
								if (exiting == 1) {
									break;
								}
							}
							i++;
						}

						if (exiting == 1) {
							break;
						}
					}

					if ((type != UbberFrame::ACQUITTEMENT) && (time2<60000)) {
						Show_Max7219(SYMBOL_POUCE);
						UF.setSourceID(UbberFrame::DAMS);
						UF.setDestID(source_id);
						UF.setType(UbberFrame::ACQUITTEMENT);
						delay(1000);
						sm = STATE_TRANSMIT_EMIT;
						break;
					}
				}
				else {
					sm = STATE_ABORT;
					break;
				}
			}

not_for_me:

			BoutonX = analogRead (BOUTONX_PIN);
			BoutonY = analogRead (BOUTONY_PIN);
			BoutonSW = digitalRead (BOUTONSW_PIN);

#ifdef DEBUG
			Serial.print ("Bouton\t");
			Serial.print ("X:");
			Serial.print (BoutonX);
			Serial.print ("\t");
			Serial.print ("Y:");
			Serial.print (BoutonY);
			Serial.print ("\t");
			Serial.print ("SW:");
			Serial.print (BoutonSW);
			Serial.print ("\n");
#endif

			if (BoutonSW == 0)
			{
				sm = STATE_TRANSMIT_DEST;
				delay (500);
				break;
			}

			sel_led = ((beautiful_loop>>7) & LED_ALL);
			if (sel_led != old_led) {
				led_colors((led_mask_t)(1<<sel_led), 255 , 0, 0);
				old_led = sel_led;
			}

			beautiful_loop += x;
			if (sel_led == 0x5)
				x = -1;
			if (sel_led == 0x1f)
				x = 1;
	
		}

	}
	/*****************/
	/* TRANSMIT DEST */
	/*****************/
	else if (sm == STATE_TRANSMIT_DEST)
	{
		int err;
		led_colors(LED_ALL, 255, 255, 255);
		Show_Max7219 (SYMBOL_D);	
		Serial.print ("STATE_TRANSMIT_DEST\n");
		UF.setSourceID(UbberFrame::DAMS);
		UF.setDestID(UbberFrame::GUILLAUME_L);
		if ((err = Photo_code (&code_dest)) == 0)
		{
			UF.setDestID(code_dest);
			sm = STATE_TRANSMIT_EVENT;
		}
		else
		{
			if (err != -1)
			{
				Show_Max7219 (SYMBOL_SMILEY_BAD);
				delay (2000);
			}
			else
			{
				sm = STATE_TRANSMIT_EVENT;
				delay (2000);
			}
		}
	}
	/******************/
	/* TRANSMIT EVENT */
	/******************/
	else if (sm == STATE_TRANSMIT_EVENT)
	{
		int err;
		Show_Max7219 (SYMBOL_E);
		Serial.print ("STATE_TRANSMIT_EVENT\n");
		UF.setType(UbberFrame::PAUSE);
		if ((err = Photo_code (&code_event)) == 0)
		{
			UF.setType(code_event);
			//UF.setPayload((const uint8_t *)(""),0);
			sm = STATE_TRANSMIT_EMIT;
		}
		else
		{
			if (err != -1)
			{
				Show_Max7219 (SYMBOL_SMILEY_BAD);
				delay (2000);
			}
			else
			{
				sm = STATE_TRANSMIT_EMIT;
				delay (2000);
			}
		}
	}
	/*********/
	/* ABORT */
	/*********/
	else if (sm == STATE_ABORT)
	{
		Show_Max7219 (SYMBOL_SMILEY_BAD);
		delay (2000);
		sm = STATE_RECEIVE;
	}
	/***********/
	/* UNKNOWN */
	/***********/
	else
	{
		Serial.print ("STATE_MACHINE_ISSUE\n");
		Show_Max7219 (SYMBOL_FULL);
		delay (2000);
		sm = STATE_RECEIVE;
	}

//#ifdef DEBUG
	Serial.println(freeRam());
//#endif
	
}

