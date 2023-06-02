#ifndef MEMPOOL_CLIENT_H_
#define MEMPOOL_CLIENT_H_

#include <WebSocketsClient.h>

typedef std::function<void(int blockHeight)> BlockEvent;

class MempoolClient
{
private:
    WebSocketsClient mWebSocket;
    BlockEvent mBlockEvent;

    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    void sendInitMessage();
    void handleMessage(const char *payload);

public:
    void begin(const char *host, int port, const char *path, bool useSSL);
    void onBlock(BlockEvent blockEvent);
    bool isConnected();
    void loop();
};

#endif
