#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

// HX710B pins
const int dataPin = 4;
const int clockPin = 5;

// Multiplex pins
const int pinA = 12;
const int pinB = 13;
const int pinC = 14;

// Input pin to gate actions
const int gatePin = 15;   // choose a free GPIO

int sequence[][3] = {
  {0, 0, 1},  // 001 → humidity
  {0, 1, 1},  // 011 → temperature
  {1, 0, 1}   // 101 → pressure
};

int seqIndex = 0;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);   // second UART

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);

  pinMode(gatePin, INPUT);   // external condition pin

  // HX710B setup
  pinMode(dataPin, INPUT);
  pinMode(clockPin, OUTPUT);

  // AHT10 setup
  Wire.begin(21, 22);
  if (!aht.begin()) {
    Serial.println("Sensor error!");
    while (1);
  }
}

long readHX710B() {
  long result = 0;
  while (digitalRead(dataPin) == HIGH);
  for (int i = 0; i < 24; i++) {
    digitalWrite(clockPin, HIGH);
    result = (result << 1) | digitalRead(dataPin);
    digitalWrite(clockPin, LOW);
  }
  digitalWrite(clockPin, HIGH);
  delayMicroseconds(1);
  digitalWrite(clockPin, LOW);
  return result;
}

void loop() {
  // Wait until gatePin is LOW before proceeding
  while (digitalRead(gatePin) == HIGH) {
    // Optional: print status or just idle
    Serial.println("Gate pin HIGH → waiting...");
    delay(100); // small delay to avoid busy loop
  }

  // Apply current sequence
  digitalWrite(pinA, sequence[seqIndex][0]);
  digitalWrite(pinB, sequence[seqIndex][1]);
  digitalWrite(pinC, sequence[seqIndex][2]);

  // Read sensors
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  long pressureRaw = readHX710B();

  // Choose value based on pattern
  int valueToSend = 0;
  if (seqIndex == 0) {        // 001 → humidity
    valueToSend = (int)humidity.relative_humidity;
  } else if (seqIndex == 1) { // 011 → temperature
    valueToSend = (int)temp.temperature;
  } else if (seqIndex == 2) { // 101 → pressure
    valueToSend = (int)(pressureRaw % 10000); // keep within 4 digits
  }

  // Format as 4 digits
  char formatted[5]; // 4 digits + null terminator
  sprintf(formatted, "%04d", valueToSend);

  // Debug + send
  Serial.print("Pattern: ");
  Serial.print(sequence[seqIndex][0]);
  Serial.print(sequence[seqIndex][1]);
  Serial.print(sequence[seqIndex][2]);
  Serial.print(" → ");
  Serial.println(formatted);

  Serial2.println(formatted);

  // Next sequence
  seqIndex++;
  if (seqIndex >= 3) seqIndex = 0;

  delay(1000);
}
