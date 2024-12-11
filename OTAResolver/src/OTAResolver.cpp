#include <OTAResolver.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>

OTAResolver::OTAResolver() = default;

void OTAResolver::begin(String versionCheckURL, String bearerToken)
{
    _versionCheckURL = std::move(versionCheckURL);
    _bearerToken = std::move(bearerToken);
    _loadInMemoryVersion();
}

void OTAResolver::begin(String wifiSSID, String wifiPassword, String versionCheckURL,
                        String bearerToken)
{
    _wiFiSSID = std::move(wifiSSID);
    _wiFiPassword = std::move(wifiPassword);
    _versionCheckURL = std::move(versionCheckURL);
    _bearerToken = std::move(bearerToken);
    _loadInMemoryVersion();

    _beginWiFi();
}

void OTAResolver::begin(String wifiSSID, String wifiPassword, String versionCheckURL,
                        String bearerToken, const unsigned long baud)
{
    _wiFiSSID = std::move(wifiSSID);
    _wiFiPassword = std::move(wifiPassword);
    _versionCheckURL = std::move(versionCheckURL);
    _bearerToken = std::move(bearerToken);
    _baud = baud;
    _loadInMemoryVersion();

    _beginSerial();
    _beginWiFi();
}

void OTAResolver::_beginSerial()
{
    Serial.begin(_baud);
    int count = 10;
    while (!Serial && count--)
    {
        delay(1000);
    }
    if (count == 0)
    {
        _logln("serial not available! rebooting...");
        delay(3000);
        ESP.restart();
    }
    _logln("serial initialized");
    _isSerialInitialized = true;
}

void OTAResolver::_beginWiFi()
{
    _logln("connecting to wi-fi: " + _wiFiSSID);
    WiFi.begin(_wiFiSSID, _wiFiPassword);
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        _logln("connection to wi-fi " + _wiFiSSID + " failed! rebooting...");
        delay(3000);
        ESP.restart();
    }
    _logln("connected to wi-fi: " + _wiFiSSID);
    WiFi.setAutoReconnect(true);
    WiFi.setAutoConnect(true);
    _logln("wi-fi initialized");
    _isWiFiInitialized = true;
}

OTAResolver& OTAResolver::setCheckIntervalSeconds(const int checkIntervalSeconds)
{
    _checkInterval = checkIntervalSeconds * 1000;
    return *this;
}

void OTAResolver::tick()
{

    const unsigned long currentTime = millis();
    if (currentTime - _lastTickTime >= _checkInterval)
    {
        _tick();
        _lastTickTime = currentTime;
    }
}

void OTAResolver::_update(const VersionInfo& versionInfo)
{
    HTTPClient http;
    http.addHeader("Authorization", "Bearer " + _bearerToken);
    http.begin(versionInfo.url);
    const int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK)
    {
        const int contentLength = http.getSize();
        WiFiClient* stream = http.getStreamPtr();
        _logln("update content length: " + String(contentLength));

        if (!Update.begin(contentLength))
        {
            _logln("update initialization error!");
            http.end();
            return;
        }

        size_t downloaded = 0;

        _logln("downloading started from: " + versionInfo.url);

        while (http.connected() && (downloaded < contentLength || contentLength == -1))
        {
            size_t sizeAvailable = stream->available();
            if (sizeAvailable > 0)
            {
                uint8_t buffer[UPDATE_CHUNK_SIZE];
                const size_t chunkSize = stream->readBytes(buffer, std::min<size_t>(sizeAvailable, UPDATE_CHUNK_SIZE));
                downloaded += chunkSize;

                if (Update.write(buffer, chunkSize) != chunkSize)
                {
                    _logln("block data write error!");
                    Update.abort();
                    http.end();
                    return;
                }

                // _logln("downloaded " + String(downloaded) + " from " + String(contentLength) + " bytes");
            }
            delay(1);
        }

        _logln("downloaded " + String(downloaded) + " from " + String(contentLength) + " bytes from " + versionInfo.url);

        http.end();

        if (Update.end(true))
        {
            _logln("file successfully downloaded!");
            if (Update.isFinished())
            {
                _logln("update successfully completed!");
                _saveVersionInMemory(versionInfo.version);
                ESP.restart();
            }

            _logln("update error!");
            return;
        }

        _logln("update error!\n" + String(Update.errorString()));
        return;
    }

    _logln("http request error. code: " + String(httpResponseCode) + ", url: " + versionInfo.url);
    http.end();
}

void OTAResolver::_tick()
{
    const VersionInfo versionInfo = _getVersionInfo();

    if (versionInfo.version.isEmpty() || versionInfo.url.isEmpty())
    {
        _logln("version info not found!");
        return;
    }

    _logln("latest version: " + versionInfo.version);
    _logln("current version: " + _version);

    if (_version.isEmpty() || _updateRequired(versionInfo.version))
    {
        _logln("update required!");
        _update(versionInfo);
    }

    _logln("tick executed");
}

