
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
#include "config.h"
#include "TX_Def.h"
#include "wifi_config.h"

#include "GUIslice.h"
#include "GUIslice_ex.h"
#include "GUIslice_drv.h"

// #include "audio.hpp"

#define PAYLOAD_SIZE 16

<<<<<<< HEAD
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
uint8_t sub_protocol = H8S3D;
uint8_t protocol;
uint8_t option;
uint8_t cur_protocol[3];
uint8_t prev_option;
uint8_t prev_power=0xFD; // unused power value


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
uint16_t state;
uint8_t  len;
uint8_t  RX_num;
uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};

// uint16_t adc_value[8];

uint8_t mode_select;
uint8_t protocol_flags=0,protocol_flags2=0;


uint8_t item_select = 0;
uint8_t item_select_old = 0;


// Enumerations for pages, elements, fonts, images
enum {
	E_PG_MAIN,
	E_PG_ADC,
};

enum {
	E_ELEM_BOX,
	E_ELEM_PROGRESS0,
	E_ELEM_PROGRESS1,
	E_ELEM_PROGRESS2,
	E_ELEM_PROGRESS3,
	E_ELEM_PROGRESS4,
	E_ELEM_PROGRESS5,
	E_ELEM_PROGRESS6,
	E_ELEM_PROGRESS7,
	E_SLIDER_R,
	E_SLIDER_G,
	E_SLIDER_B,
};

enum {
	E_FONT_BTN,
	E_FONT_TXT
};

enum {
	E_GROUP1
};

uint8_t   m_nPosR = 0;
uint8_t   m_nPosG = 0;
uint8_t   m_nPosB = 0;

char*	text[8] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"ADC4",
	"ADC5",
	"ADC6",
	"ADC7",
	"ADC8"
};


// Instantiate the GUI
#define MAX_PAGE                1
#define MAX_FONT                2

// Define the maximum number of elements per page
#define MAX_ELEM_PG_MAIN          50                                        // # Elems total
#define MAX_ELEM_PG_MAIN_RAM      MAX_ELEM_PG_MAIN                          // # Elems in RAM

gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsPage                 m_asPage[MAX_PAGE];
gslc_tsElem                 m_asPageElem[MAX_ELEM_PG_MAIN_RAM];
gslc_tsElemRef              m_asPageElemRef[MAX_ELEM_PG_MAIN];

gslc_tsXGauge               m_sXGauge[8];

gslc_tsElemRef*             m_pElemProgress[8];

// Define debug message function
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

