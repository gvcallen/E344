#pragma once

#include <Arduino.h>
#include <BLEServer.h>

#include <string_view>

class BluetoothServer : public BLEServerCallbacks, public BLECharacteristicCallbacks
{
  public:
    int begin(std::string_view name);
    bool deviceConnected();

  private:
    bool deviceIsConnected;

  private:
    void onConnect(BLEServer *server) override;
    void onDisconnect(BLEServer *server) override;
    void onWrite(BLECharacteristic *characteristic) override;
};