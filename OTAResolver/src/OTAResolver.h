#ifndef OTAResolver_h
#define OTAResolver_h

#include <Arduino.h>
#include <WiFi.h>

#define VERSION_EEPROM_SIZE 64
#define VERSION_ADDR 0
#define UPDATE_CHUNK_SIZE 1024

struct VersionInfo
{
    String version;
    String url;
};

class OTAResolver
{
public:
    OTAResolver();
    void begin(String versionCheckURL, String bearerToken);
    void begin(String wifiSSID, String wifiPassword, String versionCheckURL, String bearerToken);
    void begin(String wifiSSID, String wifiPassword, String versionCheckURL, String bearerToken, unsigned long baud);
    auto setCheckIntervalSeconds(int checkIntervalSeconds) -> OTAResolver&;
    void tick();

private:
    String _wiFiSSID;
    String _wiFiPassword;
    unsigned long _baud = 115200;
    String _versionCheckURL;
    String _bearerToken;
    bool _isWiFiInitialized = false;
    bool _isSerialInitialized = false;
    int _checkInterval = 60 * 1000;
    unsigned long _lastTickTime = 0;
    String _version;

    void _beginSerial();
    void _beginWiFi();
    VersionInfo& _getVersionInfo() const;
    static void _logln(const String& message);
    bool _updateRequired(const String& lastVersion) const;
    static bool _isValidVersionFormat(const String& version);
    static void _initializeEEPROM();
    void _saveVersionInMemory(String newVersion);
    void _loadInMemoryVersion();
    void _update(const VersionInfo& versionInfo);
    void _tick();
};

#endif