// Create page elements
bool InitOverlays()
{
	gslc_tsElemRef* pElemRef;

	gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPageElem,MAX_ELEM_PG_MAIN_RAM,m_asPageElemRef,MAX_ELEM_PG_MAIN);

	// Background flat color
	gslc_SetBkgndColor(&m_gui,GSLC_COL_GRAY_DK2);

	// Create background box
	pElemRef = gslc_ElemCreateBox(&m_gui,E_ELEM_BOX,E_PG_MAIN,(gslc_tsRect){10,10,300,220});
	gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_WHITE,GSLC_COL_BLACK,GSLC_COL_BLACK);


	// Create progress bar (horizontal)
	for (int i=0; i < 8; i++) {
		pElemRef = gslc_ElemCreateTxt(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){20,20 + i * 15,50,10},
			text[i],0,E_FONT_TXT);
		pElemRef = gslc_ElemXGaugeCreate(&m_gui,E_ELEM_PROGRESS0 + i,E_PG_MAIN,&m_sXGauge[i],
			(gslc_tsRect){50,20 + i * 15,50,10},0,100,0,GSLC_COL_YELLOW,false);
		m_pElemProgress[i] = pElemRef; // Save for quick access
	}


	// Create three sliders (R,G,B) and assign callback function
	// that is invoked upon change. The common callback will update
	// the color box.

	// Static text label
	gslc_ElemCreateTxt_P(&m_gui,106,E_PG_MAIN,20,140,30,20,"Red:",&m_asFont[0],
		GSLC_COL_GRAY_LT3,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
	// Slider
	gslc_ElemXSliderCreate_P(&m_gui,E_SLIDER_R,E_PG_MAIN,60,140,80,20,
		0,255,0,5,false,GSLC_COL_RED,GSLC_COL_BLACK);
	pElemRef = gslc_PageFindElemById(&m_gui,E_PG_MAIN,E_SLIDER_R);
	gslc_ElemXSliderSetStyle(&m_gui,pElemRef,true,GSLC_COL_RED_DK4,10,5,GSLC_COL_GRAY_DK2);
	gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbSlidePos);

	// Static text label
	gslc_ElemCreateTxt_P(&m_gui,107,E_PG_MAIN,20,160,30,20,"Green:",&m_asFont[0],
		GSLC_COL_GRAY_LT3,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
	// Slider
	gslc_ElemXSliderCreate_P(&m_gui,E_SLIDER_G,E_PG_MAIN,60,160,80,20,
		0,255,0,5,false,GSLC_COL_GREEN,GSLC_COL_BLACK);
	pElemRef = gslc_PageFindElemById(&m_gui,E_PG_MAIN,E_SLIDER_G);
	gslc_ElemXSliderSetStyle(&m_gui,pElemRef,true,GSLC_COL_GREEN_DK4,10,5,GSLC_COL_GRAY_DK2);
	gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbSlidePos);

	// Static text label
	gslc_ElemCreateTxt_P(&m_gui,108,E_PG_MAIN,20,180,30,20,"Blue:",&m_asFont[0],
		GSLC_COL_GRAY_LT3,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
	// Slider
	gslc_ElemXSliderCreate_P(&m_gui,E_SLIDER_B,E_PG_MAIN,60,180,80,20,
		0,255,0,5,false,GSLC_COL_BLUE,GSLC_COL_BLACK);
	pElemRef = gslc_PageFindElemById(&m_gui,E_PG_MAIN,E_SLIDER_B);
	gslc_ElemXSliderSetStyle(&m_gui,pElemRef,true,GSLC_COL_BLUE_DK4,10,5,GSLC_COL_GRAY_DK2);
	gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbSlidePos);

	return true;
}



=======
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b
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

<<<<<<< HEAD
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
=======
#ifdef USE_WIFI
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b

#endif

<<<<<<< HEAD
//
// typedef enum item_type {
// 	text = 0,
// 	text_input,
// 	float_input,
// 	sub_menu,
// 	select,
// 	slider
// };
//
// typedef struct s_item {
// 	char title[256];
// 	item_type type;
// 	char *data;
// } t_item;
//
// typedef struct s_menu {
// 	char title[256];
// 	t_item items[20];
// 	uint16_t item_nb;
// } t_menu;
//
// t_menu menu = {
// 	"Main Menu",
// 	{
// 		{
// 			"About",
// 			text,
// 		},
// 		{
// 			"Model menu",
// 			text
// 		},
// 		{
// 			"Wifi setting",
// 			text
// 		},
// 		{
// 			"ADC TEST",
// 			text
// 		}
// 	},
// 	4
// };


// Callback function for when a slider's position has been updated
// - After a slider position has been changed, update the color box
// - Note that all three sliders use the same callback for
//   convenience. From the element's ID we can determine which
//   slider was updated.
bool CbSlidePos(void* pvGui,void* pvElemRef,int16_t nPos)
{
	gslc_tsGui*     pGui      = (gslc_tsGui*)(pvGui);
	gslc_tsElemRef* pElemRef  = (gslc_tsElemRef*)(pvElemRef);
	gslc_tsElem*    pElem     = gslc_GetElemFromRef(pGui,pElemRef);
	//gslc_tsXSlider* pSlider = (gslc_tsXSlider*)(pElem->pXData);

	// Fetch the new RGB component from the slider
	switch (pElem->nId) {
		case E_SLIDER_R:
			m_nPosR = gslc_ElemXSliderGetPos(pGui,pElemRef);
			break;
		case E_SLIDER_G:
			m_nPosG = gslc_ElemXSliderGetPos(pGui,pElemRef);
			break;
		case E_SLIDER_B:
			m_nPosB = gslc_ElemXSliderGetPos(pGui,pElemRef);
			break;
		default:
			break;
	}

	// Calculate the new RGB value
	gslc_tsColor colRGB = (gslc_tsColor){m_nPosR,m_nPosG,m_nPosB};
	if (m_nPosR < 50) m_nPosR = 0;
	if (m_nPosG < 50) m_nPosG = 0;
	if (m_nPosB < 50) m_nPosB = 0;

	Serial.print(m_nPosR);Serial.print(" ,");
	Serial.print(m_nPosG);Serial.print(" ,");
	Serial.println(m_nPosB);

	#ifdef USE_LED
		leds.setPixelColor(0, leds.Color(m_nPosR,m_nPosG,m_nPosB));
		leds.show();
	#endif

	// Update the color box
	// gslc_tsElemRef* pElemColor = gslc_PageFindElemById(pGui,E_PG_MAIN,E_ELEM_COLOR);
	// gslc_ElemSetCol(pGui,pElemColor,GSLC_COL_WHITE,colRGB,GSLC_COL_WHITE);

	return true;
}
=======
uint16_t adc_value[8];
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b

