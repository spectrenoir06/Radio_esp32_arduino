
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_NeoPixel.h>
#include "RF24.h"
#include <XPT2046_Touchscreen.h>

#define NEOPIXEL_PIN 5

#define TFT_DC 2
#define TFT_CS 15
#define TOUCH_CS 4

#define NRF24_CE 26
#define NRF24_CS 26
#define PAYLOAD_SIZE 16

#define RADIO_PE1 13
#define RADIO_PE2 12


//#define USE_RADIO
#define USE_TFT
#define USE_LED
#define USE_INT_ADC

uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};
// uint8_t data[PAYLOAD_SIZE];

uint16_t adc_value[8];

#ifdef USE_RADIO
	RF24 radio(NRF24_CE, NRF24_CS);
#endif

#ifdef USE_TFT
	Adafruit_ILI9341 tft   = Adafruit_ILI9341(TFT_CS, TFT_DC);
	XPT2046_Touchscreen ts(TOUCH_CS);
#endif

#ifdef USE_LED
	Adafruit_NeoPixel leds = Adafruit_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif


void setup() {
	Serial.begin(115200);

	#ifdef USE_LED
		leds.begin();
		leds.show();
		leds.setPixelColor(0, leds.Color(50,25,0));
		leds.show();
	#endif

	#ifdef USE_RADIO
		pinMode(RADIO_PE1, OUTPUT);
		pinMode(RADIO_PE2, OUTPUT);
		digitalWrite(RADIO_PE1, HIGH); // nrf24 (pE1 high, PE2 low)
		digitalWrite(RADIO_PE2, LOW);

		//SPI.setFrequency(2000000);
		//SPI.setBitOrder(MSBFIRST);
		//SPI.setDataMode(0);

		if (radio.begin())
			Serial.println("OK!");
		else
			Serial.println("Radio error init");

		Serial.println("\n");

		radio.setPALevel(RF24_PA_LOW);
		radio.setChannel(0);
		radio.setAddressWidth(5);
		radio.setDataRate(RF24_1MBPS);
		radio.setCRCLength(RF24_CRC_16);
		//radio.setAutoAck(false);
		radio.setRetries(15, 7);
		radio.setPayloadSize(PAYLOAD_SIZE);
		radio.openWritingPipe(addresses);

		radio.powerUp();
		radio.printDetails();

	#endif

	#ifdef USE_TFT
		tft.begin();
		tft.fillScreen(ILI9341_BLACK);
		ts.begin();
	#endif
}

void loop() {
	#ifdef USE_TFT
		tft.fillRect(0, 0, 320, 240, 0);
		tft.setCursor(0, 0);
		tft.setTextColor(ILI9341_WHITE,0);
		tft.setTextSize(3);
	#endif

	#ifdef USE_INT_ADC
		adc_value[0] = analogRead(36);
		adc_value[1] = analogRead(39);
		adc_value[2] = analogRead(34);
		adc_value[3] = analogRead(35);
	#endif

	#ifdef USE_TFT
		tft.println(adc_value[0]);
		tft.println(adc_value[1]);
		tft.println(adc_value[2]);
		tft.println(adc_value[3]);

		if (ts.touched()) {
			TS_Point p = ts.getPoint();
			tft.print("x: "); tft.println(p.x);
			tft.print("y: "); tft.println(p.y);
			Serial.print("x = "); Serial.println(p.x);
			Serial.print("y = "); Serial.println(p.y);
		}
	#endif

	#ifdef USE_LED
		leds.setPixelColor(0, leds.Color(
			map(adc_value[0], 0, 4095, 0, 255),
			map(adc_value[1], 0, 4095, 0, 255),
			map(adc_value[2], 0, 4095, 0, 255)));
			leds.show();
	#endif

	#ifdef USE_RADIO
		if (radio.write(adc_value, PAYLOAD_SIZE))
		Serial.print("s\n");
		else
		Serial.print("f\n");
	#endif

	delay(100);
}
