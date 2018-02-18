
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_NeoPixel.h>
#include "RF24.h"


#define NEOPIXEL_PIN 5
#define TFT_DC 2
#define TFT_CS 15

#define NRF24_CE 26
#define NRF24_CS 26
#define PAYLOAD_SIZE 15

#define RADIO_PE1 13
#define RADIO_PE2 12

uint8_t addresses[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};
uint8_t data[PAYLOAD_SIZE];

RF24 radio(NRF24_CE, NRF24_CS);
Adafruit_ILI9341 tft   = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_NeoPixel leds = Adafruit_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
 	Serial.begin(115200);

	leds.begin();
  leds.show();
  leds.setPixelColor(0, leds.Color(50,25,0));
  leds.show();

  pinMode(RADIO_PE1, OUTPUT);
  pinMode(RADIO_PE2, OUTPUT);
  digitalWrite(RADIO_PE1, HIGH); // nrf24 (pE1 high, PE2 low)
  digitalWrite(RADIO_PE2, LOW);

  //SPI.setFrequency(2000000);
  //SPI.setBitOrder(MSBFIRST);
  //SPI.setDataMode(0);

  if (radio.begin())
    Serial.println("OK!");
  else
    Serial.println("Radio error init");

  Serial.println("\n");

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

  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
}

void loop() {
  tft.fillRect(0, 0, 80, 100, 0);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE,0);
  tft.setTextSize(3);
  uint16_t *ptr = (uint16_t *)data;
  ptr[0] = analogRead(36);
	ptr[1] = analogRead(39);
	ptr[2] = analogRead(34);
	ptr[3] = analogRead(35);

	tft.println(ptr[0]);
	tft.println(ptr[1]);
	tft.println(ptr[2]);
	tft.println(ptr[3]);

  leds.setPixelColor(0, leds.Color(
		map(ptr[0], 0, 4095, 0, 255),
		map(ptr[1], 0, 4095, 0, 255),
		map(ptr[2], 0, 4095, 0, 255)));
  leds.show();

  if (radio.write(data, PAYLOAD_SIZE))
    Serial.print("s\n");
  else
    Serial.print("f\n");

  delay(20);
}
