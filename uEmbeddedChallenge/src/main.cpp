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

// 1. flip starting from z upward
// 2. half circle above from y to x to y(x negative)
// 3. half circle below from y to x to y(x positive)
// 4. rapidly moving in the x_axis, with y_axis up
// 5. rapidly moving in the z_axis, with y_axis up
// 6. first tilted to the left (X-axis negative), then rolled over the top (Z-axis positive), and finally tilted to the right (X-axis positive).
// 7. Flip followed by rapid movement in Y-axis
// 8. X facing up, rotating till Y facing up
// 9. Involves tilting the device left (X-axis negative), then right (X-axis positive), and finally left again.
// 10. Involves tilting the device forward (Y-axis positive), then backward (Y-axis negative), and then forward again.

int message = -1;

// Thresholds to detect orientation
const float threshold = 2.5;
const float rapidMovementThreshold = 4.0;
// Debounce time in milliseconds to prevent multiple detections
const unsigned long debounceTime = 4000;
bool gestureDetected = false;
// States for Gesture 2 and Gesture 3
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

enum class Gesture6State {
  Start,
  LeftTilt,
  RollOverTop,
  RightTilt,
  Complete
};
enum class Gesture7State {
  Start,
  FlipDetected,
  RapidYMovement,
  Complete
};

enum class Gesture8State {
  Start,
  XUp,
  YUp,
  Complete
};
enum class Gesture9State {
  Start,
  LeftTilt,
  RightTilt,
  LeftTiltAgain,
  Complete
};

enum class Gesture10State {
  Start,
  ForwardTilt,
  BackwardTilt,
  ForwardTiltAgain,
  Complete
};



// Variables to keep track of the state and last time a gesture was detected
Gesture2State gesture2State = Gesture2State::Start;
Gesture3State gesture3State = Gesture3State::Start;
Gesture6State gesture6State = Gesture6State::Start;
Gesture7State gesture7State = Gesture7State::Start;
Gesture8State gesture8State = Gesture8State::Start;
Gesture9State gesture9State = Gesture9State::Start;
Gesture10State gesture10State = Gesture10State::Start;


