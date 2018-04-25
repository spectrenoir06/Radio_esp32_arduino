
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP3008.h>
#include <RF24.h>
#include <XPT2046_Touchscreen.h>
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "pins.h"
#include "Multiprotocol.h"
#include "iface_nrf24l01.h"
#include "config.h"
#include "TX_Def.h"
#include "wifi_config.h"

// #include "audio.hpp"

#define PAYLOAD_SIZE 16

#ifdef USE_TFT
	Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CSN_pin, TFT_DC_pin);
#endif

#ifdef USE_TS
	XPT2046_Touchscreen ts(TOUCH_CSN_pin);
#endif

#ifdef USE_LED
	Adafruit_NeoPixel leds = Adafruit_NeoPixel(4, NEOPIXEL_pin, NEO_GRB + NEO_KHZ800);
#endif

#ifdef USE_EXT_ADC
	Adafruit_MCP3008 adc;
#endif

#ifdef USE_WIFI

#endif

uint16_t adc_value[8];

void setup() {
	Serial.begin(115200);
	SPI.begin();

	pinMode(TFT_CSN_pin, OUTPUT);
	pinMode(TOUCH_CSN_pin, OUTPUT);
	pinMode(ADC_CSN_pin, OUTPUT);
	pinMode(SD_CSN_pin, OUTPUT);

	pinMode(NRF_CSN_pin, OUTPUT);
	pinMode(CC25_CSN_pin, OUTPUT);
	pinMode(CYRF_CSN_pin, OUTPUT);
	pinMode(A7105_CSN_pin, OUTPUT);
	pinMode(CYRF_RST_pin, OUTPUT);

	pinMode(PE1_pin, OUTPUT);
	pinMode(PE2_pin, OUTPUT);

	digitalWrite(TFT_CSN_pin, HIGH);
	digitalWrite(TOUCH_CSN_pin, HIGH);
	digitalWrite(ADC_CSN_pin, HIGH);
	digitalWrite(SD_CSN_pin, HIGH);

	PE1_on;
	PE2_off;
	A7105_CSN_on;
	CC25_CSN_on;
	NRF_CSN_on;
	CYRF_CSN_on;
	CYRF_RST_HI; //reset cyrf

	#ifdef USE_LED
		leds.begin();
		leds.show();
		delay(10);
		leds.setPixelColor(0, leds.Color(255, 0, 0));
		leds.show();
		delay(500);
		leds.setPixelColor(0, leds.Color(255, 255, 0));
		leds.show();
		delay(500);
		leds.setPixelColor(0, leds.Color(0, 0, 255));
		leds.show();
		delay(500);
		leds.setPixelColor(0, leds.Color(255, 255, 255));
		leds.show();
		delay(500);
		leds.setPixelColor(0, leds.Color(0, 0, 0));
		leds.show();
		delay(500);
	#endif

	#ifdef USE_TFT
		tft.begin(40000000); // spi speed 40Mhz
	#endif

	#ifdef USE_TS
		ts.begin();
	#endif

	#ifdef USE_ADC
		init_adc();
	#endif

	#ifdef USE_TFT
		tft.setRotation(1);
		tft.fillScreen(ILI9341_BLACK);
		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		tft.setTextSize(FONT_SIZE);
	#endif

	#ifdef USE_RADIO
		init_radio();
	#endif

	#ifdef USE_WIFI
		init_wifi();
	#endif

	#ifdef USE_SD
		init_sd();
	#endif

	// initBAYANG();
	// delay(5);
}

void update_tft() {

	#ifdef USE_TFT
		// tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0, 0);
	#endif

	#if defined(USE_TFT) && defined(PRINT_ADC)
		tft.print("\nADC inputs: ");
		#ifdef USE_EXT_ADC
			tft.println("(EXT)");
		#else
			tft.println("(INT)");
		#endif
		for (int chan=0;chan,chan<8;chan++) {
			tft.print("  [");
			tft.print(chan);
			tft.print("] = ");
			tft.print(adc_value[chan]);
			tft.println("   ");
		}
	#endif

	#if defined(USE_TFT) && defined(USE_TS) && defined(PRINT_TS)
		tft.println("\nTouchScreen:");
		if (ts.touched()) {
			TS_Point p = ts.getPoint();
			tft.print("  x: "); tft.print(p.x); tft.println("   ");
			tft.print("  y: "); tft.print(p.y); tft.println("   ");
			tft.print("  z: "); tft.print(p.z); tft.println("   ");
			// tft.drawCircle((int16_t)((p.x - 300) * .09), (int16_t)(p.y * .11376), 2, ILI9341_RED);
			// Serial.print("x = "); Serial.println(p.x);
			// Serial.print("y = "); Serial.println(p.y);
		} else {
			tft.println("  x:       ");
			tft.println("  y:       ");
			tft.println("  z:       ");
		}
	#endif


	// print_info_sd();
}

void loop() {

	#if  defined(USE_INT_ADC)
		Channel_data[0] = map16b(analogRead(36), 0, 4095, CHANNEL_MIN_100, CHANNEL_MAX_100);
		Channel_data[1] = map16b(analogRead(39), 0, 4095, CHANNEL_MIN_100, CHANNEL_MAX_100);
		Channel_data[2] = map16b(analogRead(34), 0, 4095, CHANNEL_MIN_100, CHANNEL_MAX_100);
		Channel_data[3] = map16b(analogRead(35), 0, 4095, CHANNEL_MIN_100, CHANNEL_MAX_100);
		Channel_data[4] = 127;
		Channel_data[5] = 255;
		Channel_data[6] = 511;
		Channel_data[7] = 1023;
	#endif

	#ifdef USE_EXT_ADC
		for (int chan = 0; chan < 8; chan++) {
			adc_value[chan] = adc.readADC(chan);
			// Serial.print(adc_value[chan]); Serial.print("\t");
		}
		// Serial.println();
	#endif

	#ifdef USE_LED
		// leds.setPixelColor(0, leds.Color(
		// 	map(adc_value[0], 0, 1023, 0, 255),
		// 	0,
		// 	0
		// ));
		// leds.setPixelColor(1, leds.Color(
		// 	0,
		// 	map(adc_value[1], 0, 1023, 0, 255),
		// 	0
		// ));
		// leds.setPixelColor(2, leds.Color(
		// 	0,
		// 	0,
		// 	map(adc_value[2], 0, 1023, 0, 255)
		// ));
		// leds.setPixelColor(3, leds.Color(
		// 	map(adc_value[3], 0, 1023, 0, 255),
		// 	map(adc_value[3], 0, 1023, 0, 255),
		// 	0
		// ));
		// leds.show();
	#endif

	#if defined(USE_RADIO) && defined(RADIO_SEND)
		radio_send();
	#endif

	#if defined(USE_WIFI) && defined(WIFI_SEND)
		wifi_udp_send();
	#endif

	update_tft();

	// while(1) {
	// 	int x = 320-10;
	// 	int y = 240-10;
	// 	tft.drawCircle(x, y, 2, ILI9341_RED);
	// 	while (!ts.touched());
	// 	TS_Point p = ts.getPoint();
	// 	Serial.print("  x: "); Serial.print(p.x); Serial.println("   ");
	// 	Serial.print("  y: "); Serial.print(p.y); Serial.println("   ");
	// 	delay(1000);
	// }


	// BAYANG_callback();
	delay(5);
}
