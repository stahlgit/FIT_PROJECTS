#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ESP32Servo.h>


 // Definition GPIO pin for servo
#define SERVO_PIN 13 
// Definition GPIO pin for stepper 
#define STEPPER_IN1 14 
#define STEPPER_IN2 27
#define STEPPER_IN3 16
#define STEPPER_IN4 17

#define EEPROM_SIZE 1
// network credentails

const char* apSSID = "ESP32_Control";
const char* apPassword = "password";
int lastServoAngle = 0;

//web server object on port 80 (should be free)
WebServer server(80);
Servo myServo;


//make it volatile, so it is not optimised
volatile bool stepperRunning = false;
volatile int stepperDirection = 1;
volatile unsigned long stepInterval = 2;
unsigned long lastStepTime = 0;
unsigned long startTime = 0;
unsigned long endTime = 0;
float rpm = 0;
int stepsCounter = 0;


void setupStepper() {
    pinMode(STEPPER_IN1, OUTPUT);
    pinMode(STEPPER_IN2, OUTPUT);
    pinMode(STEPPER_IN3, OUTPUT);
    pinMode(STEPPER_IN4, OUTPUT);
}

void step(int stepNumber){
    switch(stepNumber){
        case 0:
            digitalWrite(STEPPER_IN1, HIGH);
            digitalWrite(STEPPER_IN2, LOW);
            digitalWrite(STEPPER_IN3, HIGH);
            digitalWrite(STEPPER_IN4, LOW);
            break;
        case 1:
            digitalWrite(STEPPER_IN1, LOW);
            digitalWrite(STEPPER_IN2, HIGH);
            digitalWrite(STEPPER_IN3, HIGH);
            digitalWrite(STEPPER_IN4, LOW);
            break;
        case 2:
            digitalWrite(STEPPER_IN1, LOW);
            digitalWrite(STEPPER_IN2, HIGH);
            digitalWrite(STEPPER_IN3, LOW);
            digitalWrite(STEPPER_IN4, HIGH);
            break;
        case 3:
            digitalWrite(STEPPER_IN1, HIGH);
            digitalWrite(STEPPER_IN2, LOW);
            digitalWrite(STEPPER_IN3, LOW);
            digitalWrite(STEPPER_IN4, HIGH);
            break;
    }
}

void handleRoot() {
    String lastAngle = String(lastServoAngle);
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Control Panel</title>
    <style>
        body {
            display: flex;
            flex-direction: column; 
            align-items: center;
            min-height: 100vh;
            background-color: goldenrod
        }
        .knob-container {
            position: relative;
            width: 150px;
            height: 150px;
        }
        .knob {
            width: 100px;
            height: 100px;
            border: 5px solid #333;
            border-radius: 80%;
            background: linear-gradient( #555, #222);
            position: relative;
            transform: translate(-50%, -50%);
            cursor: grab; 
            user-select: none; 
            top: 50%;
            left: 50%;
        }
        .knob::before { 
            content: "";
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%);
            width: 6px;
            height: 40%; 
            background-color: #fff;
            border-radius: 3px;
        }
        .value-display {
            margin-bottom: 20px;
            text-align: center;
            font-size: 20px;
        }
        .buttons {
            display: flex;
            flex-direction: row;
            justify-content: center;
            gap: 10px;
            margin-bottom: 20px;
        }
        .buttons button {
            font-size: 18px; 
            padding: 10px 20px; 
            border: none;
            border-radius: 5px;
            background-color: #555;
            color: white;
            cursor: pointer;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }
        .buttons button:hover {
            background-color: #666;
        }
        input[type="range"] {
            width: 300px;
            height: 20px;
            background: #ddd;
            border-radius: 5px;
            outline: none;
        }
    </style>
</head>
<body>
    <h1>Servo Control</h1>
    <div class="value-display" id="servoValue">Last angle: )=====";

    html += lastAngle;
    html += R"=====(</div>


    <div class="knob-container">
        <div class="knob" id="servoKnob"></div>
    </div>

    <h1>Stepper Control</h1>
    <div class="buttons">
        <button onclick="sendStepperCommand('start')">Activate</button>
        <button onclick="sendStepperCommand('stop')">Stop</button>
        <button onclick="sendStepperCommand('forward')">Forward</button>
        <button onclick="sendStepperCommand('backward')">Backward</button> <p></p>

    </div>
    Speed: <input type="range" id="speedSlider" min="1" max="9" value="5">
    <p>RPM: <span id="rpmValue">0</span></p>

    <script>
        const lastAngle = )=====" +lastAngle+ R"=====(
        const knob = document.getElementById('servoKnob');
        const servoValueDisplay = document.getElementById('servoValue');
        let isDragging = false;
        let startY;
        let currentAngle = lastAngle;
        servoValueDisplay.textContent = currentAngle;

        const speedSlider = document.getElementById("speedSlider");
        const rpmDisplay = document.getElementById("rpmValue");

        //set initial knob position
        knob.style.transform = `translate(-50%, -50%) rotate(${currentAngle}deg)`;

        knob.addEventListener('mousedown', (e) => {
            isDragging = true;
            startY = e.clientY;
        });

        document.addEventListener('mousemove', (e) => {
            if (!isDragging) return;

            const deltaY = e.clientY - startY;
            let newAngle = currentAngle - deltaY;

            newAngle = Math.max(0, Math.min(180, newAngle));

            knob.style.transform = `translate(-50%, -50%) rotate(${newAngle}deg)`;
            servoValueDisplay.textContent = Math.round(newAngle);
            fetch('/servo?angle=' + Math.round(newAngle)); // Send angle to server
        });

        document.addEventListener('mouseup', () => {
            isDragging = false;
            currentAngle = parseFloat(knob.style.transform.replace('translate(-50%, -50%) rotate(', '').replace('deg)', ''));
        });

        knob.addEventListener('dragstart', (e) => {
            e.preventDefault();
        });


        function sendStepperCommand(command) {
            fetch('/stepper?cmd=' + command + '&speed=' + speedSlider.value);
        }

        setInterval(function() {
            fetch('/rpm')
            .then(response => response.text())
            .then(data => rpmDisplay.textContent = data);
        }, 500);
    </script>
</body>
</html>
    )=====";
    server.send(200, "text/html", html);
}


