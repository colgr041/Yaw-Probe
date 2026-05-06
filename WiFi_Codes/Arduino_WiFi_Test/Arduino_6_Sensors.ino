#include <WiFiS3.h>

const char* WIFI_SSID     = "Hotspot Di Leo";
const char* WIFI_PASSWORD = "Pablinaa";

const String scriptId = "AKfycbxE_sUwXWHQ3hDi8FSbz7GFitqB_oUvsKnhC_mb9QbFsoPlSex9I8ZLZfTi9aFJ7a_u";
const char* host = "script.google.com";

const int sensorPins[6] = {A0, A1, A2, A3, A4, A5};

float refVolt  = 5000.0;
float vPerUnit = refVolt / 1024.0;

// Timing
unsigned long sampleInterval = 1000;
unsigned long nextSampleTime = 0;

// Batch
const int batchSize = 5;
unsigned long timeBuffer[batchSize];
float sensorBuffer[6][batchSize]; // [sensor][sample]

int bufferIndex = 0;

WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);

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

    timeBuffer[bufferIndex] = nextSampleTime;

    for (int s = 0; s < 6; s++) {
      float mV = analogRead(sensorPins[s]) * vPerUnit;
      sensorBuffer[s][bufferIndex] = mV; // raw voltage (or convert if needed)
    }

    Serial.println("Buffered sample " + String(bufferIndex));
    bufferIndex++;

    if (bufferIndex >= batchSize) {
      sendBatch();
      bufferIndex = 0;
    }
  }
}

void sendBatch() {
  if (client.connect(host, 443)) {

    String timeData = "";
    String sData[6] = {"", "", "", "", "", ""};

    for (int i = 0; i < batchSize; i++) {
      timeData += String(timeBuffer[i]);

      for (int s = 0; s < 6; s++) {
        sData[s] += String(sensorBuffer[s][i], 2);
        if (i < batchSize - 1) {
          sData[s] += ",";
        }
      }

      if (i < batchSize - 1) {
        timeData += ",";
      }
    }

    String url = "/macros/s/" + scriptId + "/exec?";
    url += "time=" + timeData;
    url += "&s0=" + sData[0];
    url += "&s1=" + sData[1];
    url += "&s2=" + sData[2];
    url += "&s3=" + sData[3];
    url += "&s4=" + sData[4];
    url += "&s5=" + sData[5];

    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("Connection: close");
    client.println();

    Serial.println("Sent batch (6 sensors)");

    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 2000) {
      while (client.available()) client.read();
    }

    client.stop();

  } else {
    Serial.println("Connection Failed");
  }
}