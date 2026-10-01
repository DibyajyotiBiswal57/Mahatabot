#include <Servo.h>

// --- Pin Definitions (Same as your wiring) ---
const int TRIG_PIN = 12;
const int ECHO_PIN = 11;
const int SERVO_PIN = 10;
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

// --- Ultra-Short Range Precision Tuning ---
const int SAFE_DISTANCE = 20;     // Trigger scan ONLY if wall is closer than 20cm
const int SCAN_DELAY = 220;        // Quickened snap delay for the servo (220ms)

Servo radarServo;

// Spatial Memory Variables
int distanceLeft = 999;
int distanceCenter = 999;
int distanceRight = 999;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  radarServo.attach(SERVO_PIN);
  radarServo.write(90); // Start locked straight ahead
  delay(600); 
}

void loop() {
  // Keep the sensor eye locked straight ahead while moving forward normally
  radarServo.write(90);
  
  // Read current distance with the new ultra-short range limits
  int currentDistance = readFilteredDistance();

  // If path is clear (nothing within 30cm), keep moving forward
  if (currentDistance > SAFE_DISTANCE) {
    moveForward();
  } 
  // True obstacle detected very close right in front! 
  else {
    moveStop(); // Halt the wheels immediately to scan safely
    delay(100); // Quick settle of momentum
    
    // EXECUTE FAST RADAR SCAN AND FILL SPATIAL MEMORY
    distanceCenter = currentDistance; 
    
    // Snap Left and measure
    radarServo.write(160);
    delay(SCAN_DELAY); 
    distanceLeft = readFilteredDistance();
    
    // Snap Right and measure
    radarServo.write(20);
    delay(SCAN_DELAY);
    distanceRight = readFilteredDistance();
    
    // Re-center the radar eye
    radarServo.write(90);
    delay(150);

    // SMART BRAIN DECISION LOGIC BASED ON THE ULTRA-CLOSE SCAN
    if (distanceLeft > distanceRight) {
      // Left side is clearer! Execute a sharp left pivot
      spinLeft();
      delay(260); // Time to turn away cleanly
    } 
    else if (distanceRight > distanceLeft) {
      // Right side is clearer! Execute a sharp right pivot
      spinRight();
      delay(260); 
    } 
    else {
      // Both sides are tight or blocked! Back up fluidly and turn out
      reverseFluid();
      delay(300);
      spinRight();
      delay(250);
    }
    
    // Reset spatial memory cache before continuing back to the forward loop
    distanceLeft = 999;
    distanceCenter = 999;
    distanceRight = 999;
    
    moveStop();
    delay(100); 
  }
}

// Noise-filtering distance read function with HARD-RESTRICTED PRECISION RANGE
int readFilteredDistance() {
  int totalDistance = 0;
  int validReadings = 0;
  
  for (int i = 0; i < 2; i++) { 
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // 1800 microseconds timeout physically cuts off anything beyond ~30cm!
    // This forms a tight shield against distant echoes or noise.
    long duration = pulseIn(ECHO_PIN, HIGH, 1800); 
    int cm = duration * 0.034 / 2;
    
    if (duration > 0 && cm > 0 && cm <= 30) {
      totalDistance += cm;
      validReadings++;
    }
    delayMicroseconds(150);
  }
  
  if (validReadings > 0) {
    return totalDistance / validReadings;
  }
  return 999; // If out of range (>30cm), treat it as perfectly wide open space
}

// --- Drivetrain Movements ---
void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void spinLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  
}

void spinRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); 
}

void reverseFluid() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void moveStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