bool gesture2InProgress = false;
bool gesture3InProgress = false;
unsigned long lastGestureTime = 0;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop() {
  if (gestureDetected) {
    CircuitPlayground.clearPixels();


    CircuitPlayground.setPixelColor(message-1, 255,   0,   0);
    return;
  }
  // Raw accelerometer values
  float x = CircuitPlayground.motionX();
  float y = CircuitPlayground.motionY();
  float z = CircuitPlayground.motionZ();

  // Smoothen data
  float mod_x = AvgX(x);
  float mod_y = AvgY(y);
  float mod_z = AvgZ(z);

  unsigned long currentTime = millis();

  // Gesture 1: Flip
  if (mod_z < -threshold && currentTime - lastGestureTime > debounceTime) {
    Serial.println("Gesture 1 detected: Flip");
    lastGestureTime = currentTime;
    // Reset gesture states
    gesture2State = Gesture2State::Start;
    gesture3State = Gesture3State::Start;
    gesture2InProgress = false;
    gesture3InProgress = false;

    gestureDetected = true;
    message = 1;
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
      if (currentTime - lastGestureTime > debounceTime) {
        Serial.println("Gesture 2 detected: Complete");
        lastGestureTime = currentTime;
        gesture2State = Gesture2State::Start; // Reset to start for next detection
        gestureDetected = true;
        message = 2;
      }
      break;
    case Gesture2State::Complete:

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
        gestureDetected = true;
        message = 3;
      }
      break;
    case Gesture3State::Complete:

      break;


  }
    // Gesture 4: Rapid movement in X axis with Y up
  if (gesture2InProgress && abs(mod_x) > rapidMovementThreshold) {
    Serial.println("Gesture 4 detected: Rapid X movement with Y up");
    gesture2State = Gesture2State::Start;
    gesture3State = Gesture3State::Start;
    gesture2InProgress = false;
    gesture3InProgress = false;

    gestureDetected = true;
    message = 4;
  }

  // Gesture 5: Rapid movement in Z axis with Y up
  if (gesture2InProgress && abs(mod_z) > rapidMovementThreshold) {
    Serial.println("Gesture 5 detected: Rapid Z movement with Y up");
    gesture2State = Gesture2State::Start;
    gesture3State = Gesture3State::Start;
    gesture2InProgress = false;
    gesture3InProgress = false;

    gestureDetected = true;
    message = 5;
  }
  // Gesture 6 - Roll
  switch (gesture6State) {
    case Gesture6State::Start:
      if (x < -threshold) {
        gesture6State = Gesture6State::LeftTilt;
      }
      break;
    case Gesture6State::LeftTilt:
      if (z > threshold) {
        gesture6State = Gesture6State::RollOverTop;
      }
      break;
    case Gesture6State::RollOverTop:
      if (x > threshold) {
        gesture6State = Gesture6State::RightTilt;
      }
      break;
    case Gesture6State::RightTilt:
      Serial.println("Gesture 6 detected: Roll");
      gesture6State = Gesture6State::Complete;
      gestureDetected = true;
      message = 6;
      break;
    case Gesture6State::Complete:
      // Reset state
      gesture6State = Gesture6State::Start;
      break;

  }

  // Gesture 7: Flip followed by rapid movement in Y-axis
  switch (gesture7State) {
    case Gesture7State::FlipDetected:
      if (abs(mod_y) > rapidMovementThreshold) {
        gesture7State = Gesture7State::RapidYMovement;
      }
      break;
    case Gesture7State::RapidYMovement:
      Serial.println("Gesture 7 detected: Flip followed by rapid Y movement");
      gesture7State = Gesture7State::Complete;
      gestureDetected = true;
      message = 7;
      break;
    case Gesture7State::Complete:
      // Reset state
      gesture7State = Gesture7State::Start;
      break;

  }

  // Gesture 8: X facing up, rotating till Y facing up
  switch (gesture8State) {
    case Gesture8State::Start:
      if (mod_x > threshold) {
        gesture8State = Gesture8State::XUp;
      }
      break;
    case Gesture8State::XUp:
      if (mod_y > threshold) {
        gesture8State = Gesture8State::YUp;
      }
      break;
    case Gesture8State::YUp:
      Serial.println("Gesture 8 detected: X up to Y up rotation");
      gesture8State = Gesture8State::Complete;
      gestureDetected = true;
      message = 8;
      break;
    case Gesture8State::Complete:
      // Reset state
      gesture8State = Gesture8State::Start;
      break;

  }
    // Gesture 9 - Zig-Zag
  switch (gesture9State) {
    case Gesture9State::Start:
      if (x < -threshold) {
        gesture9State = Gesture9State::LeftTilt;
      }
      break;
    case Gesture9State::LeftTilt:
      if (x > threshold) {
        gesture9State = Gesture9State::RightTilt;
      }
      break;
    case Gesture9State::RightTilt:
      if (x < -threshold) {
        gesture9State = Gesture9State::LeftTiltAgain;
      }
      break;
    case Gesture9State::LeftTiltAgain:
      Serial.println("Gesture 9 detected: Zig-Zag");
      gesture9State = Gesture9State::Complete;
      gestureDetected = true;
      message = 9;
      break;
    case Gesture9State::Complete:
      // Reset state
      gesture9State = Gesture9State::Start;
      break;

  }

  // Gesture 10 - Swoop
  switch (gesture10State) {
    case Gesture10State::Start:
      if (y > threshold) {
        gesture10State = Gesture10State::ForwardTilt;
      }
      break;
    case Gesture10State::ForwardTilt:
      if (y < -threshold) {
        gesture10State = Gesture10State::BackwardTilt;
      }
      break;
    case Gesture10State::BackwardTilt:
      if (y > threshold) {
        gesture10State = Gesture10State::ForwardTiltAgain;
      }
      break;
    case Gesture10State::ForwardTiltAgain:
      Serial.println("Gesture 10 detected: Swoop");
      gesture10State = Gesture10State::Complete;
      gestureDetected = true;
      message = 10;
      break;
    case Gesture10State::Complete:
      // Reset state
      gesture10State = Gesture10State::Start;
      break;


  }



  delay(100);
}


