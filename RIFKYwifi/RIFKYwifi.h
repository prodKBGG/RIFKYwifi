#ifndef RIFKYwifi_h
#define RIFKYwifi_h

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>  // Use this for ESP8266 boards

class RIFKYwifi {
public:
    RIFKYwifi();
    void PengaturanAwal(const char* apName, const char* apPassword);
    void begin();
    void handleClient();

private:
    void startAP(const char* apName, const char* apPassword);
    void setupPortal();
    void saveCredentials(String ssid, String password);
    void loadCredentials();
    bool connectWiFi();

    // Functions to handle HTTP requests
    void handleRoot();
    void handleSave();
    void handleScan();  // New function for scanning SSIDs

    ESP8266WebServer server;
    DNSServer dnsServer;

    String ssid;
    String password;
};

#endif
