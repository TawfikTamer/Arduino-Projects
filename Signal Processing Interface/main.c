#include <LiquidCrystal.h>

// Pin definitions for the LCD
#define RS 8
#define E 9
#define d4 4
#define d5 5
#define d6 6
#define d7 7

// Pin definitions for the rotary encoder and button
#define A A3
#define B A5
#define EnButton A4

// Global variables
int modcount = 1; // Current mode counter
int button;

float gain = 1;   // Gain value
float offset = 0; // Offset value
float signal;     // Signal value
float y;          // Processed signal value

float min, max;   // Minimum and maximum signal values
int reset = 0;    // Reset flag

char Current_Encouder_Read = 0; // Current encoder reading
char Encoder_cycle = 0b011;     // Encoder cycle state
int encoder_state;              // Encoder state
int encoder_button;             // Encoder button state
int encoder_button_state = 0;   // Encoder button press state

// Initialize the LCD
LiquidCrystal lcd(RS, E, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2); // Set up the LCD with 16 columns and 2 rows
  lcd.print("Press \"Select\"");
  lcd.setCursor(0, 1);
  lcd.print("to start");

  // Set up pins for the encoder and button
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(EnButton, INPUT);
}

void loop() {
  // Check if the select button is pressed to start or switch modes
  if (SelectIsPressed()) {
    lcd.clear();
    switch (modcount) {
      case 1:
        Mode1();
        break;
      case 2:
        Mode2();
        break;
      case 3:
        Mode3();
        break;
      case 4:
        Mode4();
        break;
    }
  }
}

// Mode 1: Signal processing and display
void Mode1() {
  while (SelectIsPressed()) {}
  lcd.print("Mode 1");

  while (modcount == 1) {
    signal = analogRead(A1); // Read the signal from analog pin A1
    lcd.setCursor(0, 1);
    lcd.print("signal: ");
    y = gain * ((signal * 5 / 1024.0) + offset); // Process the signal
    lcd.print(y);

    if (reset == 0) {
      min = 100;
      max = -100;
      reset = 1;
    }
    if (y < min) min = y;
    if (y > max) max = y;

    Changebutton(); // Check for button presses to change mode
  }
}

// Mode 2: Adjust gain
void Mode2() {
  while (SelectIsPressed()) {}
  float change = 0.5;
  lcd.print("Mode 2");
  lcd.setCursor(12, 0);
  lcd.print("+");
  lcd.setCursor(0, 1);
  lcd.print("gain = ");

  while (modcount == 2) {
    encoder_state = Encoder();
    ChangeEncoderButton();

    // Adjust change and gain values based on encoder and button inputs
    if (RightIsPressed() || ((encoder_state == 1) && (encoder_button_state == 1))) {
      change += 0.1;
      if (RightIsPressed()) delay(300);
      if (change > 3) change = 3;
    }
    if (LeftIsPressed() || ((encoder_state == 2) && (encoder_button_state == 1))) {
      change -= 0.1;
      if (LeftIsPressed()) delay(300);
      if (change < 0.1) change = 0.1;
    }
    if (UpIsPressed() || ((encoder_state == 1) && (encoder_button_state == 0))) {
      gain += change;
      if (UpIsPressed()) delay(300);
      if (gain > 99.9) gain = 99.9;
    }
    if (DownIsPressed() || ((encoder_state == 2) && (encoder_button_state == 0))) {
      gain -= change;
      if (DownIsPressed()) delay(300);
      if (gain < 0) gain = 0.01;
    }
    lcd.setCursor(13, 0);
    lcd.print(change);
    lcd.setCursor(7, 1);
    lcd.print(gain);
    Changebutton();
  }
}