void setup() {
	Serial.begin(115200);
	gslc_InitDebug(&DebugOut);
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

	if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }
	if (!gslc_FontAdd(&m_gui,E_FONT_BTN,GSLC_FONTREF_PTR,NULL,1)) { return; }
	if (!gslc_FontAdd(&m_gui,E_FONT_TXT,GSLC_FONTREF_PTR,NULL,1)) { return; }

	InitOverlays();

	// Start up display on main page
	gslc_SetPageCur(&m_gui,E_PG_MAIN);

	#ifdef USE_LED
		leds.begin();
		leds.show();
		// delay(10);
		// leds.setPixelColor(0, leds.Color(255, 0, 0));
		// leds.show();
		// delay(500);
		// leds.setPixelColor(0, leds.Color(255, 255, 0));
		// leds.show();
		// delay(500);
		// leds.setPixelColor(0, leds.Color(0, 0, 255));
		// leds.show();
		// delay(500);
		// leds.setPixelColor(0, leds.Color(255, 255, 255));
		// leds.show();
		// delay(500);
		// leds.setPixelColor(0, leds.Color(0, 0, 0));
		// leds.show();
		// delay(500);
	#endif

	#ifdef USE_TFT
		tft.begin(40000000); // spi speed 40Mhz
	#endif

	#ifdef USE_TS
		ts.begin();
		ts.setCalc(
			520,
			3320,
			720,
			3435
		);
	#endif

<<<<<<< HEAD
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
=======
	#ifdef USE_ADC
		init_adc();
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b
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
	delay(5);
}


void update_tft() {

	#ifdef USE_TFT
		// tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0, 0);
		// tft.println("Hello");
	#endif

	#if defined(USE_TFT) && defined(PRINT_ADC)
		tft.print("\nADC inputs: ");
		#ifdef USE_EXT_ADC
			tft.println("(EXT)");
		#else
			tft.println("(INT)");
		#endif
		for (int chan=0;chan,chan<8;chan++) {
			tft.printf("  [%d] = %04x [%02d%%]\n", chan, Channel_data[chan], map(Channel_data[chan],0x0000, 0xFFFF, 0, 99));
			// tft.print("  [");
			// tft.print(chan);
			// tft.print("] = ");
			// tft.print(Channel_data[chan]);
			// tft.println("   ");
		}
	#endif

	#if defined(USE_TFT) && defined(USE_TS) && defined(PRINT_TS)
		tft.println("\nTouchScreen:");
		if (ts.touched()) {
			// TS_Point p = ts.getPoint();
			// tft.println("\nRaw:");
			// tft.print("x : "); tft.print(p.x); tft.println("  ");
			// tft.print("y : "); tft.print(p.y); tft.println("  ");
			// tft.print("z : "); tft.print(p.z); tft.println("  ");
			TS_Point p = ts.getPointCalc();
			// tft.println("\nCalibrate:");
			tft.print("  x: "); tft.print(p.x); tft.println("     ");
			tft.print("  y: "); tft.print(p.y); tft.println("     ");
			tft.print("  z: "); tft.print(p.z); tft.println("     ");
			// tft.fillCircle(p.x, p.y, 2, ILI9341_RED);

			// Serial.print("x = "); Serial.println(p.x);
			// Serial.print("y = "); Serial.println(p.y);
		} else {
			tft.println("  x:       ");
			tft.println("  y:       ");
			tft.println("  z:       ");
		}
	#endif


<<<<<<< HEAD
	#if defined(USE_TFT) && defined(USE_WIFI) && defined(PRINT_WIFI)
		tft.printf("\nWifi:\n  SSID: %s\n  IP: ", WIFI_SSID);
		if (connected)
			tft.println(WiFi.localIP());
		else
			tft.println("disconnected");
	#endif

	// tft.drawCircle( 30, 240-30, 2, ILI9341_RED);

=======
	// print_info_sd();
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b
}

