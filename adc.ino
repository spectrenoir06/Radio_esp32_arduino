#ifdef USE_EXT_ADC

Adafruit_MCP3008 adc;

void init_adc() {
	#ifdef USE_EXT_ADC
		adc.begin(ADC_CSN_pin);
	#endif
}




#endif
