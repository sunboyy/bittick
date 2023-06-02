#include <ArduinoJson.h>

#include "MempoolClient.h"

void MempoolClient::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("Disconnected from WebSocket server");
        break;
    case WStype_CONNECTED:
        Serial.println("Connected to WebSocket server");
        sendInitMessage();
        break;
    case WStype_TEXT:
        handleMessage((const char *)payload);
        break;
    default:
        break;
    }
}

void MempoolClient::sendInitMessage()
{
    String initialData = "{\"action\":\"want\",\"data\":[\"blocks\"]}";
    mWebSocket.sendTXT(initialData);
}

void MempoolClient::handleMessage(const char *message)
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
        if (mBlockEvent != NULL)
        {
            mBlockEvent(height);
        }
    }
}

void MempoolClient::begin(const char *host, int port, const char *path, bool useSSL)
{
    if (useSSL)
    {
        mWebSocket.beginSSL(host, port, path);
    }
    else
    {
        mWebSocket.begin(host, port, path);
    }

    mWebSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        this->webSocketEvent(type, payload, length);
    });
    mWebSocket.setReconnectInterval(5000);
    mWebSocket.enableHeartbeat(30000, 2000, 2);
}

bool MempoolClient::isConnected()
{
    return mWebSocket.isConnected();
}

void MempoolClient::onBlock(BlockEvent blockEvent)
{
    mBlockEvent = blockEvent;
}

void MempoolClient::loop()
{
    mWebSocket.loop();
}
