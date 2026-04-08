#define readPin1 A0

float refVolt = 5000; // TODO: Measure Arduino reference voltage with multimeter and replace the standard value here
float maxVolt = refVolt * 0.9;
float minVolt = refVolt * 0.1;
float vPerUnit = refVolt / 1024.0;
float paPerMV = 2500 / (maxVolt - minVolt);
float paOffset = -21.3; // Depends on the sensor, but that is roughly the value we observed for one of the sensors

float mV = 0;
float pa = 0;



unsigned long lastTime = 0;
unsigned long sampleTime = 100; // Sampling frequency of 20 Hz

void setup() {
  Serial.begin(115200);
  //Serial.println("Time_ms,Pa");

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
