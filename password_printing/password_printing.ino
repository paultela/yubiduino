/* 
 Keyboard Password Printing
 
 When button is pressed, will type "this is your password".
 
 The circuit:
 * button attached from pin 42 to +3.3V
 * 10-kilohm resistor attached from pin 42 to ground
 
 
 Based off of the example from:
 http://www.arduino.cc/en/Tutorial/KeyboardMessage
 */

const int buttonPin = 42;          // input pin for button
int previousButtonState = HIGH;   // for checking the state of a button

void setup() {
  // make the button pin an input:
  pinMode(buttonPin, INPUT);
  Keyboard.begin();
}

void loop() {
  // read the button:
  int buttonState = digitalRead(buttonPin);
  // if the button state has changed,
  if ((buttonState != previousButtonState) && (buttonState == HIGH)) {
    Keyboard.print("this is your password");
  }
  // save the current button state for comparison next time:
  previousButtonState = buttonState;
}
