
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP3008.h>
#include <RF24.h>
#include <XPT2046_Touchscreen.h>
#include <FS.h>
#include <SD.h>

#include "audio.hpp"

#define NEOPIXEL_PIN 5

#define TFT_DC 2
#define TFT_CS 15
#define TOUCH_CS 4
#define SD_CS 16

#define ADC_CS 21

#define NRF24_CE 26
#define NRF24_CS 26
#define PAYLOAD_SIZE 16

#define RADIO_PE1 13
#define RADIO_PE2 12


//#define USE_RADIO
#define USE_TFT
#define USE_LED
//#define USE_INT_ADC
#define USE_EXT_ADC
#define USE_SD


uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};
// uint8_t data[PAYLOAD_SIZE];

uint16_t adc_value[8];

#ifdef USE_RADIO
	RF24 radio(NRF24_CE, NRF24_CS);
#endif

#ifdef USE_TFT
	Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
	XPT2046_Touchscreen ts(TOUCH_CS);
#endif

#ifdef USE_LED
	Adafruit_NeoPixel leds = Adafruit_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

#ifdef USE_EXT_ADC
	Adafruit_MCP3008 adc;
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
		tft.setRotation(1);
		tft.fillScreen(ILI9341_BLACK);
		ts.begin();
	#endif

	#ifdef USE_EXT_ADC
		adc.begin(ADC_CS);
	#endif

	#ifdef USE_SD
		if(!SD.begin(SD_CS)){
			Serial.println("Card Mount Failed");
		} else {
			uint8_t cardType = SD.cardType();

			if(cardType == CARD_NONE){
				Serial.println("No SD card attached");
			} else {
				Serial.print("SD Card Type: ");
				if(cardType == CARD_MMC){
					Serial.println("MMC");
				} else if(cardType == CARD_SD){
					Serial.println("SDSC");
				} else if(cardType == CARD_SDHC){
					Serial.println("SDHC");
				} else {
					Serial.println("UNKNOWN");
				}
				uint64_t cardSize = SD.cardSize() / (1024 * 1024);
				Serial.printf("SD Card Size: %lluMB\n", cardSize);
			}
		}
	#endif
}

void loop() {
	#ifdef USE_TFT
		tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0, 0);
		tft.setTextColor(ILI9341_WHITE,0);
		tft.setTextSize(2);
	#endif

	#ifdef USE_INT_ADC
		adc_value[0] = analogRead(36)>>2;
		adc_value[1] = analogRead(39)>>2;
		adc_value[2] = analogRead(34)>>2;
		adc_value[3] = analogRead(35)>>2;
	#endif

	#ifdef USE_EXT_ADC
		for (int chan = 0; chan < 8; chan++) {
			adc_value[chan] = adc.readADC(chan);
			//Serial.print(adc_value[chan]); Serial.print("\t");
		}
		//Serial.println();
	#endif

	#ifdef USE_TFT
		tft.println("\nADC inputs:");
		for (int chan=0;chan,chan<8;chan++) {
			tft.print("  [");
			tft.print(chan);
			tft.print("] = ");
			tft.println(adc_value[chan]);
		}

		tft.println("\nTouchScreen:");
		if (ts.touched()) {
			TS_Point p = ts.getPoint();
			tft.print("  x: "); tft.println(p.x);
			tft.print("  y: "); tft.println(p.y);
			// Serial.print("x = "); Serial.println(p.x);
			// Serial.print("y = "); Serial.println(p.y);
		} else {
			tft.println("  x:");
			tft.println("  y:");
		}
	#endif

	#ifdef USE_LED
		leds.setPixelColor(0, leds.Color(
			map(adc_value[0], 0, 1023, 0, 255),
			map(adc_value[1], 0, 1023, 0, 255),
			map(adc_value[2], 0, 1023, 0, 255)));
			leds.show();
	#endif

	#ifdef USE_RADIO
		if (radio.write(adc_value, PAYLOAD_SIZE))
			Serial.print("s\n");
		else
			Serial.print("f\n");
	#endif

	delay(2000);
}
