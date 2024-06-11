#include <LiquidCrystal.h>

// LCD pin definitions
#define RS 8
#define E 9
#define d4 4
#define d5 5
#define d6 6
#define d7 7

// Output pin and interrupt pin definitions
#define out_pin 3
#define int_pin 2

// Number of cycles to run
#define cycles_num 1

// Initialize the LCD
LiquidCrystal lcd(RS, E, d4, d5, d6, d7);

// Variables to store pulse counts and timing
volatile int pulses;
unsigned long timer = 0;

// Arrays to store flow rates (F) and durations (T)
float F[5] = { 0, 0, 0, 10, 30 };
float T[5] = { 4, 3, 6, 7, 5 };

// Variables to store the number of cycles, current cycle, flow rate (Q), and volume (V)
int num_of_cycles;
int i = 0;
float Q = 0;
float V = 0;

void setup() {
  // Initialize the LCD and set up the pins
  lcd.begin(16, 2);
  pinMode(out_pin, OUTPUT);
  pinMode(int_pin, INPUT_PULLUP);
  
  // Attach an interrupt to the interrupt pin
  attachInterrupt(digitalPinToInterrupt(int_pin), interrupt, FALLING);
  
  // Initialize the timer
  timer = millis();
  delay(1000);
}

void loop() {
  // Loop through the defined number of cycles
  for (; i < cycles_num; i++) {
    // Loop through the flow rate and duration arrays
    for (int c = 0; c < 5; c++) {
      // Calculate the number of cycles for the current flow rate and duration
      num_of_cycles = F[c] * T[c];
      
      // If the flow rate is 0, perform a special case calculation
      if (F[c] == 0) {
        lcd.setCursor(0, 0);
        lcd.print("Q=");
        Q = pulses * 1 / 7.5;
        V = V + Q / 60;
        lcd.print(Q);
        lcd.setCursor(7, 0);
        lcd.print("L/m");
        lcd.setCursor(12, 0);
        lcd.print("F:");
        lcd.print(pulses);
        lcd.print("  ");
        
        // Reset pulse count and timer
        pulses = 0;
        timer = millis();
        delay(T[c] * 1000);
      }
      
      // Loop through the number of cycles
      for (int j = 0; j <= num_of_cycles; j++) {
        // Check if one second has passed to update the flow rate and volume display
        if (millis() - timer > 1000) {
          lcd.setCursor(0, 0);
          lcd.print("Q=");
          Q = pulses * 1 / 7.5;
          V = V + Q / 60;
          lcd.print(Q);
          lcd.setCursor(7, 0);
          lcd.print("L/m");
          lcd.setCursor(12, 0);
          lcd.print("F:");
          lcd.print(pulses);
          lcd.print("  ");
          
          // Reset pulse count and timer
          pulses = 0;
          timer = millis();
        }
        
        // Generate the signal based on the current flow rate
        delay(1000 / (F[c] * 2));
        digitalWrite(out_pin, HIGH);
        delay(1000 / (F[c] * 2));
        digitalWrite(out_pin, LOW);
      }
    }
    // Display the total volume at the end of all cycles
    lcd.setCursor(0, 1);
    lcd.print("V=");
    lcd.print(V);
    lcd.setCursor(7, 1);
    lcd.print("L");
  }

  // Update the flow rate and volume display every second
  if (millis() - timer >= 1000) {
    lcd.setCursor(0, 0);
    lcd.print("Q=");
    Q = pulses * 1 / 7.5;
    V = V + Q / 60;
    lcd.print(Q);
    lcd.setCursor(7, 0);
    lcd.print("L/m");
    lcd.setCursor(12, 0);
    lcd.print("F:");
    lcd.print(pulses);
    lcd.print("  ");
    
    lcd.setCursor(0, 1);
    lcd.print("V=");
    lcd.print(V);
    lcd.setCursor(7, 1);
    lcd.print("L");
    
    // Reset pulse count and timer
    pulses = 0;
    timer = millis();
  }
}

// Interrupt service routine to count pulses
void interrupt() {
  pulses++;
}
