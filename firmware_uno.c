
// These have to be 2 and 3, that's where interrupts 0 and 1 are hooked up.
int leftInputPin = 2;
int rightInputPin = 3;
int modeInputPin = 4;

int leftOutputPin = 10;
int rightOutputPin = 11;
int modeOutputPin = 12;

int led = 13;
 
volatile int leftInputState = LOW;
volatile int rightInputState = LOW;
volatile int modeInputState = LOW;

#define LEFT 0
#define RIGHT 1

int lastButtonPressedAlone;

int chargeMode = 1;

int leftOutputState;
int rightOutputState;
int modeOutputState;

int desiredLeftOutputState;
int desiredRightOutputState;
int desiredModeOutputState;

#define USE_INTERRUPTS 0

void setup()
{
  pinMode(leftInputPin, INPUT);
  pinMode(rightInputPin, INPUT);
  pinMode(modeInputPin, INPUT);

  // Turn on internal pull-ups for the two input GPIOs.
  digitalWrite(leftInputPin, HIGH);
  digitalWrite(rightInputPin, HIGH);
  digitalWrite(modeInputPin, HIGH);

  // Prime the input state; don't want to end up with inverted buttons
  // if we power the stick on with a button pressed.
  leftInputState = !digitalRead(leftInputPin);
  rightInputState = !digitalRead(rightInputPin);
  modeInputState = !digitalRead(modeInputPin);

#if USE_INTERRUPTS == 1
  attachInterrupt(0, onLeftChange, CHANGE);
  attachInterrupt(1, onRightChange, CHANGE);
#endif
 
  pinMode(leftOutputPin, OUTPUT);
  pinMode(rightOutputPin, OUTPUT);
  pinMode(modeOutputPin, OUTPUT);

  pinMode(led, OUTPUT);

  // Primt output state
  digitalWrite(leftOutputPin, LOW);
  digitalWrite(rightOutputPin, LOW);
  digitalWrite(modeOutputPin, LOW);

  digitalWrite(led, chargeMode);

  leftOutputState = LOW;
  rightOutputState = LOW;
  modeOutputState = LOW;
}

#if USE_INTERRUPTS == 1
void onLeftChange()
{
  leftInputState = !digitalRead(leftInputPin);
}

void onRightChange()
{
  rightInputState = !digitalRead(rightInputPin);
}
#endif

void loop()
{
  // Poor man's debouncing.
  delay(1);

#if USE_INTERRUPTS == 0
  leftInputState = !digitalRead(leftInputPin);
  rightInputState = !digitalRead(rightInputPin);
#endif

  modeInputState = !digitalRead(modeInputPin);

  // Toggle charge mode if mode pressed, and pass it through.
  if (modeInputState != desiredModeOutputState) {
    desiredModeOutputState = modeInputState;
    if (modeInputState) {
      chargeMode = !chargeMode;
      digitalWrite(led, chargeMode);
    }
  }

  if (leftInputState ^ rightInputState) {
    // One button is pressed alone; save it.
    if (leftInputState) {
      lastButtonPressedAlone = LEFT;
    }
    if (rightInputState) {
      lastButtonPressedAlone = RIGHT;
    }
  }

  // Do we have two buttons pressed?
  if (chargeMode && leftInputState && rightInputState) {
    // If so, arbitrate.
    if (lastButtonPressedAlone == LEFT) {
      desiredLeftOutputState = LOW;
      desiredRightOutputState = HIGH;
    } else {
      desiredLeftOutputState = HIGH;
      desiredRightOutputState = LOW;
    }
  } else {
    // Otherwise, pass through.
    desiredLeftOutputState = leftInputState;
    desiredRightOutputState = rightInputState;
  }

  // Commit state to output pins.
  if (desiredLeftOutputState != leftOutputState) {
    digitalWrite(leftOutputPin, desiredLeftOutputState);
    leftOutputState = desiredLeftOutputState;
  }
  if (desiredRightOutputState != rightOutputState) {
    digitalWrite(rightOutputPin, desiredRightOutputState);
    rightOutputState = desiredRightOutputState;
  }
  if (desiredModeOutputState != modeOutputState) {
    digitalWrite(modeOutputPin, desiredModeOutputState);
    modeOutputState = desiredModeOutputState;
  }
}
