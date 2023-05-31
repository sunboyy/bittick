#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#include "config.h"

const int BLOCK_PIN = D6;
const int DA_PIN = D5;

WebSocketsClient webSocket;

void sendInitialData()
{
  String initialData = "{\"action\":\"want\",\"data\":[\"blocks\"]}";
  webSocket.sendTXT(initialData);
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

void handleMessage(const char *message)
{
  DynamicJsonDocument doc(8192);
  deserializeJson(doc, message);
  JsonObject data = doc.as<JsonObject>();
  Serial.print("Received message: ");
  serializeJson(data, Serial);
  Serial.println();

  if (data.containsKey("block"))
  {
    int height = data["block"]["height"];
    Serial.print("Received block ");
    Serial.print(data["block"]["id"].as<String>());
    Serial.print(" at height ");
    Serial.println(height);

    int pins_to_flash[] = {BLOCK_PIN, DA_PIN};
    if (height % 2016 == 0)
    {
      Serial.println("Difficulty adjustment block");
      flashLed(pins_to_flash, 2);
    }
    else
    {
      flashLed(pins_to_flash, 1);
    }
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("Disconnected from WebSocket server");
    break;
  case WStype_CONNECTED:
    Serial.println("Connected to WebSocket server");
    digitalWrite(DA_PIN, HIGH);
    delay(1000);
    digitalWrite(DA_PIN, LOW);
    break;
  case WStype_TEXT:
    handleMessage((const char *)payload);
    break;
  }
}

void connectWifiAndWait()
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

void connectWebsocket()
{
  connectWifiAndWait();

  if (MEMPOOL_USE_SSL)
  {
    webSocket.beginSSL(MEMPOOL_HOST, MEMPOOL_PORT, MEMPOOL_PATH);
  }
  else
  {
    webSocket.begin(MEMPOOL_HOST, MEMPOOL_PORT, MEMPOOL_PATH);
  }

  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(30000, 2000, 2);

  Serial.println("Connecting to WebSocket server...");

  while (!webSocket.isConnected())
  {
    webSocket.loop();
    delay(10);
  }

  sendInitialData();
}

void setup()
{
  pinMode(BLOCK_PIN, OUTPUT);
  pinMode(DA_PIN, OUTPUT);
  digitalWrite(BLOCK_PIN, LOW);
  digitalWrite(DA_PIN, LOW);
  Serial.begin(115200);
  connectWebsocket();
}

void loop()
{
  webSocket.loop();
}
