#include <OTA_Resolver.h>

OTA_Resolver::OTA_Resolver(String versionCheckURL, String bearerToken)
    : _versionCheckURL(std::move(versionCheckURL)), _bearerToken(std::move(bearerToken))
{
}

OTA_Resolver::OTA_Resolver(const String& wifiSSID, const String& wifiPassword, String versionCheckURL,
                           String bearerToken)
    : _versionCheckURL(std::move(versionCheckURL)), _bearerToken(std::move(bearerToken))
{
    this->_beginWiFi(wifiSSID, wifiPassword);
}

OTA_Resolver::OTA_Resolver(const String& wifiSSID, const String& wifiPassword, String versionCheckURL,
                           String bearerToken, const unsigned long baud)
    : _versionCheckURL(std::move(versionCheckURL)), _bearerToken(std::move(bearerToken))
{
    this->_beginWiFi(wifiSSID, wifiPassword);
    this->_beginSerial(baud);
}

void OTA_Resolver::_beginWiFi(const String& wifiSSID, const String& wifiPassword)
{
    Serial.println("connecting to wi-fi: " + wifiSSID);
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("connection to wi-fi " + wifiSSID + " failed! rebooting...");
        delay(3000);
        ESP.restart();
    }
    Serial.println("connected to wi-fi: " + wifiSSID);
    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);
    Serial.println("wi-fi initialized");
    this->_isWiFiInitialized = true;
}

void OTA_Resolver::_beginSerial(const unsigned long baud)
{
    Serial.begin(baud);
    int count = 10;
    while (!Serial && count--)
    {
        delay(1000);
    }
    if (count == 0)
    {
        Serial.println("serial not available! rebooting...");
        delay(3000);
        ESP.restart();
    }
    this->_isSerialInitialized = true;
}
