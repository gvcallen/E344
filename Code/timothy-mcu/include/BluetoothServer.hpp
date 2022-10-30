#pragma once

#include <Arduino.h>
#include <BLEServer.h>

#include <string_view>

namespace tim
{

class BluetoothServer : public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  public:
    int begin(std::string_view name);
    int end();
    bool running()
    {
        return this->isRunning;
    }
    bool connected()
    {
        return deviceIsConnected;
    };
    int send(uint8_t *buffer, size_t size);
    size_t receive(uint8_t *buffer, size_t size);
    bool tryReceive(uint8_t *buffer, size_t bufferSize);

    volatile bool dataReceived;

  private:
    bool isRunning;
    bool deviceIsConnected;
    BLECharacteristic *rxCharacteristic;
    BLECharacteristic *txCharacteristic;
    BLEServer *server;
    BLEService *service;

  private:
    void onConnect(BLEServer *server) override;
    void onDisconnect(BLEServer *server) override;
    void onWrite(BLECharacteristic *characteristic) override;
};

} // namespace tim