// Only include if not running in Arduino IDE
// #include <Arduino.h>

// Initialize pin connections
const int potentiometerPin = A0;
const int button0Pin = 2;
const int button1Pin = 3;
const int redLEDpin = 9;
const int greenLEDpin = 10;
const int blueLEDpin = 11;

// Initialize initial state, LED state, and button 0 state
volatile int state = 0;
volatile bool LEDState = 1;
volatile bool button0HeldDown = 0;

// Initialize debounce default values
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;

void setup() {
  Serial.begin(9600);

  // Initialize pin modes
  pinMode(potentiometerPin, INPUT);
  pinMode(button0Pin, INPUT); // Pulldown resistor
  pinMode(button1Pin, INPUT); // Pulldown resistor
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  pinMode(blueLEDpin, OUTPUT);

  // Initialize LED color to white
  setColor(0, 0, 0);

  // Callback when button 0 is pressed
  attachInterrupt(digitalPinToInterrupt(button0Pin), ISRCallback, RISING);

  Serial.println("Setup Complete");
  displayState(state);
}

void loop() {

  if (digitalRead(button0Pin) == LOW) {
    button0HeldDown = 0;
  }

  switch(state) {
    case 0: // Blinking LED
      case0Handler();
      break;

    case 1: // Controlling brightness
      case1Handler();
      break;

    case 2: // Handling serial commands
      case2Handler();
      break;

    default:
      Serial.println("Unable to identify state");
      break;
  }
}

void case0Handler() {
  // Initial button 1 is off
  static bool button1State = 0;

  if (digitalRead(button1Pin) == HIGH) {
    if (!button1State) {
      // Turn LED off
      if (LEDState) {
        LEDState = 0;
        Serial.println("Turning LED off");
        setColor(255, 255, 255);
      }
      // Turn LED on
      else {
        LEDState = 1;
        Serial.println("Turning LED on");
        setColor(0, 0, 0);
      }
    }

    button1State = 1;
    delay(debounceDelay);
  }

  else {
    button1State = 0;
  }
}

void case1Handler() {
  // Read potentiometer and adjust luminosity
  int potential = analogRead(potentiometerPin);
  int luminosity = map(potential, 0, 1023, 0, 255);
  setColor(luminosity, luminosity, luminosity);
  delay(debounceDelay);
}

void case2Handler() {
  if (Serial.available()) {

    // Read string and seperate into color and luminosity
    String incomingString = Serial.readStringUntil('\n');
    Serial.println("Received command: " + incomingString);

    char color = incomingString[0];
    String luminosity = incomingString.substring(1);

    // Check if color is valid
    bool validColor = false;
    if (color == 'r' || color == 'g' || color == 'b') {
      validColor = true;
    }

    // Check if luminosity is valid
    bool validLuminosity = true;
    for (int i = 0; i < luminosity.length(); i++) {
      if (!isDigit(luminosity.charAt(i))) {
        validLuminosity = false;
      }
    }

    int luminosityInt = luminosity.toInt();
    if (luminosityInt < 0 || luminosityInt > 255) {
      validLuminosity = false;
    }

    // Update LED if luminosity and color is valid
    if (validLuminosity && validColor) {
      
      if (color == 'r') {
        analogWrite(redLEDpin, luminosityInt);
        Serial.println("Setting red luminosity to " + String(luminosityInt));
      }
      else if (color == 'g') {
        analogWrite(greenLEDpin, luminosityInt);
        Serial.println("Setting green luminosity to " + String(luminosityInt));
      }
      else {
        analogWrite(blueLEDpin, luminosityInt);
        Serial.println("Setting blue luminosity to " + String(luminosityInt));
      }
    }

    else {
      Serial.println("Command is not valid");
    }
  }
}

void ISRCallback() {
  // Check if callback is not from falling interrupt debounce due to holding down and releasing button 0
  if (button0HeldDown) {
    return;
  }
  button0HeldDown = 1;

  // Only update state if it is not a debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    state = (state + 1) % 3;

    // Reset LED when switching states
    LEDState = 1;
    setColor(0, 0, 0);
    displayState(state);
    lastDebounceTime = millis();
  }
}

void displayState(int state) {
  Serial.println("State machine is now in state " + String(state));
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redLEDpin, redValue);
  analogWrite(greenLEDpin, greenValue);
  analogWrite(blueLEDpin, blueValue);
}