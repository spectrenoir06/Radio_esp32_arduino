
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP3008.h>
#include <RF24.h>
#include <XPT2046_Touchscreen.h>
#include <FS.h>
#include <SD.h>
#include "pins.h"
#include "Multiprotocol.h"
#include "iface_nrf24l01.h"
#include "config.h"
#include "TX_Def.h"

// #include "audio.hpp"

#define PAYLOAD_SIZE 16

//Global constants/variables
uint32_t MProtocol_id;//tx id,
uint32_t MProtocol_id_master;
uint32_t blink=0,last_signal=0;
//
uint16_t counter;
uint8_t  channel;
uint8_t  packet[40];

#define NUM_CHN 16
// Servo data
uint16_t Channel_data[NUM_CHN];
uint8_t  Channel_AUX;
#ifdef FAILSAFE_ENABLE
	uint16_t Failsafe_data[NUM_CHN];
#endif

//Serial protocol
uint8_t sub_protocol;
uint8_t protocol;
uint8_t option;
uint8_t cur_protocol[3];
uint8_t prev_option;
uint8_t prev_power=0xFD; // unused power value


// Protocol variables
uint8_t  cyrfmfg_id[6];//for dsm2 and devo
uint8_t  rx_tx_addr[5];
uint8_t  rx_id[4];
uint8_t  phase;
uint16_t bind_counter;
uint8_t  bind_phase;
uint8_t  binding_idx;
uint16_t packet_period;
uint8_t  packet_count;
uint8_t  packet_sent;
uint8_t  packet_length;
uint8_t  hopping_frequency[50];
uint8_t  *hopping_frequency_ptr;
uint8_t  hopping_frequency_no=0;
uint8_t  rf_ch_num;
uint8_t  throttle, rudder, elevator, aileron;
uint8_t  flags;
uint16_t crc;
uint8_t  crc8;
uint16_t seed;
uint16_t failsafe_count;
//
uint16_t state;
uint8_t  len;
uint8_t  RX_num;
uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};

uint16_t adc_value[8];

uint8_t mode_select;
uint8_t protocol_flags=0,protocol_flags2=0;

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

#ifdef USE_RADIO
	RF24 radio(NRF_CE_pin, NRF_CSN_pin);
	uint8_t radio_on = 0;
#endif

#ifdef USE_SD
	uint8_t cardType;
	uint8_t cardState;
	uint64_t cardSize;
#endif

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
	#endif

	#ifdef USE_TFT
		tft.begin(40000000); // spi speed 40Mhz
	#endif

	#ifdef USE_TS
		ts.begin();
	#endif

	#ifdef USE_EXT_ADC
		adc.begin(ADC_CSN_pin);
	#endif

	#ifdef USE_SD
		if(!SD.begin(SD_CSN_pin)){
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

	initBAYANG();
	delay(5);

	Serial.println("Init");
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

	BAYANG_callback();
	delay(5);
}
