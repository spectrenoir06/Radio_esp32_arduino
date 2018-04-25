
#ifdef USE_RADIO

RF24 radio(NRF_CE_pin, NRF_CSN_pin);
uint8_t radio_on = 0;

void init_radio() {
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
}
void print_info_radio() {
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
}

void radio_send() {
	if (radio.write(adc_value, PAYLOAD_SIZE))
		Serial.print("Radio send success\n");
	else
		Serial.print("Radio send failed\n");
}


#endif