void handleServo() {
    if (server.hasArg("angle")) {
        int angle = server.arg("angle").toInt();
        if (angle >= 0 && angle <= 180) {
            myServo.write(angle);
            server.send(200, "text/plain", "OK");
            Serial.print("Servo angle set to: ");
            Serial.println(angle);
            lastServoAngle = angle;
            
            EEPROM.write(0, lastServoAngle);
            EEPROM.commit();
            } 
        else {
            server.send(400, "text/plain", "Invalid angle (0-180)");
            Serial.println("Invalid angle received!");
        }
    } 
    else {
    server.send(400, "text/plain", "Angle parameter missing");
    Serial.println("Angle Parameter missing!");
    }
}


void handleStepper() {
    if (server.hasArg("cmd")) {
        String command = server.arg("cmd");
        if(server.hasArg("speed")){
            stepInterval = 11 - server.arg("speed").toInt();
            if(stepInterval < 2){ //safety check
                stepInterval = 2;
            }
        }
        if (command == "start") {
            stepperRunning = true;
            startTime = millis();
        } 
        else if (command == "stop") {
            stepperRunning = false;
            endTime = millis();
            rpm = 0;
            stepsCounter = 0;
        } 
        else if (command == "forward") {
            stepperDirection = 1;
        } 
        else if (command == "backward") {
            stepperDirection = -1;
        }
        server.send(200, "text/plain", "OK");
    } 
    else {
        server.send(400, "text/plain", "Command parameter missing");
    }
}

void handleRPM() {
    server.send(200, "text/plain", String(rpm));
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}

void setup() {
    Serial.begin(115200);
    delay(3000);

    EEPROM.begin(EEPROM_SIZE);
    lastServoAngle = EEPROM.read(0);
    if(lastServoAngle < 0 || lastServoAngle > 180){
        lastServoAngle = 0;
    }

    myServo.attach(SERVO_PIN);
    myServo.write(lastServoAngle);
    setupStepper();

    // Access Point
    WiFi.softAP(apSSID, apPassword);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: \n");
    Serial.print(IP);
    Serial.print("\n");

    // Set up the web server
    server.on("/", handleRoot); //handling client
    server.on("/servo", handleServo); //handle servo endpoint
    server.on("/stepper", handleStepper); // handle stepper endpoint
    server.on("/rpm", handleRPM); //show rpm endpoint
    server.onNotFound(handleNotFound); //if not found
    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    server.handleClient(); // Handle client requests
    if (stepperRunning) {
        if (millis() - lastStepTime >= stepInterval) {
            lastStepTime = millis();
            static int currentStep = 0;
            currentStep += stepperDirection;
            if (currentStep > 3) currentStep = 0;
            if (currentStep < 0) currentStep = 3;
            step(currentStep);
            stepsCounter++;
            endTime = millis();
            if(endTime - startTime >= 1000){
                rpm = (float)stepsCounter / 200 * 60; //200 steps per rotation
                stepsCounter = 0;
                startTime = millis();
            }
        }
    }
}