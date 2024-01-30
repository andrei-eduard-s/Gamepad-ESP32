#include <Arduino.h>
#include <BleGamepad.h>

#define DEADZONE 2000  // Prag pentru ignorarea micilor schimbări
#define LED_PIN 26  // pin pentru LED

// ABXY BUTTONS
#define X_BUTTON 32         // A
#define CIRCLE_BUTTON 33    // B
#define TRIANGLE_BUTTON 27  // Y
#define SQUARE_BUTTON 14    // X

// TRIGGERS
#define R1_BUTTON 0
#define R2_BUTTON 0
#define L1_BUTTON 0
#define L2_BUTTON 0

// MENU BUTTONS
#define START_BUTTON 0
#define SELECT_BUTTON 0
#define PS_BUTTON 0

// JOYSTICKS BUTTONS
#define R3_BUTTON 0
#define L3_BUTTON 0

// JOYSTICKS
#define LEFT_VRX_JOYSTICK 12
#define LEFT_VRY_JOYSTICK 13
#define RIGHT_VRX_JOYSTICK 35
#define RIGHT_VRY_JOYSTICK 34

#define NUM_BUTTONS 13


BleGamepad bleGamepad("ESP32 Gamepad", "ESP32");

BleGamepadConfiguration bleGamepadConfig;

uint16_t processJoystickValue(uint16_t currentValue, uint16_t centerValue) {
    if (abs(currentValue - centerValue) < DEADZONE) {
        return centerValue; // Returnează valoarea de centru dacă schimbarea este sub prag
    }
    return currentValue; // Altfel, returnează valoarea curentă
}

int buttonsPins[NUM_BUTTONS] = {X_BUTTON, CIRCLE_BUTTON, TRIANGLE_BUTTON, SQUARE_BUTTON,
                                R1_BUTTON, R2_BUTTON, L1_BUTTON, L2_BUTTON,
                                START_BUTTON, SELECT_BUTTON, PS_BUTTON,
                                R3_BUTTON, L3_BUTTON};

int androidGamepadButtons[NUM_BUTTONS] = {1, 2, 3, 4, 8, 10, 7, 9, 12, 11, 13, 15, 14};
int PCGamepadButtons[NUM_BUTTONS] = {1, 2, 4, 3, 6, 8, 5, 7, 10, 9, 0, 12, 11};

uint16_t leftVrxJoystickLecture = 0;
uint16_t leftVryJoystickLecture = 0;
uint16_t rightVrxJoystickLecture = 0;
uint16_t rightVryJoystickLecture = 0;

uint16_t leftVrxJoystickValue = 0;
uint16_t leftVryJoystickValue = 0;
uint16_t rightVrxJoystickValue = 0;
uint16_t rightVryJoystickValue = 0;

typedef enum {ANDROID, PC} GamepadModes;
GamepadModes gamepadMode = PC;

void setup() {
  delay(1000);
  Serial.begin(115200);

  for(int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonsPins[i], INPUT_PULLUP);
  }

  pinMode(LED_PIN, OUTPUT);  // Setează pinul LED ca ieșire

  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setVid(0xe502);
  bleGamepadConfig.setPid(0xabcd);
  bleGamepadConfig.setHatSwitchCount(4);
  bleGamepad.begin(&bleGamepadConfig);
}

void loop() {
  if(bleGamepad.isConnected()) {
    digitalWrite(LED_PIN, HIGH); // Aprinde LED-ul
    // Joysticks lecture
    leftVrxJoystickLecture = analogRead(LEFT_VRX_JOYSTICK);
    leftVryJoystickLecture = analogRead(LEFT_VRY_JOYSTICK);
    rightVrxJoystickLecture = analogRead(RIGHT_VRX_JOYSTICK);
    rightVryJoystickLecture = analogRead(RIGHT_VRY_JOYSTICK);

    // Compute joysticks value
    leftVrxJoystickValue = processJoystickValue(map(leftVrxJoystickLecture, 0, 4095, 0, 32737), 16368);
    leftVryJoystickValue = processJoystickValue(map(leftVryJoystickLecture, 0, 4095, 0, 32737), 16368);
    rightVrxJoystickValue = processJoystickValue(map(rightVrxJoystickLecture, 0, 4095, 0, 32737), 16368);
    rightVryJoystickValue = processJoystickValue(map(rightVryJoystickLecture, 0, 4095, 0, 32737), 16368);

    switch(gamepadMode) {
      case ANDROID:
        for(int i = 0; i < NUM_BUTTONS; i++) {
          if(!digitalRead(buttonsPins[i])) {
            bleGamepad.press(androidGamepadButtons[i]);   
          } else {
            bleGamepad.release(androidGamepadButtons[i]);     
          }
          joysticksHandlerForMobile(leftVrxJoystickValue, leftVryJoystickValue, rightVrxJoystickValue, rightVryJoystickValue);
        } 
        break;

      case PC:
        for(int i = 0; i < NUM_BUTTONS; i++) {
          if(!digitalRead(buttonsPins[i])) {
            bleGamepad.press(PCGamepadButtons[i]);
          } else {
            bleGamepad.release(PCGamepadButtons[i]);
          }
          joysticksHandlerForPC(leftVrxJoystickValue, leftVryJoystickValue, rightVrxJoystickValue, rightVryJoystickValue);
        }
        break;
    }

    bleGamepad.sendReport();
  }
  else {
    digitalWrite(LED_PIN, 0); // Stinge LED-ul
  }
}

void joysticksHandlerForMobile(uint16_t leftVrx, uint16_t leftVry, uint16_t rightVrx, uint16_t rightVry){
  bleGamepad.setLeftThumb(leftVrx, leftVryJoystickValue);
  bleGamepad.setRightThumb(rightVrxJoystickValue, rightVryJoystickValue);  
}

void joysticksHandlerForPC(uint16_t leftVrx, uint16_t leftVry, uint16_t rightVrx, uint16_t rightVry){
  bleGamepad.setX(leftVrxJoystickValue);
  bleGamepad.setY(leftVryJoystickValue);
  bleGamepad.setZ(rightVrxJoystickValue);
  bleGamepad.setRX(rightVryJoystickValue);
}