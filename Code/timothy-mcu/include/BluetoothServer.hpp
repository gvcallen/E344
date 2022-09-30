#pragma once

#include <Arduino.h>
#include <BLEServer.h>

#include <string_view>

class BluetoothServer : public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  public:
    int begin(std::string_view name);
    bool deviceConnected()
    {
        return deviceIsConnected;
    };
    int send(uint8_t *buffer, size_t size);
    size_t receive(uint8_t *buffer, size_t size);

    volatile bool dataReceived;

  public:
    bool deviceIsConnected;
    BLECharacteristic *rxCharacteristic;
    BLECharacteristic *txCharacteristic;
    BLEServer *server;

  private:
    void onConnect(BLEServer *server) override;
    void onDisconnect(BLEServer *server) override;
    void onWrite(BLECharacteristic *characteristic) override;
};