VersionInfo& OTAResolver::_getVersionInfo() const
{
    HTTPClient http;
    http.addHeader("Authorization", "Bearer " + _bearerToken);
    http.begin(_versionCheckURL);
    const int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        _logln("http response code: " + String(httpResponseCode));
        String payload = http.getString();

        JsonDocument doc;
        deserializeJson(doc, payload);

        const String version = doc["latest_version"];
        const String url = doc["latest_version_url"];

        http.end();

        return *new VersionInfo{version, url};
    }

    _logln("error sending request. Error code: " + String(httpResponseCode));
    http.end();
    return *new VersionInfo{"", ""};
}

void OTAResolver::_logln(const String& message)
{
    Serial && Serial.println("[ota] " + message);
}

bool OTAResolver::_updateRequired(const String& lastVersion) const
{
    if (_version.isEmpty())
    {
        return true;
    }

    auto parseVersion = [](const String& version, long& major, long& minor, long& patch)
    {
        char* endPtr;
        const char* cStr = version.c_str();

        major = strtol(cStr, &endPtr, 10);
        if (*endPtr != '.') return false;

        minor = strtol(endPtr + 1, &endPtr, 10);
        if (*endPtr != '.') return false;

        patch = strtol(endPtr + 1, &endPtr, 10);
        if (*endPtr != '\0') return false;

        return true;
    };

    long currentMajor, currentMinor, currentPatch;
    long lastMajor, lastMinor, lastPatch;

    if (!parseVersion(_version, currentMajor, currentMinor, currentPatch))
    {
        _logln("parse current version error: `" + _version + "`!");
        return false;
    }

    if (!parseVersion(lastVersion, lastMajor, lastMinor, lastPatch))
    {
        _logln("parse last version error: `" + lastVersion + "`!");
        return false;
    }

    _logln("current version: ma=" + String(currentMajor) + ", mi=" + String(currentMinor) + ", pa=" + String(currentPatch));
    _logln("last version: ma=" + String(lastMajor) + ", mi=" + String(lastMinor) + ", pa=" + String(lastPatch));

    if (lastMajor < currentMajor) return true;
    if (lastMajor == currentMajor && lastMinor > currentMinor) return true;
    if (lastMajor == currentMajor && lastMinor == currentMinor && lastPatch > currentPatch) return true;

    return false;
}

bool OTAResolver::_isValidVersionFormat(const String& version) {
    int dots = 0;
    for (size_t i = 0; i < version.length(); i++) {
        const char c = version[i];
        if (c == '.') {
            dots++;
        } else if (!isDigit(c)) {
            return false;
        }
    }
    return dots == 2;
}

void OTAResolver::_initializeEEPROM() {
    if (!EEPROM.begin(VERSION_EEPROM_SIZE)) {
        _logln("version eeprom initialization error!");
        return;
    }
    for (int i = 0; i < VERSION_EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    _logln("eeprom initialized");
}

void OTAResolver::_saveVersionInMemory(String newVersion) {
    if (!EEPROM.begin(VERSION_EEPROM_SIZE)) {
        _logln("version eeprom error in write!");
        return;
    }

    for (int i = 0; i < newVersion.length() && i < VERSION_EEPROM_SIZE - 1; i++) {
        EEPROM.write(VERSION_ADDR + i, newVersion[i]);
    }

    EEPROM.write(VERSION_ADDR + newVersion.length(), '\0');

    if (!EEPROM.commit()) {
        _logln("version save error!");
    } else {
        _logln("version saved successfully!");
        _version = newVersion;
    }
    EEPROM.end();
}

void OTAResolver::_loadInMemoryVersion() {
    if (!EEPROM.begin(VERSION_EEPROM_SIZE)) {
        _logln("version eeprom error in load!");
        return;
    }

    char buffer[VERSION_EEPROM_SIZE];
    int i = 0;

    while (true) {
        const uint8_t c = EEPROM.read(VERSION_ADDR + i);
        if (c == '\0' || i >= VERSION_EEPROM_SIZE - 1) break;
        buffer[i] = static_cast<char>(c);
        i++;
    }
    buffer[i] = '\0';

    _version = String(buffer);
    EEPROM.end();

    if (!_isValidVersionFormat(_version)) {
        _logln("version format is not valid!");
        _initializeEEPROM();
        return;
    }

    if (_version.length() == 0 || _version[0] == '\0') {
        _logln("version not found in eeprom!");
    } else {
        _logln("loaded version: " + _version);
    }
}
