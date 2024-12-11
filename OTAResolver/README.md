# JSON Format
```json
{
    "latest_version": "1.0.1",
    "latest_version_url": "https://example.com/some-path/1.0.1.bin"
}
```

# Code sample
```c++
#include <OTAResolver.h>

OTAResolver ota;

void setup(void) {
  ota.begin("wifi-ssid", "wifi-password", "https://example.com/some-path/version.json", "some-auth", 115200);
  ota.setCheckIntervalSeconds(5); // how often to check for updates
}

void loop(void) {
  ota.tick();
}
```