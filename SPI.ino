void SPI_Write(uint8_t command) {
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	SPI.transfer(command);
	SPI.endTransaction();
}

uint8_t SPI_Read(void) {
	uint8_t result=0;
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	result = _SPI.transfer(0xff);
	SPI.endTransaction();
	return result;
}

// #ifdef A7105_INSTALLED
// uint8_t SPI_SDI_Read(void)
// {
// 	uint8_t result=0;
// 	SDI_input;
// 	for(uint8_t i=0;i<8;i++)
// 	{
// 		result=result<<1;
// 		if(SDI_1)  ///if SDIO =1
// 			result |= 0x01;
// 		SCLK_on;
// 		NOP();
// 		SCLK_off;
// 	}
// 	SDI_output;
// 	return result;
// }
// #endif
