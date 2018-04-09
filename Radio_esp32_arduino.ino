
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP3008.h>
#include <RF24.h>
#include <XPT2046_Touchscreen.h>
#include <FS.h>
#include <SD.h>

// #include "audio.hpp"

#define OLD

#define NEOPIXEL_PIN 5

#define TFT_DC 2
#define TFT_CS 15
#define TOUCH_CS 4
#define SD_CS 16

#define ADC_CS 21

#define NRF24_CE 35 // not use

#ifdef OLD
	#define NRF24_CS 26
#else
	#define NRF24_CS 33
#endif

#define CC25_CS 27

#define A7105_CS 14

#ifdef OLD
	#define CYRF_CS 25
#else
	#define CYRF_CS 32
#endif

#define CYRF_RST 17

#define PAYLOAD_SIZE 16

#define RADIO_PE1 13
#define RADIO_PE2 12


#define USE_RADIO
#define USE_TFT
#define USE_LED
#define USE_INT_ADC
// #define USE_EXT_ADC // if plug need to be on
#define USE_SD // if plug need to be on
#define USE_TS // if plug need to be on

#define PRINT_ADC
#define PRINT_TS
#define PRINT_RADIO
#define PRINT_SD

#define FONT_SIZE 1

uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};

uint16_t adc_value[8];

#ifdef USE_TFT
	Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#endif

#ifdef USE_TS
	XPT2046_Touchscreen ts(TOUCH_CS);
#endif