void loop() {

	#if  defined(USE_INT_ADC)
		Channel_data[0] = map(analogRead(36), 0x00, 0xFFF, 0x00, 0xFFFF); // 12bit to 16bit
		Channel_data[1] = map(analogRead(39), 0x00, 0xFFF, 0x00, 0xFFFF);
		Channel_data[2] = map(analogRead(34), 0x00, 0xFFF, 0x00, 0xFFFF);
		Channel_data[3] = map(analogRead(35), 0x00, 0xFFF, 0x00, 0xFFFF);
		Channel_data[4] = 127;
		Channel_data[5] = 255;
		Channel_data[6] = 511;
		Channel_data[7] = 1023;
	#endif

	#ifdef USE_EXT_ADC
		for (int chan = 0; chan < 8; chan++) {
			Channel_data[chan] = map(adc.readADC(chan), 0x00, 0x3FF, 0x00, 0xFFFF);
			// Serial.print(Channel_data[chan]); Serial.print("\t");
		}
		// Serial.println();
	#endif

	#ifdef USE_LED
		// leds.setPixelColor(0, leds.Color(
		// 	Channel_data[0]>>8,
		// 	0,
		// 	0
		// ));
		// leds.setPixelColor(1, leds.Color(
		// 	0,
		// 	Channel_data[1]>>8,
		// 	0
		// ));
		// leds.setPixelColor(2, leds.Color(
		// 	0,
		// 	0,
		// 	Channel_data[2]>>8
		// ));
		// leds.setPixelColor(3, leds.Color(
		// 	Channel_data[3]>>8,
		// 	Channel_data[3]>>8,
		// 	0
		// ));
		// leds.show();
	#endif

	#if defined(USE_RADIO) && defined(RADIO_SEND)
		radio_send();
	#endif

<<<<<<< HEAD
	#ifdef USE_WIFI
		if(connected){
			udp.beginPacket(udpAddress, udpPort);
				udp.write((uint8_t *)Channel_data, PAYLOAD_SIZE);
			udp.endPacket();
		}
=======
	#if defined(USE_WIFI) && defined(WIFI_SEND)
		wifi_udp_send();
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b
	#endif

	for (int i=0; i < 8; i++)
		gslc_ElemXGaugeUpdate(&m_gui,m_pElemProgress[i], map(Channel_data[i], 0x0000, 0xFFFF, 0, 99));


	// Periodically call GUIslice update function
	gslc_Update(&m_gui);

	// BAYANG_callback();
	delay(5);
}
<<<<<<< HEAD

#ifdef USE_WIFI
	//wifi event handler
	void WiFiEvent(WiFiEvent_t event){
		switch(event) {
			case SYSTEM_EVENT_STA_GOT_IP:
				//When connected set
				Serial.print("WiFi connected! IP address: ");
				Serial.println(WiFi.localIP());
				//initializes the UDP state
				//This initializes the transfer buffer
				udp.begin(WiFi.localIP(),udpPort);
				connected = true;
				break;
			case SYSTEM_EVENT_STA_DISCONNECTED:
				Serial.println("WiFi lost connection");
				connected = false;
			break;
		}
	}
	#endif
=======
>>>>>>> bda47566da81f26f6cc45ef1aa5f52ba4b31490b
