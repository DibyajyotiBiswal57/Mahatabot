#include <Servo.h>

// --- Pin Definitions ---
const int TRIG_PIN = 12;
const int ECHO_PIN = 11;
const int SERVO_PIN = 10;

// Motor Driver Pins (L298N)
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

// --- Performance Tuning Settings ---
const int DISTANCE_THRESHOLD = 30; // Stop/Turn if an obstacle is within 30cm
const int SERVO_SPEED_DELAY = 12;  // Speed of sweep: Lower = faster (milliseconds per step)
int servoDirection = 15;           // Degrees to jump per step. Higher = faster scan, lower accuracy.

Servo radarServo;
int servoAngle = 90;
unsigned long lastServoMoveTime = 0;

void setup() {
  // Initialize Ultrasonic Sensor Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize Motor Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Initialize Servo
  radarServo.attach(SERVO_PIN);
  radarServo.write(servoAngle);
  
  delay(600); // Give the servo time to center before the robot starts moving
}

void loop() {
  // 1. High-Speed Non-Blocking Servo Scan
  if (millis() - lastServoMoveTime >= SERVO_SPEED_DELAY) {
    lastServoMoveTime = millis();
    
    servoAngle += servoDirection;
    // Reverse direction at the 180-degree boundaries
    if (servoAngle >= 165 || servoAngle <= 15) {
      servoDirection = -servoDirection; 
    }
    radarServo.write(servoAngle);
  }

  // 2. Continuous Distance Reading
  int distance = readDistance();

  // 3. Fluid Evasive Steering Logic
  if (distance > 0 && distance < DISTANCE_THRESHOLD) {
    // Obstacle detected on the left side of the vision field
    if (servoAngle > 95) {
      spinRight(); 
      delay(180); // Quick snap-turn to clear the obstacle
    } 
    // Obstacle detected on the right side of the vision field
    else if (servoAngle < 85) {
      spinLeft();  
      delay(180); 
    } 
    // Obstacle is dead ahead
    else {
      reverseFluid();
      delay(250);
      spinRight();
      delay(200);
    }
  } else {
    // Path is completely clear, full speed ahead!
    moveForward(); 
  }
}

// Function to calculate exact distance via soundwaves
int readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // 18ms timeout limits maximum range to ~3 meters so the code never lags out waiting
  long duration = pulseIn(ECHO_PIN, HIGH, 18000); 
  if (duration == 0) return 999; // Assume path is clear if no echo returns
  
  return duration * 0.034 / 2; // Convert duration to centimeters
}

// --- Drivetrain Movement Profiles for 2-Motor + Castor Roller Chassis ---

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void spinLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); // Left wheel backward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  // Right wheel forward
}

void spinRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  // Left wheel forward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); // Right wheel backward
}

void reverseFluid() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void moveStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
