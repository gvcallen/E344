#include <Arduino.h>
#include <BLE2902.h>
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
    BLEDevice::init(name.data());

    // Create server
    server = BLEDevice::createServer();
    server->setCallbacks(this);

    // Create UART service and characteristics
    BLEService *service = server->createService(BLUETOOTH_SERVICE_UUID);
    txCharacteristic =
        service->createCharacteristic(BLUETOOTH_CHARACTERISTIC_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    txCharacteristic->addDescriptor(new BLE2902());
    rxCharacteristic =
        service->createCharacteristic(BLUETOOTH_CHARACTERISTIC_RX_UUID, BLECharacteristic::PROPERTY_WRITE);

    // Set callbacks for "connected" and RX
    rxCharacteristic->setCallbacks(this);

    // Start service and advertising
    service->start();
    BLEAdvertising *advertising = server->getAdvertising();
    // advertising->addServiceUUID(BLUETOOTH_SERVICE_UUID);
    advertising->start();

    dataReceived = false;

    return ERROR_NONE;
}

void BluetoothServer::onConnect(BLEServer *server)
{
    deviceIsConnected = true;
}

void BluetoothServer::onDisconnect(BLEServer *server)
{
    deviceIsConnected = false;

    delay(500); // give the bluetooth stack the chance to get things ready
    server->startAdvertising();
}

void BluetoothServer::onWrite(BLECharacteristic *characteristic)
{
    dataReceived = true;
}

int BluetoothServer::send(uint8_t *buffer, size_t size)
{
    if (this->deviceIsConnected)
    {
        this->txCharacteristic->setValue(buffer, size);
        this->txCharacteristic->notify();
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }

    return ERROR_NONE;
}

size_t BluetoothServer::receive(uint8_t *buffer, size_t maxSize)
{
    const uint8_t *rxValue = rxCharacteristic->getData();
    auto length = rxCharacteristic->getLength();

    for (int i = 0; i < length; i++)
    {
        if (i == maxSize)
            break;

        buffer[i] = rxValue[i];
    }

    return length;
}