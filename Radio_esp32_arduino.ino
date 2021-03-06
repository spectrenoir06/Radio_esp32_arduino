
#include <SPI.h>
#include <Adafruit_GFX.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>

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
#include "iface_cc2500.h"

#include "config.h"
#include "TX_Def.h"

#ifdef USE_WIFI
	#include "wifi_config.h"
#endif

// #include "GUIslice.h"
// #include "GUIslice_ex.h"
// #include "GUIslice_drv.h"

// #include "audio.hpp"

#ifdef USE_BLE
#include <BleGamepad.h>
	BleGamepad bleGamepad("Multiprotocol TX", "Spectre", 100);
#endif

#define PAYLOAD_SIZE 32

//Global constants/variables
uint32_t MProtocol_id;//tx id,
uint32_t MProtocol_id_master;
uint32_t blink=0,last_signal=0;
uint8_t calData[48];
#define MAX_PKT 29
uint8_t pkt[MAX_PKT];//telemetry receiving packets
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

//Channel mapping for protocols
const uint8_t CH_AETR[]={AILERON, ELEVATOR, THROTTLE, RUDDER, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};
const uint8_t CH_TAER[]={THROTTLE, AILERON, ELEVATOR, RUDDER, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};
const uint8_t CH_RETA[]={RUDDER, ELEVATOR, THROTTLE, AILERON, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};
const uint8_t CH_EATR[]={ELEVATOR, AILERON, THROTTLE, RUDDER, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16};

//Serial protocol
uint8_t sub_protocol = H8S3D;
uint8_t protocol;
uint8_t option;
uint8_t cur_protocol[3];
uint8_t prev_option;
uint8_t prev_power=0xFD; // unused power value

uint8_t pass = 0;
uint8_t pktt[MAX_PKT];//telemetry receiving packets

#define TXBUFFER_SIZE 96
volatile uint8_t tx_buff[TXBUFFER_SIZE];
volatile uint8_t tx_head=0;
volatile uint8_t tx_tail=0;

uint8_t v_lipo1;
uint8_t v_lipo2;
uint8_t RX_RSSI;
uint8_t TX_RSSI;
uint8_t RX_LQI;
uint8_t TX_LQI;
uint8_t telemetry_link=0;
uint8_t telemetry_counter=0;
uint8_t telemetry_lost;


// Protocol variables
uint8_t  cyrfmfg_id[6];//for dsm2 and devo
uint8_t  rx_tx_addr[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
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
uint16_t state = 0;
uint8_t  len;
uint8_t  RX_num = 0;
uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};

// uint16_t adc_value[8];

uint8_t mode_select;
uint8_t protocol_flags=0,protocol_flags2=0;

char*	text[16] = {
	"ADC1:",
	"ADC2:",
	"ADC3:",
	"ADC4:",
	"ADC5:",
	"ADC6:",
	"ADC7:",
	"ADC8:",
	"ADC9:",
	"ADC10:",
	"ADC11:",
	"ADC12:",
	"ADC13:",
	"ADC14:",
	"ADC15:",
	"ADC16:"
};

void delayMilliseconds(uint32_t t) {
	delay(t);
}

// Convert 32b id to rx_tx_addr
static void set_rx_tx_addr(uint32_t id)
{ // Used by almost all protocols
	rx_tx_addr[0] = (id >> 24) & 0xFF;
	rx_tx_addr[1] = (id >> 16) & 0xFF;
	rx_tx_addr[2] = (id >>  8) & 0xFF;
	rx_tx_addr[3] = (id >>  0) & 0xFF;
	rx_tx_addr[4] = (rx_tx_addr[2]&0xF0)|(rx_tx_addr[3]&0x0F);
}

void modules_reset()
{
	#ifdef	CC2500_INSTALLED
		CC2500_Reset();
	#endif
	#ifdef	A7105_INSTALLED
		A7105_Reset();
	#endif
	#ifdef	CYRF6936_INSTALLED
		CYRF_Reset();
	#endif
	#ifdef	NRF24L01_INSTALLED
		NRF24L01_Reset();
	#endif

	//Wait for every component to reset
	delay(100);
	prev_power=0xFD;		// unused power value
}

#ifdef USE_TFT
	Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CSN_pin, TFT_DC_pin);
#endif

#ifdef USE_TS
	XPT2046_Touchscreen ts(TOUCH_CSN_pin);
#endif

#ifdef USE_LED
	Adafruit_NeoPixel leds = Adafruit_NeoPixel(1, NEOPIXEL_pin, NEO_GRB + NEO_KHZ800);
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
	char *str_cardType;
	char *str_cardState;
	char *str_cardSize;
#endif

#ifdef USE_WIFI
	const char * networkName = WIFI_SSID;
	const char * networkPswd = WIFI_PASSWORD;
	const char * udpAddress = "192.168.0.24";
	const int udpPort = 1234;

	boolean connected = false;
	WiFiUDP udp;
#endif

