#include <RIFKYwifi.h>

RIFKYwifi wifiManager;

void setup() {
    Serial.begin(115200);
    wifiManager.PengaturanAwal("RetroAP", "12345678");
}

void loop() {
    wifiManager.handleClient(); // Handle client requests
}
