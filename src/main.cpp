#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MempoolClient.h>

#include "config.h"

const int BLOCK_PIN = D6;
const int DA_PIN = D5;

MempoolClient mempoolClient;

void initGPIO()
{
  pinMode(BLOCK_PIN, OUTPUT);
  pinMode(DA_PIN, OUTPUT);
  digitalWrite(BLOCK_PIN, LOW);
  digitalWrite(DA_PIN, LOW);
}

void initSerial()
{
  Serial.begin(115200);
}

void setLedMulti(int *pins, int numPins, int value)
{
  for (int i = 0; i < numPins; i++)
  {
    digitalWrite(pins[i], value);
  }
}

void flashLed(int *pins, int numPins)
{
  for (int i = 0; i < 9; i++)
  {
    setLedMulti(pins, numPins, HIGH);
    delay(100);
    setLedMulti(pins, numPins, LOW);
    delay(100);
  }
  setLedMulti(pins, numPins, HIGH);
  delay(500);
  setLedMulti(pins, numPins, LOW);
}

void onBlockEvent(int height)
{
  Serial.print("Received block at height ");
  Serial.println(height);

  int pins_to_flash[] = {BLOCK_PIN, DA_PIN};
  if (height % 2016 == 0)
  {
    flashLed(pins_to_flash, 2);
  }
  else
  {
    flashLed(pins_to_flash, 1);
  }
}

void connectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(BLOCK_PIN, HIGH);
    delay(500);
    digitalWrite(BLOCK_PIN, LOW);
    delay(500);
  }
}

void connectMempool()
{
  mempoolClient.begin(MEMPOOL_HOST, MEMPOOL_PORT, MEMPOOL_PATH, MEMPOOL_USE_SSL);
  mempoolClient.onBlock(onBlockEvent);

  Serial.println("Connecting to Mempool...");

  while (!mempoolClient.isConnected())
  {
    mempoolClient.loop();
  }
}

void setup()
{
  initGPIO();
  initSerial();

  connectWifi();
  digitalWrite(BLOCK_PIN, HIGH);

  connectMempool();
  digitalWrite(DA_PIN, HIGH);

  delay(1000);

  digitalWrite(BLOCK_PIN, LOW);
  digitalWrite(DA_PIN, LOW);
}

void loop()
{
  mempoolClient.loop();
}
