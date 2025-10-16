#include <ESP32Servo.h>

#define SERVO_PIN 13  // Define the GPIO pin for the servo

Servo myServo;  // Create a Servo object

void setup() {
    myServo.attach(SERVO_PIN);  // Attach the servo to the defined pin
}

void loop() {
    // Move servo to 0 degrees
    myServo.write(0);
    delay(1000);  // Wait for 1 second

    // Move servo to 90 degrees
    myServo.write(90);
    delay(1000);  // Wait for 1 second

    // Move servo to 180 degrees
    myServo.write(200);
    delay(1000);  // Wait for 1 second
}

// nepoužívať H-bridge