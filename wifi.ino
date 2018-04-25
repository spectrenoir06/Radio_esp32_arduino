#ifdef USE_WIFI

const char * networkName = WIFI_SSID;
const char * networkPswd = WIFI_PASSWORD;
const char * udpAddress = "192.168.11.169";
const int udpPort = 1234;

boolean connected = false;
WiFiUDP udp;

void init_wifi() {
	Serial.println("Connecting to WiFi network: " + String(networkName));
	WiFi.disconnect(true);
	WiFi.onEvent(wifi_event);
	WiFi.begin(networkName, networkPswd);
}

void wifi_udp_send() {
	if(connected){
		//Send a packet
		udp.beginPacket(udpAddress, udpPort);
			udp.write((uint8_t *)adc_value, PAYLOAD_SIZE);
		udp.endPacket();
	}
}


//wifi event handler
void wifi_event(WiFiEvent_t event){
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
