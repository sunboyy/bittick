#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MempoolClient.h>

#include "config.h"

const int BLOCK_PIN = D6;

MempoolClient mempoolClient;

void initGPIO()
{
  pinMode(BLOCK_PIN, OUTPUT);
  digitalWrite(BLOCK_PIN, LOW);
}

void initSerial()
{
  Serial.begin(115200);
}

void flashLed(int pin)
{
  for (int i = 0; i < 9; i++)
  {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
}

void onBlockEvent(int height)
{
  Serial.print("Received block at height ");
  Serial.println(height);

  flashLed(BLOCK_PIN);
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

  digitalWrite(BLOCK_PIN, LOW);
}

void loop()
{
  mempoolClient.loop();
}