// Mode 3: Adjust offset
void Mode3() {
  while (SelectIsPressed()) {}
  float change = 0.5;
  lcd.print("Mode 3");
  lcd.setCursor(11, 0);
  lcd.print("+");
  lcd.setCursor(0, 1);
  lcd.print("offset = ");

  while (modcount == 3) {
    encoder_state = Encoder();
    ChangeEncoderButton();

    // Adjust change and offset values based on encoder and button inputs
    if (RightIsPressed() || ((encoder_state == 1) && (encoder_button_state == 1))) {
      change += 0.05;
      if (RightIsPressed()) delay(300);
      if (change > 2) change = 2;
    }
    if (LeftIsPressed() || ((encoder_state == 2) && (encoder_button_state == 1))) {
      change -= 0.05;
      if (LeftIsPressed()) delay(300);
      if (change < 0.05) change = 0.05;
    }
    if (UpIsPressed() || ((encoder_state == 1) && (encoder_button_state == 0))) {
      offset += change;
      if (UpIsPressed()) delay(300);
      if (offset > 2.5) offset = 2.5;
    }
    if (DownIsPressed() || ((encoder_state == 2) && (encoder_button_state == 0))) {
      offset -= change;
      if (DownIsPressed()) delay(300);
      if (offset < -2.5) offset = -2.5;
    }
    lcd.setCursor(12, 0);
    lcd.print(change);
    lcd.setCursor(9, 1);
    lcd.print(offset);
    Changebutton();
  }
}

// Mode 4: Display and reset min/max signal values
void Mode4() {
  while (SelectIsPressed()) {}
  lcd.print("Mode 4");

  while (modcount == 4) {
    if (RightIsPressed()) {
      delay(2000);
      if (RightIsPressed()) {
        max = 0;
        min = 0;
        reset = 0;
        lcd.setCursor(15, 0);
        lcd.print(" ");
        lcd.setCursor(15, 1);
        lcd.print(" ");
      }
    }
    lcd.setCursor(7, 0);
    lcd.print("Max:");
    lcd.print(max);
    lcd.setCursor(7, 1);
    lcd.print("Min:");
    lcd.print(min);
    Changebutton();
  }
}

// Functions to handle button presses and mode changes
void Changebutton() {
  if (SelectIsPressed()) {
    modcount++;
  }
  if (modcount > 4) modcount = 1;
}

// Button press detection functions
int SelectIsPressed() {
  button = analogRead(A0);  // Select = 720
  return (button > 700 && button < 990) ? 1 : 0;
}

int LeftIsPressed() {
  button = analogRead(A0);  // Left = 480
  return (button > 400 && button < 650) ? 1 : 0;
}

int DownIsPressed() {
  button = analogRead(A0);  // Down = 308
  return (button > 260 && button < 350) ? 1 : 0;
}

int UpIsPressed() {
  button = analogRead(A0);  // Up = 132
  return (button > 100 && button < 200) ? 1 : 0;
}

int RightIsPressed() {
  button = analogRead(A0);  // Right = 0
  return (button < 50) ? 1 : 0;
}

// Encoder functions
int Encoder() {
  Current_Encouder_Read = digitalRead(A) << 1 | digitalRead(B);
  if ((Encoder_cycle & 0b11) != (Current_Encouder_Read & 0b11)) {
    Encoder_cycle = Encoder_cycle << 2 | Current_Encouder_Read & 0b11;

    // 00 | 10 | 11 | 01 | 00 for CW
    // 00 | 01 | 11 | 10 | 00 for CCW
    if ((Encoder_cycle & 0b111111) == 0b000111 || (Encoder_cycle & 0b111111) == 0b111000 || (Encoder_cycle & 0b111111) == 0b001011 || (Encoder_cycle & 0b111111) == 0b110100) {
      return ((Encoder_cycle & 0b111111) == 0b001011 || (Encoder_cycle & 0b111111) == 0b110100) ? 1 : 2;
    }
  }
  return 0;
}

int EncoderIsPressed() {
  encoder_button = digitalRead(EnButton);
  return (encoder_button == 0) ? 1 : 0;
}

void ChangeEncoderButton() {
  if (EncoderIsPressed()) {
    delay(10);
    while (EncoderIsPressed()) {}
    encoder_button_state = !encoder_button_state;
  }
}
