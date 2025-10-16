#include <Arduino.h>
/**STEPPER MOTOR 
 *          WHITE = COMMON RED
 * 
 *   (from bridge = to stepper motor)
 * MOTOR A:
 *          Green = Pink
 *         Purple = Orange
 * MOTOR B:
 *          Yellow = Yellow
 *          Blue = Blue
 */

#define IN1 14 // blue, motor A - B 
#define IN2 27 // purple, motor A - A
#define IN3 16 // yellow, motor B - B 
#define IN4 17 // green, motor B - A

void setup(){
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
}

void step(int stepNumber){
    switch(stepNumber){
        case 0:
            digitalWrite(IN1, HIGH);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, LOW);
            break;
        case 1:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, LOW);
            break;
        case 2:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, HIGH);
            break;
        case 3:
            digitalWrite(IN1, HIGH);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, HIGH);
            break;
    }
}

void loop(){

    for(int i = 0; i < 4; i++){
        step(i);
        delay(2);
    }

    /*
    for(int i = 3; i>= 0; i--){
        step(i);
        
    }
    */

}