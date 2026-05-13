#include <WiFiS3.h>

const char* WIFI_SSID     = "Hotspot Di Leo";
const char* WIFI_PASSWORD = "Pablinaa";

// Use your working Script ID here
const String scriptId = "AKfycbwv9RFOYNjLV6F6g3CUYPKRrHGMfxH_fRbu3cSkxhsYp9RVLMWqVUEPcWmLfMCb6rI";
const char* host = "script.google.com";

const int sensorPins[6] = {A0, A1, A2, A3, A4, A5};

// Your specific calibration values
float paOffset = 0;
float refVolt  = 5000.0; 
float vPerUnit = refVolt / 1024.0;
float paPerMV  = 2500.0 / (4500.0 - 500.0);

// Timing
unsigned long sampleInterval = 500; 
unsigned long nextSampleTime = 0;

// Batch settings
const int batchSize = 5;
float pressureBuffer[6][batchSize]; // 6 sensors, 5 samples each
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

  nextSampleTime = millis();
}

void loop() {
  if (millis() >= nextSampleTime) {
    nextSampleTime += sampleInterval;

    // Record data for all 6 sensors
    for (int s = 0; s < 6; s++) {
      float mV = (analogRead(sensorPins[s]) * vPerUnit);
      float pa = ((mV - 500.0) * paPerMV) - 1250 + paOffset;
      pressureBuffer[s][bufferIndex] = pa;
    }
    
    timeBuffer[bufferIndex] = nextSampleTime;
    bufferIndex++;

    Serial.println("Sampled " + String(bufferIndex) + "/" + String(batchSize));

    if (bufferIndex >= batchSize) {
      sendBatch();
      bufferIndex = 0;
    }
  }
}

void sendBatch() {
  if (client.connect(host, 443)) {

    // 1. Build Time string
    String timeData = "";
    for (int i = 0; i < batchSize; i++) {
      timeData += String(timeBuffer[i]);
      if (i < batchSize - 1) timeData += ",";
    }

    // 2. Build Sensor strings
    String pData[6] = {"", "", "", "", "", ""};
    for (int s = 0; s < 6; s++) {
      for (int i = 0; i < batchSize; i++) {
        pData[s] += String(pressureBuffer[s][i], 2);
        if (i < batchSize - 1) pData[s] += ",";
      }
    }

    // 3. Construct URL
    String url = "/macros/s/" + scriptId + "/exec?time=" + timeData;
    for (int s = 0; s < 6; s++) {
      url += "&p" + String(s) + "=" + pData[s];
    }

    // 4. Send HTTP Request
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println();

    Serial.println("Batch sent to Google Sheets");

    // Clean up connection
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 2000) {
      while (client.available()) client.read();
    }
    client.stop();

  } else {
    Serial.println("Connection Failed");
  }
}
