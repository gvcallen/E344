#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "BluetoothServer.hpp"
#include "Error.hpp"

#define BLUETOOTH_SERVICE_UUID "980e293c-3e62-11ed-b878-0242ac120002"
#define BLUETOOTH_CHARACTERISTIC_TX_UUID "980e2c70-3e62-11ed-b878-0242ac120002"
#define BLUETOOTH_CHARACTERISTIC_RX_UUID "980e2d7e-3e62-11ed-b878-0242ac120002"

int BluetoothServer::begin(std::string_view name)
{
    // Initialize device
    Serial.println("Setting up bluetooth");

    BLEDevice::init(name.data());

    // Create server
    BLEServer *server = BLEDevice::createServer();

    // Create UART service and characteristics
    BLEService *service = server->createService(BLUETOOTH_SERVICE_UUID);
    BLECharacteristic *txCharacteristic =
        service->createCharacteristic(BLUETOOTH_CHARACTERISTIC_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    BLECharacteristic *rxCharacteristic =
        service->createCharacteristic(BLUETOOTH_CHARACTERISTIC_RX_UUID, BLECharacteristic::PROPERTY_WRITE);

    // Set callbacks for "connected" and RX
    server->setCallbacks(this);
    rxCharacteristic->setCallbacks(this);

    // Start service and advertising
    service->start();
    // server->getAdvertising()->start();

    BLEAdvertising *advertising = server->getAdvertising();
    advertising->addServiceUUID(BLUETOOTH_SERVICE_UUID);
    advertising->start();

    Serial.println("Bluetooth setup finished");

    return ERROR_NONE;
}

void BluetoothServer::onConnect(BLEServer *server)
{
    deviceIsConnected = true;
}

void BluetoothServer::onDisconnect(BLEServer *server)
{
    deviceIsConnected = true;
}

void BluetoothServer::onWrite(BLECharacteristic *characteristic)
{
    std::string rxValue = characteristic->getValue();

    if (!rxValue.empty())
    {
        Serial.print("Received: ");
        Serial.println(rxValue.c_str());
    }
}