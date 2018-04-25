
#ifdef USE_SD

uint8_t cardType;
uint8_t cardState;
uint64_t cardSize;

void init_sd() {
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
}

void print_info_sd() {
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
}

#endif
