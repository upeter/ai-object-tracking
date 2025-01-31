#include <Arduino.h>

// Define the LED pin
const int ledPin = 2; // Onboard LED pin for ESP32

void setup() {
    // Initialize the LED pin as an output
    pinMode(ledPin, OUTPUT);

    // Initialize serial communication at 115200 baud rate
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB port only
    }

    // Print a message to the serial monitor
    Serial.println("ESP32 Blink Sketch");
}

void loop() {
    // Turn the LED on
    digitalWrite(ledPin, HIGH);
    // Print a message to the serial monitor
    Serial.println("LED is ON");
    // Wait for a second
    delay(1000);

    // Turn the LED off
    digitalWrite(ledPin, LOW);
    // Print a message to the serial monitor
    Serial.println("LED is OFF");
    // Wait for a second
    delay(1000);
}