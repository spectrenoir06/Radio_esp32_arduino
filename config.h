#define OLD

// #define USE_RADIO
#define USE_TFT
// #define USE_LED
// #define USE_SD
#define USE_TS
#define USE_ADC

#ifdef USE_ADC
	#ifdef OLD
		#define USE_INT_ADC
	#else
		#define USE_EXT_ADC
	#endif
#endif

#define PRINT_ADC
#define PRINT_TS
#define PRINT_RADIO
#define PRINT_SD

#define FONT_SIZE 1

#define NRF24L01_INSTALLED

// #define	BAYANG_NRF24L01_INO
