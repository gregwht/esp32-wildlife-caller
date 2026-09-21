const int speakerPowerPin = 10;

void setup() {
  Serial.begin(115200);
  pinMode(speakerPowerPin, OUTPUT);
  digitalWrite(speakerPowerPin, LOW); // start off
}

void loop() {
  digitalWrite(speakerPowerPin, HIGH);
  Serial.println("GPIO10: HIGH (LED/amp should be ON)");
  delay(4000);

  digitalWrite(speakerPowerPin, LOW);
  Serial.println("GPIO10: LOW (LED/amp should be OFF)");
  delay(4000);
}
