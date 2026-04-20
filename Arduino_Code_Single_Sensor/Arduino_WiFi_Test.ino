#include <WiFiS3.h>

// WiFi
const char* WIFI_SSID     = "HotspotName";
const char* WIFI_PASSWORD = "HotspotPassword";

// Google Apps Script URL
const char* SCRIPT_URL = "https://script.google.com/a/macros/liuformulastudent.se/s/AKfycbzcoRDyBfO1Hcdgptk7WB1JkkOOY6EASXmN3cq4tBwQGqlMAuNQ06c6Bqk8QjobqHaPVg/exec";

// Sensor pin
const int sensorPin = A0;

// Offset
float paOffset = 15.26; // change to value of sensor

// Conversion
float refVolt  = 5000.0; // measure Arduino reference voltage
float maxVolt  = refVolt * 0.9;
float minVolt  = refVolt * 0.1;
float vPerUnit = refVolt / 1024.0;
float paPerMV  = 2500.0 / (maxVolt - minVolt);

// Timing
unsigned long lastTime = 0;
unsigned long sampleTime = 50;

// HTTP client
WiFiClient client;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  Serial.println("Time_ms,Pressure");
}

void loop() {

  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    // Sensor read
    float mV = voltOnPin(sensorPin);
    float pa = vToPa(mV) + paOffset;

    // Build URL
    String url = String(SCRIPT_URL);
    url += "?time=" + String(lastTime);
    url += "&p1=" + String(pa, 3);

    // Send to Google
    if (client.connect("script.google.com", 80)) { // HTTP port 80

      client.print(
        String("GET ") + url + " HTTP/1.1\r\n" +
        "Host: script.google.com\r\n" +
        "Connection: close\r\n\r\n"
      );

      Serial.println("Sent:");
      Serial.println(url);
    }
  }
}

// ADC to mV
float voltOnPin(int pin) {
  return analogRead(pin) * vPerUnit;
}

// mV to Pa
float vToPa(float mV_) {
  return ((mV_ - minVolt) * paPerMV) - 1250;
}