#ifdef USE_LED
	Adafruit_NeoPixel leds = Adafruit_NeoPixel(4, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

#ifdef USE_EXT_ADC
	Adafruit_MCP3008 adc;
#endif

#ifdef USE_RADIO
	RF24 radio(NRF24_CE, NRF24_CS);
	uint8_t radio_on = 0;
#endif

#ifdef USE_SD
	uint8_t cardType;
	uint8_t cardState;
	uint64_t cardSize;
#endif

void setup() {
	Serial.begin(115200);

	pinMode(TFT_CS, OUTPUT);
	pinMode(TOUCH_CS, OUTPUT);
	pinMode(ADC_CS, OUTPUT);
	pinMode(SD_CS, OUTPUT);
	pinMode(CC25_CS, OUTPUT);
	pinMode(CYRF_CS, OUTPUT);
	pinMode(A7105_CS, OUTPUT);
	pinMode(CYRF_RST, OUTPUT);

	pinMode(RADIO_PE1, OUTPUT);
	pinMode(RADIO_PE2, OUTPUT);

	digitalWrite(TFT_CS, HIGH);
	digitalWrite(TOUCH_CS, HIGH);
	digitalWrite(ADC_CS, HIGH);
	digitalWrite(SD_CS, HIGH);
	digitalWrite(CC25_CS, HIGH);
	digitalWrite(CYRF_CS, HIGH);
	digitalWrite(A7105_CS, HIGH);
	digitalWrite(CYRF_RST, HIGH);

	digitalWrite(RADIO_PE1, HIGH); // nrf24 (PE1 high, PE2 low)
	digitalWrite(RADIO_PE2, LOW);

	#ifdef USE_LED
		leds.begin();
		leds.show();
	#endif

	#ifdef USE_TFT
		tft.begin(40000000); // spi speed 40Mhz
	#endif

	#ifdef USE_TS
		ts.begin();
	#endif

	#ifdef USE_EXT_ADC
		adc.begin(ADC_CS);
	#endif

	#ifdef USE_SD
		if(!SD.begin(SD_CS)){
			Serial.println("Card Mount Failed");
			cardState = 0;
		} else {
			cardType = SD.cardType();
			if(cardType == CARD_NONE){
				Serial.println("No SD card attached");
				cardState = 1;
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
				cardSize = SD.cardSize() / (1024 * 1024);
				cardState = 2;
				Serial.printf("SD Card Size: %lluMB\n", cardSize);
			}
		}
	#endif

	#ifdef USE_TFT
		tft.setRotation(3);
		tft.fillScreen(ILI9341_BLACK);
		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		tft.setTextSize(FONT_SIZE);
	#endif

	#ifdef USE_RADIO
		delay(100);

		if (radio.begin()) {
			Serial.println("Radio init OK!");

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
			radio_on = 1;
		} else {
			Serial.println("Radio error init");
		}
	#endif
}

void loop() {

	#if  defined(USE_INT_ADC)
		adc_value[0] = analogRead(36)>>2;
		adc_value[1] = analogRead(39)>>2;
		adc_value[2] = analogRead(34)>>2;
		adc_value[3] = analogRead(35)>>2;
		adc_value[4] = 127;
		adc_value[5] = 255;
		adc_value[6] = 511;
		adc_value[7] = 1023;
	#endif

	#ifdef USE_EXT_ADC
		for (int chan = 0; chan < 8; chan++) {
			adc_value[chan] = adc.readADC(chan);
			//Serial.print(adc_value[chan]); Serial.print("\t");
		}
		//Serial.println();
	#endif

	#ifdef USE_LED
		leds.setPixelColor(0, leds.Color(
			map(adc_value[0], 0, 1023, 0, 255),
			0,
			0
		));
		leds.setPixelColor(1, leds.Color(
			0,
			map(adc_value[1], 0, 1023, 0, 255),
			0
		));
		leds.setPixelColor(2, leds.Color(
			0,
			0,
			map(adc_value[2], 0, 1023, 0, 255)
		));
		leds.setPixelColor(3, leds.Color(
			map(adc_value[3], 0, 1023, 0, 255),
			map(adc_value[3], 0, 1023, 0, 255),
			0
		));
		leds.show();
	#endif

	#if defined(USE_TFT) && defined(PRINT_ADC)
		// tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0, 0);
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
			// Serial.print("x = "); Serial.println(p.x);
			// Serial.print("y = "); Serial.println(p.y);
		} else {
			tft.println("  x:       ");
			tft.println("  y:       ");
		}
	#endif

	#if defined(USE_TFT) && defined(USE_RADIO) && defined(PRINT_RADIO)
		tft.println("\nRadio nRF24L01P");
		if (radio_on) {
			tft.print("  PA power: ");
			switch (radio.getPALevel()) {
				case RF24_PA_MIN:
				tft.println("MIN");
				break;
				case RF24_PA_LOW:
				tft.println("LOW");
				break;
				case RF24_PA_HIGH:
				tft.println("HIGH");
				break;
				case RF24_PA_MAX:
				tft.println("MAX");
				break;
			}

			tft.print("  Data rate: ");
			switch (radio.getDataRate()) {
				case RF24_250KBPS:
				tft.println("250KBPS");
				break;
				case RF24_2MBPS:
				tft.println("2MBPS");
				break;
				case RF24_1MBPS:
				tft.println("1MBPS");
				break;
			}

			tft.print("  Channel: ");
			tft.println(radio.getChannel());

			tft.print("  CRC Length: ");
			switch (radio.getCRCLength()) {
				case RF24_CRC_DISABLED:
				tft.println("Disable");
				break;
				case RF24_CRC_16:
				tft.println("16 bits");
				break;
				case RF24_CRC_8:
				tft.println("8 bits");
				break;
			}
		} else {
			tft.println("  RADIO ERROR");
		}
	#endif

	#if defined(USE_TFT) && defined(USE_SD) && defined(PRINT_SD)
		tft.println("\nSD card:");
		if (!cardState) {
			tft.println("  Error loading SD");
		} else {
			if(cardType == CARD_NONE){
				tft.println("  No card attached");
			} else {
				tft.print("  Type: ");
				if(cardType == CARD_MMC){
					tft.println("MMC");
				} else if(cardType == CARD_SD){
					tft.println("SDSC");
				} else if(cardType == CARD_SDHC){
					tft.println("SDHC");
				} else {
					tft.println("UNKNOWN");
				}
				tft.print("  Size:");
				tft.print((uint32_t)cardSize);
				tft.println("MB");
			}
		}
	#endif

	#ifdef USE_RADIO
		if (radio.write(adc_value, PAYLOAD_SIZE))
			Serial.print("Radio send success\n");
		else
			Serial.print("Radio send failed\n");
	#endif

	delay(10);
}
