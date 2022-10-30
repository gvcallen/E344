#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#include "Message.hpp"

namespace tim
{

class WiFiServer
{
  public:
    void begin(const char *ssid, const char *password, uint16_t port);
    void end();
    bool running()
    {
        return this->isRunning;
    }
    void update();
    IPAddress getAddress()
    {
        return this->address;
    }
    WiFiClient available()
    {
        return server.available();
    };
    bool connected()
    {
        return currentClient && currentClient.connected();
    }

    std::vector<Message> getRequests();
    int sendResponse(Message message);

  private:
    bool isRunning;
    IPAddress address;
    ::WiFiServer server;
    WiFiClient currentClient;
};

} // namespace tim