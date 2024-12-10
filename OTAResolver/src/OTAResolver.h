#ifndef OTA_Resolver_h
#define OTA_Resolver_h

#include <Arduino.h>
#include <WiFi.h>

class OTAResolver
{
public:
    OTAResolver(String  versionCheckURL, String  bearerToken);
    OTAResolver(const String& wifiSSID, const String& wifiPassword, String  versionCheckURL, String  bearerToken);
    OTAResolver(const String& wifiSSID, const String& wifiPassword, String  versionCheckURL, String  bearerToken, unsigned long baud);

private:
    String _versionCheckURL;
    String _bearerToken;
    bool _isWiFiInitialized = false;
    bool _isSerialInitialized = false;

    void _beginWiFi(const String& wifiSSID, const String& wifiPassword);
    void _beginSerial(unsigned long baud);
};

#endif