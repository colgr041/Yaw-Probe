#include <WiFiS3.h>
#include <WiFiUdp.h>

// ─── CONFIGURATION — edit these to match your setup ───────────────────────────
const char* WIFI_SSID     = "YourHotspotName";       // Phone hotspot SSID
const char* WIFI_PASSWORD = "YourHotspotPassword";   // Phone hotspot password
const char* RECEIVER_IP   = "192.168.1.100";         // IP of the receiving device
const int   RECEIVER_PORT = 4210;                    // Port on the receiving device
// ──────────────────────────────────────────────────────────────────────────────

// Sensor pins
const int sensorPins[6] = {A0, A1, A2, A3, A4, A5};

// Zero-point offset correction for each sensor (Pa)
float paOffset[6] = {15.26, 15.26, 15.26, 15.26, 15.26, 15.26};

// Pressure conversion constants
float refVolt  = 5000.0;                          // Arduino reference voltage in mV
float maxVolt  = refVolt * 0.9;                   // 4500 mV
float minVolt  = refVolt * 0.1;                   // 500 mV
float vPerUnit = refVolt / 1024.0;                // ~4.88 mV per ADC unit
float paPerMV  = 2500.0 / (maxVolt - minVolt);    // 0.625 Pa/mV

// Timing
unsigned long lastTime   = 0;
unsigned long sampleTime = 50;                    // 50ms = 20 Hz

// WiFi & UDP
WiFiUDP udp;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(RECEIVER_PORT);

  // Print CSV header to Serial for local debugging
  Serial.println("Time_ms,Pa_S1,Pa_S2,Pa_S3,Pa_S4,Pa_S5,Pa_S6");
}

void loop() {
  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    // Read all 6 sensors
    float pa[6];
    for (int i = 0; i < 6; i++) {
      float mV = voltOnPin(sensorPins[i]);
      pa[i]    = vToPa(mV) + paOffset[i];
    }

    // Build CSV string: "Time_ms,Pa1,Pa2,Pa3,Pa4,Pa5,Pa6"
    String data = String(lastTime);
    for (int i = 0; i < 6; i++) {
      data += ",";
      data += String(pa[i], 3);
    }

    // Send via UDP
    udp.beginPacket(RECEIVER_IP, RECEIVER_PORT);
    udp.print(data);
    udp.endPacket();

    // Also print locally for debugging
    Serial.println(data);
  }
}

// Converts raw ADC reading to millivolts
float voltOnPin(int pin) {
  return analogRead(pin) * vPerUnit;
}

// Converts millivolts to Pascal
float vToPa(float mV_) {
  return ((mV_ - minVolt) * paPerMV) - 1250;
}