#ifdef USE_LED
	uint32_t Wheel(byte WheelPos) {
		if (WheelPos < 85) {
			return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
		} else if(WheelPos < 170) {
			WheelPos -= 85;
			return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
		} else {
			WheelPos -= 170;
			return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
		}
	}
#endif

void setup() {
	Serial.begin(115200);
	// SPI.setFrequency(5000000);
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

	pinMode(BTN1_pin, INPUT_PULLUP);
	pinMode(BTN2_pin, INPUT_PULLUP);

	MProtocol_id_master = ESP.getEfuseMac();
	MProtocol_id = RX_num + MProtocol_id_master;
	set_rx_tx_addr(MProtocol_id);

	PE_SET; // config.h

	A7105_CSN_on;
	CC25_CSN_on;
	NRF_CSN_on;
	CYRF_CSN_on;
	CYRF_RST_HI; //reset cyrf

	analogReadResolution(11);             // Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
	analogSetWidth(11);                   // Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits

	#ifdef USE_LED
		leds.begin();
		leds.setBrightness(50);
		leds.setPixelColor(0, leds.Color(0,0,0));
		leds.show();
		Serial.println("USE_LED");
	#endif

	#ifdef USE_EXT_ADC
		adc.begin(ADC_CSN_pin);
		Serial.println("USE_EXT_ADC");
	#endif

	for(uint8_t i=0; i< 16; i++) {
		Channel_data[i] = CHANNEL_MIN_100;
	}

	#ifdef USE_SD
		Serial.println("USE_SD");
		if(!SD.begin(SD_CSN_pin)){
			Serial.println("Card Mount Failed");
			cardState = 0;
		} else {
			cardType = SD.cardType();
			if(cardType == CARD_NONE){
				Serial.println("No SD card attached");
				str_cardType = strdup("No SD card attached");
				// menu.items[0].data = strdup("No SD card attached");
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
		Serial.println("USE_TFT");
		tft.begin();
		tft.setRotation(3); // v3 = 3
		tft.fillScreen(ILI9341_BLACK);
		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		tft.setTextSize(FONT_SIZE);
	#endif

	#ifdef USE_WIFI
		Serial.println("USE_WIFI");
		Serial.println("Connecting to WiFi network: " + String(networkName));
		WiFi.disconnect(true);
		WiFi.onEvent(WiFiEvent);
		WiFi.begin(networkName, networkPswd);
	#endif

	#ifdef USE_BLE
		Serial.println("Starting BLE work!");
		bleGamepad.begin();
	#endif

	modules_reset();
	// delay(500);

	// StartFrskyD();

	if (digitalRead(BTN1_pin) == 0)
		BIND_IN_PROGRESS;		// Request bind
	else
		BIND_DONE;

	INIT_RF; // config.h

	delay(5);
}

uint8_t color = 0;
uint8_t btn1_press = 0;
uint8_t btn2_press = 0;

#ifdef USE_TFT
	void update_tft() {

		// tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0, 0);

		#if defined(PRINT_ADC)
			tft.print("\nADC inputs: \n");
			for (int chan=0;chan,chan<16;chan++) {
				tft.printf("  [%02d] = %04d [%03d%%]\n", chan, Channel_data[chan], map(Channel_data[chan],CHANNEL_MIN_100, CHANNEL_MAX_100, 0, 100));
			}
		#endif

		#if defined(USE_TS) && defined(PRINT_TS)
			tft.println("\nTouchScreen:");
			if (ts.touched()) {
				TS_Point p = ts.getPoint();
				tft.println("\n Raw:");
				tft.print("  x: "); tft.print(p.x); tft.println("  ");
				tft.print("  y: "); tft.print(p.y); tft.println("  ");
				tft.print("  z: "); tft.print(p.z); tft.println("  ");
				// TS_Point p = ts.getPointCalc();
				// tft.println("\nCalibrate:");
				// tft.print("  x: "); tft.print(p.x); tft.println("     ");
				// tft.print("  y: "); tft.print(p.y); tft.println("     ");
				// tft.print("  z: "); tft.print(p.z); tft.println("     ");
				// tft.fillCircle(p.x, p.y, 2, ILI9341_RED);

				// Serial.print("x = "); Serial.println(p.x);
				// Serial.print("y = "); Serial.println(p.y);
			} else {
				tft.println("\n Raw:");
				tft.println("  x:       ");
				tft.println("  y:       ");
				tft.println("  z:       ");
				// tft.println("\nCalibrate:");
				// tft.println("  x:       ");
				// tft.println("  y:       ");
				// tft.println("  z:       ");
			}
		#endif

		#if defined(USE_WIFI) && defined(PRINT_WIFI)
			tft.printf("\nWifi:\n  SSID: %s\n  IP: ", WIFI_SSID);
			if (connected)
				tft.println(WiFi.localIP());
			else
				tft.println("disconnected");
		#endif
	}
#endif

void loop() {

	#ifdef USE_INT_ADC
		// Channel_data[0] = map16b(analogRead(34), 0, 2047, CHANNEL_MIN_100, CHANNEL_MAX_100); // Roll A
		// Channel_data[1] = map16b(analogRead(35), 0, 2047, CHANNEL_MIN_100, CHANNEL_MAX_100); // Pith E
		// Channel_data[2] = map16b(analogRead(36), 0, 2047, CHANNEL_MIN_100, CHANNEL_MAX_100); // Thro T
		// Channel_data[3] = map16b(analogRead(39), 0, 2047, CHANNEL_MIN_100, CHANNEL_MAX_100); // Yaw  R

		Channel_data[0] = analogRead(36)>>1; // Roll A
		Channel_data[1] = analogRead(39)>>1; // Pith E
		Channel_data[2] = analogRead(34)>>1; // Thro T
		Channel_data[3] = analogRead(35)>>1; // Yaw  R

	#else
		Channel_data[0] = 1023; // Roll A
		Channel_data[1] = 1023; // Pith E
		Channel_data[2] = 1023; // Thro T
		Channel_data[3] = 1023; // Yaw  R
	#endif

	if (digitalRead(BTN1_pin) == 0) {
		if (btn1_press == 0) {
			Channel_data[ 4] = Channel_data[ 4] < 1500 ? CHANNEL_MAX_100 : CHANNEL_MIN_100; // aux 1
			btn1_press = 1;
		}
	} else
		btn1_press = 0;

	if (digitalRead(BTN2_pin) == 0) {
		if (btn2_press == 0) {
			Channel_data[ 5] = Channel_data[ 5] < 1500 ? CHANNEL_MAX_100 : CHANNEL_MIN_100; // aux 2
			btn2_press = 1;
		}
	} else
		btn2_press = 0;

	// Channel_data[ 4] = CHANNEL_MIN_100; // aux 1
	// Channel_data[ 5] = CHANNEL_MIN_100; // aux 2
	Channel_data[ 6] = CHANNEL_MIN_100; // aux 3
	Channel_data[ 7] = CHANNEL_MIN_100; // aux 4

	// for (int i = 0; i < 5; i++) {
	// 	if (Channel_data[i] >= 512) {
	// 		Channel_data[i] = (map(Channel_data[i], 512, 1023, 1500, 2000));
	// 	} else {
	// 		Channel_data[i] = (map(Channel_data[i], 0, 511, 1000, 1499));
	// 	}
	// }

	#ifdef USE_EXT_ADC // 10bit MCP3008
		for (int chan = 8; chan < 16; chan++)
			Channel_data[chan] = map(adc.readADC(chan), 0x00, 0x3FF, 0x00, 0xFFFF);
	#else
		for (int chan = 8; chan < 16; chan++)
			Channel_data[chan] = CHANNEL_MIN_100;
	#endif

	#ifdef ADC_SERIAL_OUTPUT
		for (int i=0;i<16;i++)
			Serial.printf("%d\t", Channel_data[i]);
		Serial.print("\n");
	#endif

	#if defined(USE_WIFI) && defined(WIFI_SEND)
		if(connected){
			udp.beginPacket(udpAddress, udpPort);
				udp.write((uint8_t *)Channel_data, PAYLOAD_SIZE);
			udp.endPacket();
		}
	#endif

	#ifdef USE_LED
		if (Channel_data[4] > 1500)
			leds.setPixelColor(0, leds.Color(255,0,0));
		else
			leds.setPixelColor(0, leds.Color(0,255,0));
			// leds.setPixelColor(0, Wheel(map(Channel_data[2], CHANNEL_MIN_100, CHANNEL_MAX_100, 0x00, 0xFF)));
		leds.show();
	#endif


	#ifdef USE_TFT
		update_tft();
	#endif

	#ifdef USE_BLE
		bleGamepad.setAxes(
			map(Channel_data[0],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			map(Channel_data[1],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			map(Channel_data[2],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			map(Channel_data[3],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			map(Channel_data[4],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			map(Channel_data[5],CHANNEL_MIN_100,CHANNEL_MAX_100, -127, 127),
			DPAD_CENTERED
		);
		delay(30);
	#endif

	// TX_MAIN_PAUSE_on;
	CALLBACK_RF; // config.h
	// TX_MAIN_PAUSE_off;

	//Serial.printf("%d,\t%d,\t%d,\t%d,\t%d,\n",telemetry_link,TX_RSSI,TX_LQI,packet_count,state);
}

#ifdef USE_WIFI
	//wifi event handler
	void WiFiEvent(WiFiEvent_t event){
		// update_tft();
		switch(event) {
			case SYSTEM_EVENT_STA_GOT_IP:
				//When connected set
				Serial.print("WiFi connected! IP address: ");
				Serial.println(WiFi.localIP());
				connected = true;
				break;
			case SYSTEM_EVENT_STA_DISCONNECTED:
				Serial.println("WiFi lost connection");
				connected = false;
			break;
		}
	}
#endif
