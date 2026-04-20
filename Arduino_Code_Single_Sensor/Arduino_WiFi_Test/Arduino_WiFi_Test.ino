#include <WiFiS3.h>

const char* WIFI_SSID     = "Pixel_4693";
const char* WIFI_PASSWORD = "Kiaalemtar1";

// JUST the ID from your deployment URL
const String scriptId = "AKfycbzAflNSHzJLlss_OeV9qJeStFWLepOGeiiKrZvRYaBC73e9tjf8hz79jbRbn7DWuzE-EA";
const char* host = "script.google.com";

const int sensorPin = A0;
float paOffset = 15.26;
float refVolt  = 5000.0; 
float vPerUnit = refVolt / 1024.0;
float paPerMV  = 2500.0 / (4500.0 - 500.0); // Simplified calculation

unsigned long lastTime = 0;
unsigned long sampleTime = 5000; // 5 SECONDS - Google needs time to breathe!

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
}

void loop() {
  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    float mV = (analogRead(sensorPin) * vPerUnit);
    float pa = ((mV - 500.0) * paPerMV) - 1250 + paOffset;

    sendToGoogle(lastTime, pa);
  }
}

void sendToGoogle(unsigned long t, float p) {
  if (client.connect(host, 443)) {
    // Construct the path manually to ensure no hidden characters
    String url = "/macros/s/" + scriptId + "/exec?time=" + String(t) + "&p1=" + String(p, 2);
    
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println();

    Serial.println("Sent: " + String(p, 2) + " Pa");
    
    // Briefly read the response to clear the buffer
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 2000) {
      if (client.available()) {
        client.read(); // Just clear the data
      }
    }
    client.stop(); 
  } else {
    Serial.println("Connection Failed");
  }
}