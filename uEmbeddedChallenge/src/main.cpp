#include <Adafruit_CircuitPlayground.h>
#include <Arduino.h>

float movavgx[5]={0};
float movavgy[5]={0};
float movavgz[5]={0};
//make the position data smoother
float AvgX(float val){
  float avg = 0;

  for (int x=1; x<5; x++){
    avg+=movavgx[x];
    movavgx[x-1]=movavgx[x];
  }
  avg+=val;
  avg/=5;

  movavgx[4]=val;
  return avg;
}

float AvgY(float val){
  float avg = 0;

  for (int x=1; x<5; x++){
    avg+=movavgy[x];
    movavgy[x-1]=movavgy[x];
  }
  avg+=val;
  avg/=5;

  movavgy[4]=val;
  return avg;
}

float AvgZ(float val){
  float avg = 0;

  for (int x=1; x<5; x++){
    avg+=movavgz[x];
    movavgz[x-1]=movavgz[x];
  }
  avg+=val;
  avg/=5;

  movavgz[4]=val;
  return avg;
}
/*
void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop() {
  CircuitPlayground.clearPixels();

  delay(500);

  CircuitPlayground.setPixelColor(0, 255,   0,   0);
  CircuitPlayground.setPixelColor(1, 128, 128,   0);
  CircuitPlayground.setPixelColor(2,   0, 255,   0);
  CircuitPlayground.setPixelColor(3,   0, 128, 128);
  CircuitPlayground.setPixelColor(4,   0,   0, 255);
  
  CircuitPlayground.setPixelColor(5, 0xFF0000);
  CircuitPlayground.setPixelColor(6, 0x808000);
  CircuitPlayground.setPixelColor(7, 0x00FF00);
  CircuitPlayground.setPixelColor(8, 0x008080);
  CircuitPlayground.setPixelColor(9, 0x0000FF);
 
  delay(5000);
}

#include <Adafruit_CircuitPlayground.h>

float X, Y, Z, Mod_X, Mod_Y, Mod_Z;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop() {
  X = CircuitPlayground.motionX();
  Y = CircuitPlayground.motionY();
  Z = CircuitPlayground.motionZ();
  Mod_X = AvgX(X);
  Mod_Y = AvgY(Y);
  Mod_Z = AvgZ(Z);

  Serial.print("X: ");
  Serial.print(X);
  Serial.print("  Y: ");
  Serial.print(Y);
  Serial.print("  Z: ");
  Serial.println(Z);

  delay(1000);
}
*/


// 1. 90 degree flip from z to y 
// 2. half circle above from y to x to y(x negative)
// 3. half circle below from y to x to y(x positive)
// 4. full circle
// 5. 

#include <Adafruit_CircuitPlayground.h>

// Thresholds to detect orientation
const float threshold = 2.5;
// Debounce time in milliseconds to prevent multiple detections
const unsigned long debounceTime = 4000;

// States for gesture 2
enum class Gesture2State {
  Start,
  YUp,
  XDown,
  YDown,
  Complete
};

enum class Gesture3State {
  Start,
  YUp,
  XUp,
  YDown,
  Complete
};

// Variables to keep track of the state and last time a gesture was detected
Gesture2State gesture2State = Gesture2State::Start;
Gesture3State gesture3State = Gesture3State::Start;
unsigned long lastGestureTime = 0;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop() {
  float x = CircuitPlayground.motionX();
  float y = CircuitPlayground.motionY();
  float z = CircuitPlayground.motionZ();
  float mod_x = AvgX(x);
  float mod_y = AvgY(y);
  float mod_z = AvgZ(z);
  unsigned long currentTime = millis();

  // Gesture 1: Flip detection
  if (mod_z < -threshold && currentTime - lastGestureTime > debounceTime) {
    Serial.println("Gesture 1 detected: Flip");
    lastGestureTime = currentTime;
  }

  // Gesture 2: Sequence of movements detection
  switch (gesture2State) {
    case Gesture2State::Start:
      if (mod_y > threshold) {
        gesture2State = Gesture2State::YUp;
        Serial.println("Gesture 2 started: Y axis up");
      }
      break;
    case Gesture2State::YUp:
      if (mod_x < -threshold) {
        gesture2State = Gesture2State::XDown;
        Serial.println("Gesture 2 progress: X axis down");
      }
      break;
    case Gesture2State::XDown:
      if (mod_y < -threshold) {
        gesture2State = Gesture2State::YDown;
        Serial.println("Gesture 2 progress: Y axis down");
      }
      break;
    case Gesture2State::YDown:
      // Confirm the gesture is complete
      if (currentTime - lastGestureTime > debounceTime) {
        Serial.println("Gesture 2 detected: Complete");
        lastGestureTime = currentTime;
        gesture2State = Gesture2State::Start; // Reset to start for next detection
      }
      break;
    case Gesture2State::Complete:
      // This state is not used in this setup
      break;
  }
  // Gesture 3: Sequence of movements detection
  switch (gesture3State) {
    case Gesture3State::Start:
      if (mod_y > threshold) {
        gesture3State = Gesture3State::YUp;
        Serial.println("Gesture 3 started: Y axis up");
      }
      break;
    case Gesture3State::YUp:
      if (mod_x > threshold) {
        gesture3State = Gesture3State::XUp;
        Serial.println("Gesture 3 progress: X axis up");
      }
      break;
    case Gesture3State::XUp:
      if (mod_y < -threshold) {
        gesture3State = Gesture3State::YDown;
        Serial.println("Gesture 3 progress: Y axis down");
      }
      break;
    case Gesture3State::YDown:
      if (currentTime - lastGestureTime > debounceTime) {
        Serial.println("Gesture 3 detected: Complete");
        lastGestureTime = currentTime;
        gesture3State = Gesture3State::Start; // Reset to start for next detection
      }
      break;
    case Gesture3State::Complete:
      // This state is not used in this setup
      break;
  }


  delay(100);
}


