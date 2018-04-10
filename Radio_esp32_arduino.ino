
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

// #include "audio.hpp"

#define PAYLOAD_SIZE 16

#define USE_RADIO
// #define USE_TFT
// #define USE_LED
#define USE_INT_ADC
// #define USE_EXT_ADC // if plug need to be on
// #define USE_SD // if plug need to be on
// #define USE_TS // if plug need to be on

#define NRF24L01_INSTALLED

// #define	BAYANG_NRF24L01_INO

#define PRINT_ADC
#define PRINT_TS
#define PRINT_RADIO
#define PRINT_SD

#define FONT_SIZE 1

uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};

uint16_t adc_value[8];

uint8_t prev_power=0xFD; // unused power value
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
	pinMode(NRF_CSN_pin, OUTPUT);
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


	#define BAYANG_BIND_COUNT		1000
	#define BAYANG_PACKET_PERIOD	1000
	#define BAYANG_INITIAL_WAIT		500
	#define BAYANG_PACKET_SIZE		16
	#define BAYANG_RF_NUM_CHANNELS	4
	#define BAYANG_RF_BIND_CHANNEL	0
	#define BAYANG_RF_BIND_CHANNEL_X16_AH 10
	#define BAYANG_ADDRESS_LENGTH	5

	#ifdef USE_RADIO
		radio.begin();
		delay(100);

		protocol_flags |= _BV(7);

		NRF24L01_Initialize();
		NRF24L01_SetTxRxMode(TX_EN);
        //
		// //XN297_SetTXAddr((uint8_t *)"\x00\x00\x00\x00\x00", BAYANG_ADDRESS_LENGTH);
        //
		NRF24L01_FlushTx();
		NRF24L01_FlushRx();
		NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70);     	// Clear data ready, data sent, and retransmit
		NRF24L01_WriteReg(NRF24L01_01_EN_AA, 0x3f);      	// Auto Acknowldgement on all data pipes
		NRF24L01_WriteReg(NRF24L01_02_EN_RXADDR, 0x03);  	// Enable data pipe 0 and 1
		NRF24L01_WriteReg(NRF24L01_11_RX_PW_P0, BAYANG_PACKET_SIZE);
		NRF24L01_SetBitrate(NRF24L01_BR_1M);             	// 1Mbps
		NRF24L01_WriteReg(NRF24L01_04_SETUP_RETR, 0x03);
		NRF24L01_SetPower();
		NRF24L01_Activate(0x73);							// Activate feature register
		NRF24L01_WriteReg(NRF24L01_1C_DYNPD, 0x00);			// Disable dynamic payload length on all pipes
		NRF24L01_WriteReg(NRF24L01_1D_FEATURE, 0x01);
		NRF24L01_Activate(0x73);

		delay(100);

		// if (radio.begin()) {
			//Serial.println("Radio init OK!");

			// radio.setPALevel(RF24_PA_LOW);
			// radio.setChannel(0);
			// radio.setAddressWidth(5);
			// radio.setDataRate(RF24_1MBPS);
			// radio.setCRCLength(RF24_CRC_16);
			// //radio.setAutoAck(false);
			// radio.setRetries(15, 7);
			// radio.setPayloadSize(PAYLOAD_SIZE);
			// radio.openWritingPipe(addresses);
            //
			// radio.powerUp();
		// NRF24L01_WriteReg(NRF24L01_05_RF_CH, 0);
			radio.printDetails();

			while(42) {
				//Serial.println("hello");
				// Serial.print(NRF24L01_ReadReg(NRF24L01_10_TX_ADDR),HEX);

						NRF24L01_WriteReg(NRF24L01_05_RF_CH, 0);

						// clear packet status bits and TX FIFO
						NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70);
						NRF24L01_FlushTx();

						// XN297_WritePayload(packet, BAYANG_PACKET_SIZE);

						uint16_t test[8];

						test[0] = 42;

						NRF24L01_WritePayload((uint8_t *)test, BAYANG_PACKET_SIZE);

						NRF24L01_SetTxRxMode(TXRX_OFF);
						NRF24L01_SetTxRxMode(TX_EN);

						// Power on, TX mode, 2byte CRC
						// Why CRC0? xn297 does not interpret it - either 16-bit CRC or nothing
						// XN297_Configure(_BV(NRF24L01_00_EN_CRC) | _BV(NRF24L01_00_CRCO) | _BV(NRF24L01_00_PWR_UP));
						NRF24L01_WriteReg(NRF24L01_00_CONFIG, (_BV(NRF24L01_00_EN_CRC) | _BV(NRF24L01_00_CRCO) | _BV(NRF24L01_00_PWR_UP)) & 0xFF);

						NRF24L01_SetPower();	// Set tx_power
						Serial.println("send");
				delay(1000);
			}
			radio_on = 1;
		// } else {
		// 	Serial.println("Radio error init");
		// }
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

	// #if defined(USE_TFT) && defined(USE_RADIO) && defined(PRINT_RADIO)
	// 	tft.println("\nRadio nRF24L01P");
	// 	if (radio_on) {
	// 		tft.print("  PA power: ");
	// 		switch (radio.getPALevel()) {
	// 			case RF24_PA_MIN:
	// 			tft.println("MIN");
	// 			break;
	// 			case RF24_PA_LOW:
	// 			tft.println("LOW");
	// 			break;
	// 			case RF24_PA_HIGH:
	// 			tft.println("HIGH");
	// 			break;
	// 			case RF24_PA_MAX:
	// 			tft.println("MAX");
	// 			break;
	// 		}
    //
	// 		tft.print("  Data rate: ");
	// 		switch (radio.getDataRate()) {
	// 			case RF24_250KBPS:
	// 			tft.println("250KBPS");
	// 			break;
	// 			case RF24_2MBPS:
	// 			tft.println("2MBPS");
	// 			break;
	// 			case RF24_1MBPS:
	// 			tft.println("1MBPS");
	// 			break;
	// 		}
    //
	// 		tft.print("  Channel: ");
	// 		tft.println(radio.getChannel());
    //
	// 		tft.print("  CRC Length: ");
	// 		switch (radio.getCRCLength()) {
	// 			case RF24_CRC_DISABLED:
	// 			tft.println("Disable");
	// 			break;
	// 			case RF24_CRC_16:
	// 			tft.println("16 bits");
	// 			break;
	// 			case RF24_CRC_8:
	// 			tft.println("8 bits");
	// 			break;
	// 		}
	// 	} else {
	// 		tft.println("  RADIO ERROR");
	// 	}
	// #endif

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

	delay(1000);
}
