#define readPin1 A0
#define readPin2 A1

float refVolt = 5000; // TODO: Measure Arduino reference voltage with multimeter and replace the standard value here
float maxVolt = refVolt * 0.9;
float minVolt = refVolt * 0.1;
float vPerUnit = refVolt / 1024.0;
float paPerMV = 2500 / (maxVolt - minVolt);
float paOffset = 15.26;

float mV1 = 0;
float mV2 = 0;
float pa1 = 0;
float pa2 = 0;
float deltaP = 0;

unsigned long lastTime = 0;
unsigned long sampleTime = 50;

void setup() {
  Serial.begin(115200);
  Serial.println("Time_ms,Pa1,Pa2,DeltaP");
}

void loop() {
  if (millis() - lastTime >= sampleTime) {
    lastTime = millis();

    mV1 = voltOnPin(readPin1);
    mV2 = voltOnPin(readPin2);

    pa1 = vToPa(mV1) + paOffset;
    pa2 = vToPa(mV2) + paOffset;

    deltaP = pa1 - pa2;

    Serial.print(lastTime);
    Serial.print(",");
    Serial.print(pa1, 3);
    Serial.print(",");
    Serial.print(pa2, 3);
    Serial.print(",");
    Serial.println(deltaP, 3);
  }
}

float voltOnPin(int pin) {
  return analogRead(pin) * vPerUnit;
}

float vToPa(float mV_) {
  return ((mV_ - minVolt) * paPerMV) - 1250;
}
