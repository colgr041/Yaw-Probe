#define readPin1 A0

float refVolt = 5000; // TODO: Measure Arduino reference voltage with multimeter and replace the standard value here
float maxVolt = refVolt * 0.9;
float minVolt = refVolt * 0.1;
float vPerUnit = refVolt / 1024.0;
float paPerMV = 2500 / (maxVolt - minVolt);
float paOffset = 15.26; // Currently, that's the value from the thesis, should be replaced by our sensor offset value

float mV = 0;
float pa = 0;

unsigned long lastTime = 0;
unsigned long sampleTime = 50; // Sampling frequency of 20 Hz

void setup() {
  Serial.begin(115200);
  Serial.println("Time_ms,Pa");
}

void loop() {
  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    mV = voltOnPin(readPin1);

    pa = vToPa(mV) + paOffset;


    Serial.print(lastTime);
    Serial.print(",");
    Serial.println(pa, 3);
  }
}

float voltOnPin(int pin) {
  return analogRead(pin) * vPerUnit;
}

float vToPa(float mV_) {
  return ((mV_ - minVolt) * paPerMV) - 1250;
}
