#include <RH_ASK.h>
//#include <SPI.h>		// Not actualy used but needed to compile
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
#define PIXELPIN            6
#define BOUTONSW_PIN        5
#define MAX7219_CLK_PIN     7
#define MAX7219_CS_PIN      9
#define MAX7219_DIN_PIN     8

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      5

// Analog Pins
#define PHOTOR0_PIN 4		//0
#define PHOTOR1_PIN 3		//1
#define PHOTOR2_PIN 2		//2
#define PHOTOR3_PIN 1		//3
#define PHOTOR4_PIN 0		//4
#define BOUTONX_PIN 5
#define BOUTONY_PIN 6

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels =
Adafruit_NeoPixel (NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
RH_ASK RH_driver = RH_ASK (2000, 11, 12, 10, false);

unsigned char i;
unsigned char j;

UbberFrame * uf = NULL;

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

state_machine_t sm = STATE_RECEIVE;

uint16_t code_dest, code_event, code_done;

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
	SYMBOL_EMPTY,
	SYMBOL_FULL,
	SYMBOL_MAX
} disp_symbol_t;

const unsigned char disp1[SYMBOL_MAX][8] = {
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
	/*49*/ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	// Empty
	/*50*/ {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	// Full
};

void Write_Max7219_byte (unsigned char DATA);
void Write_Max7219 (unsigned char address, unsigned char dat);
void Show_Max7219 (unsigned char symbol);
void Init_MAX7219 (void);
int Photo_code (uint16_t * val);



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
		Write_Max7219 (i, disp1[(symbol % SYMBOL_MAX)][i - 1]);
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

#define PHOTO_SENSIBILITY0 820
#define PHOTO_SENSIBILITY1 820
#define PHOTO_SENSIBILITY2 820
#define PHOTO_SENSIBILITY3 820
#define PHOTO_SENSIBILITY4 820

//#define PHOTO_CAPTURE_CALIBRATION
#ifdef PHOTO_CAPTURE_CALIBRATION
opto_capture_t oc[80];
#endif

	int
Photo_code (uint16_t * val)
{
	int i;
	unsigned char err_x[5];
	unsigned char err_y[4];
	unsigned char err_x_total = 0;
	unsigned char err_y_total = 0;
	unsigned int Photo_R0, Photo_R1, Photo_R2, Photo_R3, Photo_R4;
	unsigned int Photo_R0_tmp, Photo_R1_tmp, Photo_R2_tmp, Photo_R3_tmp,
		     Photo_R4_tmp;
	unsigned long time1, time2a[5], time2b[5], time3;
	unsigned int local_val[5];
	char BoutonSW = 1;
	BoutonSW = digitalRead (BOUTONSW_PIN);


	Photo_R0 = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 1;
	/* Waiting for carte insertion all to zero */
	while ((Photo_R0 != 0) || (Photo_R1 != 0) || (Photo_R2 != 0)
			|| (Photo_R3 != 0) || (Photo_R4 != 0))
	{
		Photo_R0 = ((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY0) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("Photo1\n");
		Serial.print ("0:");
		Serial.print (Photo_R0);
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
		if ((BoutonSW = digitalRead (BOUTONSW_PIN)) == 0)
		{
			goto early_exit;
		}

	}
	time1 = micros ();
#ifdef PHOTO_CAPTURE_CALIBRATION
	//delay (1);
	for (int i = 0; i < 80; i++)
	{
		Photo_R0 = ((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY0) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
		oc[i].time = micros ();
		oc[i].val =
			Photo_R0 | Photo_R1 << 1 | Photo_R2 << 2 | Photo_R3 << 3 | Photo_R4 <<
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
		Photo_R0 = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 0;
		local_val[i] = 0;
		/* Waiting At least one hole found */
		while ((Photo_R0 == 0) && (Photo_R1 == 0) && (Photo_R2 == 0)
				&& (Photo_R3 == 0) && (Photo_R4 == 0))
		{
			Photo_R0 =
				((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY0) ? 0 : 1);
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
			Serial.print ("0:");
			Serial.print (Photo_R0);
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
			if ((BoutonSW = digitalRead (BOUTONSW_PIN)) == 0)
			{
				goto early_exit;
			}

		}
		time2a[i] = micros ();
		delayMicroseconds (2700);
		/* Sampling Values */
		Photo_R0 = ((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY0) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY2) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY3) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY4) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("PhotoSampling\n");
		Serial.print ("0:");
		Serial.print (Photo_R0);
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
		Photo_R0_tmp = Photo_R1_tmp = Photo_R2_tmp = Photo_R3_tmp =
			Photo_R4_tmp = 1;
		while ((Photo_R0_tmp != 0) || (Photo_R1_tmp != 0) || (Photo_R2_tmp != 0)
				|| (Photo_R3_tmp != 0) || (Photo_R4_tmp != 0))
		{
			Photo_R0_tmp =
				((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY0) ? 0 : 1);
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
			Serial.print ("0:");
			Serial.print (Photo_R0_tmp);
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

			if ((BoutonSW = digitalRead (BOUTONSW_PIN)) == 0)
			{
				goto early_exit;
			}

		}

		time2b[i] = micros ();
		err_x[i] =
			((((Photo_R0 + Photo_R1 + Photo_R2 + Photo_R3 + Photo_R4) % 2) ==
			  0) ? 1 : 0);
		err_x_total += err_x[i];
		local_val[i] =
			(((Photo_R1 & 0x1) << 0) | ((Photo_R2 & 0x1) << 1) |
			 ((Photo_R3 & 0x1) << 2) | ((Photo_R4 & 0x1) << 3));

	}

	//delay (1);
	Photo_R0 = Photo_R1 = Photo_R2 = Photo_R3 = Photo_R4 = 0;
	/* Waiting for carte exit all to one */
	while ((Photo_R0 != 1) || (Photo_R1 != 1) || (Photo_R2 != 1)
			|| (Photo_R3 != 1) || (Photo_R4 != 1))
	{
		Photo_R0 = ((analogRead (PHOTOR0_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R1 = ((analogRead (PHOTOR1_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R2 = ((analogRead (PHOTOR2_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R3 = ((analogRead (PHOTOR3_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
		Photo_R4 = ((analogRead (PHOTOR4_PIN) < PHOTO_SENSIBILITY1) ? 0 : 1);
#ifdef DEBUG
		Serial.print ("Photo3\n");
		Serial.print ("0:");
		Serial.print (Photo_R0);
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

		if ((BoutonSW = digitalRead (BOUTONSW_PIN)) == 0)
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

	*val =
		((((unsigned long) (local_val[0])) << 0) |
		 (((unsigned long) (local_val[1])) << 4) |
		 (((unsigned long) (local_val[2])) << 8) |
		 (((unsigned long) (local_val[3])) << 12));

	Serial.print (*val, HEX);
	Serial.print ("\n=====\n");
	for (int i = 0; i < 5; i++)
	{
		Serial.print (err_x[i]);
		Serial.print (",");
	}
	Serial.print (err_x_total);
	Serial.print ("\n-----\n");
	for (int i = 0; i < 4; i++)
	{
		Serial.print (err_y[i]);
		Serial.print (",");
	}
	Serial.print (err_y_total);
	Serial.print ("\n*****\n");

	return (err_x_total + err_y_total);

early_exit:
	return -1;
}


	void
setup ()
{
	Serial.begin (115200);	// Initialisation du port série pour avoir un retour sur le serial monitor
	pinMode (MAX7219_CLK_PIN, OUTPUT);
	pinMode (MAX7219_CS_PIN, OUTPUT);
	pinMode (MAX7219_DIN_PIN, OUTPUT);
	Serial.println ("Radiohead initialization");
	if (!RH_driver.init ())
		Serial.println ("Radiohead init failed");
	pixels.begin ();		// This initializes the NeoPixel library.

	Serial.println ("LED Matrix initialization");
	Init_MAX7219 ();
	Show_Max7219 (SYMBOL_EMPTY);
	Serial.println ("RGB LED initialization");
	for (int i = 0; i < NUMPIXELS; i++)
	{
		pixels.setPixelColor (i, pixels.Color (0, 0, 0));
	}
	pixels.show ();		// This sends the updated pixel color to the hardware.

	uf = new UbberFrame();
}


	void
loop ()
{
	int BoutonX, BoutonY;
	int beautiful_loop=0;
	char BoutonSW;
	uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
	uint8_t buflen = sizeof (buf);

	/*****************/
	/* TRANSMIT EMIT */
	/*****************/
	if (sm == STATE_TRANSMIT_EMIT)
	{
		int i;
		char * str;
		Show_Max7219 (SYMBOL_X);
		delay (1000);
		Serial.print ("STATE_TRANSMIT_EMIT\t");
		str = (char *)(uf->getPayload());
		i = uf->getPayloadSize();
		Serial.print ("\nSource\n");
		Serial.print (uf->getSourceID());
		Serial.print ("\nDestination\n");
		Serial.print (uf->getDestID());
		Serial.print ("\nType\n");
		Serial.print (uf->getType());
		Serial.print ("\nLength\n");
		Serial.print (uf->getLength());
		Serial.print ("\nPayload\n");
		while (i-- != 0)
			Serial.print(*(str++));
		Serial.print ("\n");

		RH_driver.send (uf->frameToChar(), uf->getLength());
		RH_driver.waitPacketSent ();
		sm = STATE_RECEIVE;
		uf->setSourceID(UbberFrame::DAMS);
		uf->setDestID(UbberFrame::DAMS);
		uf->setType(UbberFrame::PAYLOAD_DEFINED);
		uf->setPayload((const uint8_t *)(""),0);
	}
	/*************/
	/*  RECEIVE  */
	/*************/
	else if (sm == STATE_RECEIVE)
	{
		int x = 1;
		Show_Max7219 (SYMBOL_R);
#ifdef DEBUG
		Serial.print ("STATE_RECEIVE\n");
#endif

		while (1)
		{
			int sel_led;
			if (RH_driver.recv (buf, &buflen))	// Non-blocking
			{
				int i;
				char * str;
				// Message with a good checksum received, dump it.
				Serial.print ("RECEIVING !\n");
				for (int i = 0; i < NUMPIXELS; i++)
				{
					pixels.setPixelColor (i, pixels.Color (0, 0, 255));
				}
				pixels.show ();

				uf->frameFromChar(buf,buflen);
				str = (char *)(uf->getPayload());
				i = uf->getPayloadSize();
				Serial.print ("\nSource\n");
				Serial.print (uf->getSourceID());
				Serial.print ("\nDestination\n");
				Serial.print (uf->getDestID());
				Serial.print ("\nType\n");
				Serial.print (uf->getType());
				Serial.print ("\nLength\n");
				Serial.print (uf->getLength());
				Serial.print ("\nPayload\n");
				while (i-- != 0)
					Serial.print(*(str++));
				Serial.print ("\n");
			}

#if 0
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
				delay (200);
				break;
			}

			sel_led = ((beautiful_loop>>7)&0x1f);
			for (int i = 0; i < NUMPIXELS; i++)
			{
				pixels.setPixelColor (i, pixels.Color (0, 0, 0));
			}
			pixels.setPixelColor (sel_led, pixels.Color (255, 0, 0));

			beautiful_loop += x;
			if (sel_led == 0x5)
				x = -1;
			if (sel_led == 0x1f)
				x = 1;
			pixels.show ();
#endif

		}

	}
	/*****************/
	/* TRANSMIT DEST */
	/*****************/
	else if (sm == STATE_TRANSMIT_DEST)
	{
		int err;
		for (int i = 0; i < NUMPIXELS; i++)
		{
			pixels.setPixelColor (i, pixels.Color (255, 255, 255));
		}
		pixels.show ();
		Show_Max7219 (SYMBOL_D);
		Serial.print ("STATE_TRANSMIT_DEST\n");
		if ((err = Photo_code (&code_dest)) == 0)
		{
			//uf->setDestID(code_dest&0xFF);
			uf->setDestID(UbberFrame::GUILLAUME_L);
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
				sm = STATE_ABORT;
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
		if ((err = Photo_code (&code_event)) == 0)
		{
			uf->setType(code_event&0xFF);
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
				sm = STATE_ABORT;
			}
		}
	}
	/*********/
	/* ABORT */
	/*********/
	else if (sm == STATE_ABORT)
	{
		Show_Max7219 (SYMBOL_A);
		uf->setSourceID(UbberFrame::DAMS);
		uf->setDestID(UbberFrame::DAMS);
		uf->setType(UbberFrame::PAYLOAD_DEFINED);
		uf->setPayload((const uint8_t *)(""),0);
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
}

