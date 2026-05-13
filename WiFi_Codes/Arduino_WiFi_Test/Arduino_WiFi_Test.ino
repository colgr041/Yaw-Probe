#include <WiFiS3.h>

const char* WIFI_SSID     = "Hotspot Di Leo";
const char* WIFI_PASSWORD = "Pablinaa";

const String scriptId = "AKfycbzz3uQY9JE_uuTOaGcC3E7SyIkEbvtXA0z1syZgD3Duag-zFfyQmityN4lhFQuh_Skivw";
const char* host = "script.google.com";

const int sensorPin = A0;

float paOffset = 0;
float refVolt  = 5000.0; 
float vPerUnit = refVolt / 1024.0;
float paPerMV  = 2500.0 / (4500.0 - 500.0);

// Timing
unsigned long sampleInterval = 250; // 1 second
unsigned long nextSampleTime = 0;

// Batch settings
const int batchSize = 40; 
float pressureBuffer[batchSize];
unsigned long timeBuffer[batchSize];
int bufferIndex = 0;

WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  analogReadResolution(10);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  nextSampleTime = millis(); // initialize schedule
}

void loop() {
  // Precise sampling (no drift)
  if (millis() >= nextSampleTime) {
    nextSampleTime += sampleInterval;

    float mV = (analogRead(sensorPin) * vPerUnit);
    float pa = ((mV - 500.0) * paPerMV) - 1250 + paOffset;

    pressureBuffer[bufferIndex] = pa;
    timeBuffer[bufferIndex] = nextSampleTime; // fixed timestamp
    bufferIndex++;

    Serial.println("Buffered: " + String(pa, 2));

    if (bufferIndex >= batchSize) {
      sendBatch();
      bufferIndex = 0;
    }
  }
}

void sendBatch() {
  if (client.connect(host, 443)) {

    String timeData = "";
    String pressureData = "";

    for (int i = 0; i < batchSize; i++) {
      timeData += String(timeBuffer[i]);
      pressureData += String(pressureBuffer[i], 2);

      if (i < batchSize - 1) {
        timeData += ",";
        pressureData += ",";
      }
    }

    String url = "/macros/s/" + scriptId + "/exec?time=" + timeData + "&p1=" + pressureData;

    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println();

    Serial.println("Sent batch");

    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 2000) {
      while (client.available()) {
        client.read();
      }
    }

    client.stop();

  } else {
    Serial.println("Connection Failed");
  }
}
