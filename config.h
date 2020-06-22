#define PROTOCOL_BAYANG
// #define PROTOCOL_FRSKYD
// #define PROTOCOL_DSM2
// #define PROTOCOL_BLE
// #define PROTOCOL_WIFI


#define TELEMETRY

#if defined(PROTOCOL_BAYANG)
	#define NRF24L01_INSTALLED
	#define	BAYANG_NRF24L01_INO
	#define PE_SET PE1_on; PE2_off
	#define INIT_RF sub_protocol = H8S3D ;initBAYANG()
	#define CALLBACK_RF delay(BAYANG_callback()/1000.0)

#elif defined(PROTOCOL_FRSKYD)
	#define CC2500_INSTALLED
	#define	FRSKYD_CC2500_INO
	#define PE_SET PE1_off; PE2_on
	#define INIT_RF protocol = PROTO_DSM; sub_protocol = DSM2_22; delay(initDsm()/1000.0)
	#define CALLBACK_RF delay(ReadFrSky_2way()/1000.0)

#elif defined(PROTOCOL_DSM2)
	#define	DSM_CYRF6936_INO
	#define DSM_TELEMETRY
	#define PE_SET PE1_on; PE2_on
	#define INIT_RF delay(initFrSky_2way()/1000.0)
	#define CALLBACK_RF delay(ReadDsm() / 1000.0)

#elif defined(PROTOCOL_BLE)
	#define USE_BLE

#elif defined(PROTOCOL_WIFI)
	#define USE_WIFI

#endif



// #define USE_TFT
#define USE_LED
// #define USE_SD
// #define USE_TS
#define USE_ADC



// #define CYRF6936_INSTALLED


// #define USE_RADIO

#define USE_INT_ADC
// #define USE_EXT_ADC

// -------- TFT display ------------

#define PRINT_ADC
// #define PRINT_TS
// #define PRINT_RADIO
// #define PRINT_SD
// #define PRINT_WIFI

// --------------------------------

// #define ADC_SERIAL_OUTPUT
// #define WIFI_SEND


#define FONT_SIZE 1

#define COLOR_BACK 0xD69A
#define COLOR_TITLE_BAR_TEXT 0xFFFF
#define COLOR_TITLE_BAR_BACK 0xFD89

#define COLOR_ITEM_BAR_BACK 0xFFFF
#define COLOR_ITEM_BAR_BACK_SELECT 0xDEDB

#define COLOR_ITEM_BAR_TEXT 0x10A2
#define COLOR_ITEM_BAR_BORDER 0xE77D
