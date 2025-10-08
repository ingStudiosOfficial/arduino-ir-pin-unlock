// All the libraries to include
#include <IRremote.h>
#include <Servo.h>

// General setup
bool debugMode = false;

// Pin code setup
const int pinLength = 4;
int pinCode[pinLength] = {};
int userInputtedCode[pinLength] = {};
bool pinCodeCorrect = false;

// LED setup
const int ledPins[pinLength] = {8, 9, 10, 11};

// IRremote setup
const int irPin = 7;
IRrecv irrecv(irPin);
decode_results results;

// Servo setup
Servo doorlock;
int doorlockPin = 6;

void setup() {
  Serial.begin(9600);
  
  // Start the receiver
  irrecv.enableIRIn();

  // Attach the servo
  doorlock.attach(doorlockPin);
  doorlock.write(120);

  // Set the LED pins to output
  for (int i = 0; i < pinLength; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Seed the random number generator
  long seed = 0;
  for (int i = 0; i < 100; i++) {
    seed += analogRead(A0);
  }
  randomSeed(seed);

  // Generate the random pin code
  generateRandomCode();
  
  if (debugMode == true) {
    Serial.print("Pin code generated: ");
    for (int i = 0; i < pinLength; i++) {
      Serial.print(pinCode[i]);
      if (i < pinLength - 1) {
        Serial.print(", ");
      }
    }
  }
  Serial.println("");
}

void loop() {
  if (pinCodeCorrect == false) {
    getUserInputCode();
    verifyInputtedCode();
    if (pinCodeCorrect == true) {
      Serial.println("Pin code correct, welcome!");
      openDoor();
    } else {
      Serial.println("Pin code incorrect, please try again.");
    }
  }
}
void generateRandomCode() {
  for (int i = 0; i < pinLength; i++) {
    pinCode[i] = random(0, 10);
  }
}

int decodePressedButton(unsigned long hexCode) {
  switch (hexCode) {
    case 0xFF9867: return 0;
    case 0xFFA25D: return 1;
    case 0xFF629D: return 2;
    case 0xFFE21D: return 3;
    case 0xFF22DD: return 4;
    case 0xFF02FD: return 5;
    case 0xFFC23D: return 6;
    case 0xFFE01F: return 7;
    case 0xFFA857: return 8;
    case 0xFF906F: return 9;
    default: return 404;
  }
}

void getUserInputCode() {
  // Clear the array
  for (int i = 0; i < pinLength; i++) {
    userInputtedCode[i] = 0;
  }
  
  for (int i = 0; i < pinLength; i++) {
    while (true) {
      if (irrecv.decode(&results)) {
        if (results.value != 0xFFFFFFFF) {  // Ignore repeat codes
          if (debugMode == true) {
            Serial.println(results.value, HEX);
          }

          int decodedCode = decodePressedButton(results.value);
          Serial.print("Decoded code: ");
          Serial.println(decodedCode);
          
          if (decodedCode == 404) {
            Serial.println("Input invalid, please try again.");
          } else {
            userInputtedCode[i] = decodedCode;
            irrecv.resume();
            break;  // Move to next digit
          }
        }
        irrecv.resume();
      }
    }
  }

  Serial.print("User inputted code: ");
  for (int i = 0; i < pinLength; i++) {
    Serial.print(userInputtedCode[i]);
    if (i < pinLength - 1) {
      Serial.print(",");
    }
  }
  Serial.println("");
}

void verifyInputtedCode() {
  int userVerifiedCode[pinLength] = {0, 0, 0, 0};
  pinCodeCorrect = true;

  // Run through each entered number to check against the actual number
  for (int i = 0; i < pinLength; i++) {
    if (userInputtedCode[i] == pinCode[i]) {
      digitalWrite(ledPins[i], HIGH);
      userVerifiedCode[i] = 1;  // Mark as correct
    } else {
      digitalWrite(ledPins[i], LOW);
      userVerifiedCode[i] = 0;  // Mark as incorrect
      pinCodeCorrect = false;   // At least one digit is wrong
    }
  }
}

void openDoor() {
  doorlock.write(0);
}
