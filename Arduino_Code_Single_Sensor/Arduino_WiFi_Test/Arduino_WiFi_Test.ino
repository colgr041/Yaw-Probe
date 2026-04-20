#include <WiFiS3.h>

// WiFi Credentials
const char* WIFI_SSID     = "Pixel_4693";
const char* WIFI_PASSWORD = "Kiaalemtar1";


// Google Apps Script ID (Extracted from your URL)
const char* host = "script.google.com";
const String scriptId = "AKfycbwYZ_Syd-xtPo38JfX8YVcFsLa_PMY2h4M0FNJ_htyq7S97lZGC8o_w89wQ5iQ2aaWfug";

// Sensor pin
const int sensorPin = A0;

// Offset & Conversion
float paOffset = 15.26;
float refVolt  = 5000.0; 
float maxVolt  = refVolt * 0.9;
float minVolt  = refVolt * 0.1;
float vPerUnit = refVolt / 1024.0;
float paPerMV  = 2500.0 / (maxVolt - minVolt);

// Timing - INCREASED TO 2 SECONDS to prevent Google blocking you
unsigned long lastTime = 0;
unsigned long sampleTime = 2000; 

// CRITICAL: Use WiFiSSLClient for HTTPS
WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    // Sensor math
    float mV = (analogRead(sensorPin) * vPerUnit);
    float pa = ((mV - minVolt) * paPerMV) - 1250 + paOffset;

    sendDataToGoogle(lastTime, pa);
  }
}

void sendDataToGoogle(unsigned long timeVal, float pressure) {
  Serial.println("\nConnecting to Google...");

  if (client.connect(host, 443)) {
    Serial.println("Connected to server.");

    // Create the path for the GET request
    // IMPORTANT: Do NOT include "https://script.google.com" in the GET string
    String url = "/macros/s/" + scriptId + "/exec";
    url += "?time=" + String(timeVal);
    url += "&p1=" + String(pressure, 3);

    // Send HTTP Request
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: " + String(host) + "\r\n");
    client.print("User-Agent: ArduinoUnoR4\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n"); // End of headers

    Serial.println("Data sent. Waiting for response...");

    // Read response (Optional, helps debugging)
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break; // Headers finished
      }
    }
    String line = client.readStringUntil('\n');
    Serial.println("Server Response: " + line);

  } else {
    Serial.println("Connection to Google failed.");
  }
  
  client.stop(); // Close connection
}

