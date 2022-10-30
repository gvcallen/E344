#include "WiFiServer.hpp"
#include <WiFi.h>

#include "Error.hpp"

namespace tim
{

void WiFiServer::begin(const char *ssid, const char *password, uint16_t port)
{
    // ::WiFi.mode(WIFI_OFF);
    ::WiFi.softAP(ssid, password);
    ::WiFi.setSleep(WIFI_PS_MAX_MODEM);

    this->address = ::WiFi.softAPIP();
    this->server.begin(port);
    this->isRunning = true;

    Serial.print("IP Address: ");
    Serial.println(this->address);
}

void WiFiServer::end()
{
    this->server.end();
    delay(300);
    // ::WiFi.softAP("", "");
    ::WiFi.softAPdisconnect(true);
    // ::WiFi.disconnect(true, true);
    ::WiFi.mode(WIFI_OFF);
    this->isRunning = false;
}

void WiFiServer::update()
{
    if (!currentClient || !currentClient.connected())
    {
        currentClient = this->available();
    }
}

static std::string getPutValue(std::string &string, size_t keyPos)
{
    size_t ampersandPos = string.find_first_of('&', keyPos);
    return string.substr(keyPos + 4, ampersandPos + 1 - (keyPos + 4) - 1);
}

std::vector<Message> WiFiServer::getRequests()
{
    if (!(currentClient && currentClient.connected()))
    {
        return std::vector<Message>{};
    }

    std::string header = "";
    std::vector<Message> messages;

    while (currentClient.available())
    {
        char c = currentClient.read();
        // Serial.print(c);
        header += c;
    }

    if (header.rfind("GET", 0) == 0)
    {
        Message message;
        message.type = Message::Type::GetAll;
        message.id = MESSAGE_GET_ALL_ID;
        messages.push_back(message);
    }
    else if (header.rfind("PUT", 0) == 0)
    {
        if (size_t leftCommandPos = header.find("lws"))
        {
            Message message;
            auto value = atoi(getPutValue(header, leftCommandPos).c_str());
            message.type = Message::Type::SetLWSpeed;
            message.data.speed = (float)value / 15.0f;
            messages.push_back(message);
        }
        if (size_t rightCommandPos = header.find("rws"))
        {
            Message message;
            auto value = atoi(getPutValue(header, rightCommandPos).c_str());
            message.type = Message::Type::SetRWSpeed;
            message.data.speed = (float)value / 15.0f;
            messages.push_back(message);
        }
    }

    return messages;
}

int WiFiServer::sendResponse(Message message)
{
    if (message.type == Message::Type::GetAll)
    {
        currentClient.println("HTTP/1.1 200 OK");
        currentClient.println("Content-type:text/plain");
        currentClient.println("Connection: close");
        currentClient.println();
        auto &getAll = message.data.getAll;
        currentClient.print("Left wheel speed:\t\t\t");
        currentClient.println((int)(getAll.lws * 15.0f));

        currentClient.print("Left wheel current:\t\t\t");
        currentClient.println(getAll.lwc);

        currentClient.print("Left sensor range:\t\t\t");
        currentClient.println(getAll.lsr);

        currentClient.print("Right wheel speed:\t\t\t");
        currentClient.println((int)(getAll.rws * 15.0f));

        currentClient.print("Right wheel current:\t\t\t");
        currentClient.println(getAll.rwc);

        currentClient.print("Right sensor range:\t\t\t");
        currentClient.println(getAll.rsr);

        currentClient.print("Battery voltage:\t\t\t");
        currentClient.println(getAll.bat);
    }
    else if (message.type == Message::Type::SetLWSpeed || message.type == Message::Type::SetRWSpeed)
    {
        currentClient.println("HTTP/1.1 204 OK");
        currentClient.println("Connection: close");
        currentClient.println();
    }
    else
    {
        currentClient.println("HTTP/1.1 204 OK");
        currentClient.println("Connection: close");
        currentClient.println();
    }
    currentClient.println();
    currentClient.stop();

    return ERROR_NONE;
}

} // namespace tim