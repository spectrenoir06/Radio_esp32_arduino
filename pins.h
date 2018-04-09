
// #define OLD

#define NEOPIXEL_pin  5

#define TFT_DC_pin    2
#define TFT_CSN_pin   15
#define TOUCH_CSN_pin 4
#define SD_CSN_pin    16
#define ADC_CSN_pin   21

#define NRF_CE_pin  35// not use
#define CYRF_RST_pin  17
#define CC25_CSN_pin  27
#define A7105_CSN_pin 14

#define PE1_pin       13
#define PE2_pin       12

#ifdef OLD
	#define NRF_CSN_pin  26
	#define CYRF_CSN_pin 25
#else
	#define NRF_CSN_pin  33
	#define CYRF_CSN_pin 32
#endif

#define	PE1_on  		digitalWrite(PE1_pin,HIGH)
#define	PE1_off		 	digitalWrite(PE1_pin,LOW)
//
#define	PE2_on  		digitalWrite(PE2_pin,HIGH)
#define	PE2_off 		digitalWrite(PE2_pin,LOW)

#define	A7105_CSN_on	digitalWrite(A7105_CSN_pin,HIGH)
#define	A7105_CSN_off	digitalWrite(A7105_CSN_pin,LOW)

#define NRF_CE_on
#define	NRF_CE_off

#define	CC25_CSN_on		digitalWrite(CC25_CSN_pin,HIGH)
#define	CC25_CSN_off	digitalWrite(CC25_CSN_pin,LOW)

#define	NRF_CSN_on		digitalWrite(NRF_CSN_pin,HIGH)
#define	NRF_CSN_off		digitalWrite(NRF_CSN_pin,LOW)

#define	CYRF_CSN_on		digitalWrite(CYRF_CSN_pin,HIGH)
#define	CYRF_CSN_off	digitalWrite(CYRF_CSN_pin,LOW)

#define	CYRF_RST_HI		digitalWrite(CYRF_RST_pin,HIGH)	//reset cyrf
#define	CYRF_RST_LO		digitalWrite(CYRF_RST_pin,LOW)	//
