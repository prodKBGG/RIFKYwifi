#include "RIFKYwifi.h"

RIFKYwifi::RIFKYwifi() : server(80) {}

void RIFKYwifi::PengaturanAwal(const char* apName, const char* apPassword) {
    EEPROM.begin(512);  // Initialize EEPROM
    loadCredentials();

    if (!connectWiFi()) {
        startAP(apName, apPassword);
        setupPortal();
    }
}

void RIFKYwifi::begin() {
    if (!connectWiFi()) {
        Serial.println("Failed to connect, starting AP");
        startAP("RetroAP", "12345678");
    }
}

void RIFKYwifi::startAP(const char* apName, const char* apPassword) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName, apPassword);
    Serial.println("Access Point started");
    Serial.println("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Start DNS server to redirect captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());  // Capture all DNS requests
}

void RIFKYwifi::setupPortal() {
    server.on("/", std::bind(&RIFKYwifi::handleRoot, this));
    server.on("/save", std::bind(&RIFKYwifi::handleSave, this));
    server.on("/scan", std::bind(&RIFKYwifi::handleScan, this));  // New endpoint for SSID scanning

    // Handle not found requests and redirect to a custom domain
    server.onNotFound([this]() {
        server.sendHeader("Location", "http://rifkyirfanda.com", true);
        server.send(302, "text/plain", "");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void RIFKYwifi::handleRoot() {
    String ssidFromUrl = server.arg("ssid");  // Get SSID from URL if present

    String page = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>";
    
    // Modern background and mobile-friendly layout
    page += "body { background-color: #f5f5f5; color: #333; font-family: Arial, sans-serif; margin: 0; padding: 20px; }";
    page += "h1 { color: #0056b3; text-align: center; font-size: 24px; }";
    page += "form { background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }";
    page += "input[type='text'], input[type='password'] { width: calc(100% - 22px); padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; font-size: 16px; }";
    page += "input[type='submit'], button { background-color: #0056b3; color: white; border: none; padding: 10px 20px; border-radius: 5px; font-size: 16px; cursor: pointer; width: 100%; margin: 10px 0; }";
    page += "input[type='submit']:hover, button:hover { background-color: #004499; }";
    
    // Responsive and mobile-friendly
    page += "@media (max-width: 600px) { body { padding: 10px; } form { width: 100%; padding: 15px; } }";
    
    // Checkbox style
    page += ".toggle { margin: 10px 0; }";
    
    page += "</style></head>";
    page += "<body><h1>Wi-Fi Setup</h1>";
    
    // Form to input SSID and password
    page += "<form action='/save' method='POST'>";
    page += "SSID: <input type='text' id='ssid' name='ssid' value='" + ssidFromUrl + "' required><br>";
    page += "Password: <input type='password' id='password' name='password' required><br>";
    
    // Checkbox to toggle password visibility
    page += "<div class='toggle'><input type='checkbox' onclick='togglePassword()'> Show Password</div><br>";
    
    // Submit button
    page += "<input type='submit' value='Connect'>";
    page += "</form>";
    
    // Button to scan SSIDs
    page += "<br><button onclick='location.href=\"/scan\"'>Scan for Wi-Fi</button>";

    // JavaScript for password visibility toggle
    page += "<script>";
    page += "function togglePassword() { var passField = document.getElementById('password'); if (passField.type === 'password') { passField.type = 'text'; } else { passField.type = 'password'; } }";
    page += "</script>";
    
    page += "</body></html>";
    server.send(200, "text/html", page);
}




void RIFKYwifi::handleScan() {
    int n = WiFi.scanNetworks();
    String page = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>";
    
    // Modern and responsive layout for scan results
    page += "body { background-color: #f5f5f5; color: #333; font-family: Arial, sans-serif; margin: 0; padding: 20px; }";
    page += "h1 { color: #0056b3; text-align: center; font-size: 24px; }";
    page += "table { width: 100%; border-collapse: collapse; max-width: 600px; margin: auto; background: #fff; padding: 20px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
    page += "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }";
    page += "td button { background-color: #0056b3; color: white; border: none; padding: 5px 10px; border-radius: 5px; font-size: 16px; cursor: pointer; }";
    page += "td button:hover { background-color: #004499; }";

    page += "@media (max-width: 600px) { body { padding: 10px; } table { width: 100%; padding: 10px; } }";
    page += "</style></head><body>";
    
    page += "<h1>Wi-Fi Networks</h1>";
    
    if (n == 0) {
        page += "<p>No networks found.</p>";
    } else {
        // Create table for scanned SSIDs
        page += "<table><tr><th>SSID</th><th>Signal Strength</th></tr>";

        for (int i = 0; i < n; ++i) {
            page += "<tr>";
            
            // SSID button - clicking on the SSID will auto-fill it in the setup page
            page += "<td><button onclick=\"location.href='/?ssid=" + WiFi.SSID(i) + "';\">";
            page += WiFi.SSID(i);
            page += "</button></td>";
            
            // Signal strength
            page += "<td>" + String(WiFi.RSSI(i)) + " dBm</td>";
            page += "</tr>";
        }
        page += "</table>";
    }
    
    // Button to return to setup
    page += "<br><button onclick='location.href=\"/\"'>Back to Setup</button>";
    page += "</body></html>";
    server.send(200, "text/html", page);
}



void RIFKYwifi::handleSave() {
    ssid = server.arg("ssid");
    password = server.arg("password");
    this->saveCredentials(ssid, password);
    server.send(200, "text/html", "<h1>Credentials Saved! Restarting...</h1>");
    ESP.restart();
}

void RIFKYwifi::saveCredentials(String ssid, String pass) {
    // Clear EEPROM
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }

    // Write SSID
    for (int i = 0; i < ssid.length(); i++) {
        EEPROM.write(i, ssid[i]);
    }
    EEPROM.write(ssid.length(), '\0');  // Null-terminate the SSID

    // Write Password, starting at address 100
    for (int i = 0; i < pass.length(); i++) {
        EEPROM.write(100 + i, pass[i]);
    }
    EEPROM.write(100 + pass.length(), '\0');  // Null-terminate the password

    EEPROM.commit();  // Commit changes to EEPROM
}


void RIFKYwifi::loadCredentials() {
    char storedSSID[50];  // Buffer for SSID
    char storedPass[50];  // Buffer for Password

    // Read SSID from EEPROM
    for (int i = 0; i < 50; i++) {
        storedSSID[i] = EEPROM.read(i);
        if (storedSSID[i] == '\0') break;  // Stop at null character
    }
    ssid = String(storedSSID);  // Convert to String

    // Read Password from EEPROM (starting at address 100)
    for (int i = 0; i < 50; i++) {
        storedPass[i] = EEPROM.read(100 + i);
        if (storedPass[i] == '\0') break;  // Stop at null character
    }
    password = String(storedPass);  // Convert to String
}


bool RIFKYwifi::connectWiFi() {
    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: "); Serial.println(ssid);
    Serial.print("Password: "); Serial.println(password);

    WiFi.begin(ssid.c_str(), password.c_str());
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 20) {
        delay(500);
        Serial.print(".");
        counter++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
    } else {
        Serial.println("Failed to connect to WiFi.");
    }
    return WiFi.status() == WL_CONNECTED;
}

void RIFKYwifi::handleClient() {
    dnsServer.processNextRequest();  // Process DNS for captive portal
    server.handleClient();  // Handle incoming requests
}
