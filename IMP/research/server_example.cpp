#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// network credentails
const char* ssid = "TP-Link_1FEC";
const char* password = "35514916";

//web server object on port 80 (should be free)
WebServer server(8000);


void handleRoot() {
    server.send(200, "text/html", "<h1>Welcome to ESP32 Web Server!</h1>");
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}

void setup() {
    Serial.begin(115200);
    delay(3000);

    // Connect to Wi-Fi
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Set up the web server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    server.handleClient(); // Handle client requests
}