#include "math.h" // Include math library for mathematical functions
#include "string.h" // Include string library for string manipulation functions
#include <LiquidCrystal.h> // Include LiquidCrystal library for interfacing with LCD
#define RS 8 // Pin number for Register Select (RS) on LCD
#define E 9 // Pin number for Enable (E) on LCD
#define d4 4 // Pin number for data bit 4 (d4) on LCD
#define d5 5 // Pin number for data bit 5 (d5) on LCD
#define d6 6 // Pin number for data bit 6 (d6) on LCD
#define d7 7 // Pin number for data bit 7 (d7) on LCD
LiquidCrystal lcd(RS, E, d4, d5, d6, d7); // Initialize the LCD object
int modcount = 1; // Mode counter to keep track of current mode
int button; // Variable to store button state
#define numSamples 200 // Number of samples to collect
#define CAP_SIZE 6 // Size of capacitor array

// Arrays to store samples from signals
volatile int samples1[numSamples]; 
volatile int samples2[numSamples]; 
volatile int sampleIndex = 0; // Sample index counter

// Variables for storing the best combination of capacitors
int bestCombination[CAP_SIZE]; 
long int bestSum = 0;
int bestCount = CAP_SIZE + 1;
float numbers[CAP_SIZE] = { 100, 16, 40, 124, 3, 9 }; // Array to store numbers

// Variables for power calculations
float I_rms = 0;
float V_rms = 0;
float P_avg = 0;
float PF = 0;
int i = 0, j = 0;

int state;
float timer;
float Qc;
float Q;
float Qerror = 0;
float thetaN;
float thetaO;

float PFreq = 0.95; // Power factor frequency

void setup() {
  lcd.begin(16, 2); // Initialize the LCD with 16 columns and 2 rows
  pinMode(A2, INPUT); // Set pin A2 as input
  pinMode(A1, INPUT); // Set pin A1 as input
  pinMode(A5, OUTPUT); // Set pin A5 as output
  pinMode(A4, OUTPUT); // Set pin A4 as output
  pinMode(A3, OUTPUT); // Set pin A3 as output
  pinMode(1, OUTPUT); // Set pin 1 as output
  pinMode(2, OUTPUT); // Set pin 2 as output
  pinMode(3, OUTPUT); // Set pin 3 as output
  digitalWrite(A5, LOW); // Set pin A5 to LOW
  digitalWrite(A4, LOW); // Set pin A4 to LOW
  digitalWrite(A3, LOW); // Set pin A3 to LOW
  digitalWrite(1, LOW); // Set pin 1 to LOW
  digitalWrite(2, LOW); // Set pin 2 to LOW
  digitalWrite(3, LOW); // Set pin 3 to LOW

  // Configure Timer1 for interrupt at 2000 Hz (500 µs interval)
  noInterrupts(); // Disable all interrupts
  TCCR1A = 0; // Clear Timer1 control registers
  TCCR1B = 0;
  TCNT1 = 0; // Initialize counter value to 0
  OCR1A = 3999; // Compare match register for 500us (16 MHz / (8 * (3999 + 1)))
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11); // 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt
  interrupts(); // Enable all interrupts
  timer = millis(); // Initialize timer
}

void loop() {
  lcd.clear(); // Clear the LCD display
  switch (modcount) {
    case 1:
      {
        Mode1(); // Enter Mode 1
        break;
      }
    case 2:
      {
        Mode2(); // Enter Mode 2
        break;
      }
    case 3:
      {
        Mode3(); // Enter Mode 3
        break;
      }
  }
}

void Mode1() {
  while (SelectIsPressed()) {} // Wait for the Select button to be released
  while (modcount == 1) {
    // Read the signals
    if (sampleIndex >= numSamples) {
      noInterrupts(); // Temporarily disable interrupts to safely access samples

      // Process the collected samples
      for (int i = 0; i < numSamples; i++) {
        // Convert ADC values to voltages (assuming 5V reference and centered around 2.5V)
        float voltage = ((samples1[i] * 5.0 / 1023.0) - 2.5) * 300; // Apply gain of 300
        float current = ((samples2[i] * 5.0 / 1023.0) - 2.5) * 100; // Apply gain of 100

        V_rms += voltage * voltage; // Accumulate voltage squared
        I_rms += current * current; // Accumulate current squared
        P_avg += current * voltage; // Accumulate power
      }

      // Final calculations
      V_rms = sqrt(V_rms / numSamples); // Calculate root mean square (RMS) voltage
      I_rms = sqrt(I_rms / numSamples); // Calculate RMS current
      P_avg = P_avg / numSamples; // Calculate average power
      PF = P_avg / (V_rms * I_rms); // Calculate power factor
      thetaO = acos(PF); // Calculate phase angle
      thetaN = acos(PFreq); // Calculate nominal phase angle
      Qc = P_avg * (tan(thetaO) - tan(thetaN)); // Calculate reactive power
      // Reset the sample index for the next capture
      sampleIndex = 0;
      interrupts(); // Re-enable interrupts
    }
    // Display measurements on LCD
    if (millis() - timer >= 2000) {
      lcd.setCursor(0, 0);
      lcd.print("V=");
      lcd.print(V_rms);
      lcd.print(",");
      lcd.print("I=");
      lcd.print(I_rms);
      lcd.setCursor(0, 1);
      lcd.print("P=");
      lcd.print(abs(P_avg));
      timer = millis();
    }
    delay(500);
    Changebutton(); // Check for button press to change mode
  }
}

void Mode2() {
  while (SelectIsPressed()) {} // Wait for the Select button to be released
  while (modcount == 2) {
    // Read the signals
    if (sampleIndex >= numSamples) {
      noInterrupts(); // Temporarily disable interrupts to safely access samples

      // Process the collected samples
      for (int i = 0; i < numSamples; i++) {
        // Convert ADC values to voltages (assuming 5V reference and centered around 2.5V)
        float voltage = ((samples1[i] * 5.0 / 1023.0) - 2.5) * 300; // Apply gain of 300
        float current = ((samples2[i] * 5.0 / 1023.0) - 2.5) * 100; // Apply gain of 100
        V_rms += voltage * voltage; // Accumulate voltage squared
        I_rms += current * current; // Accumulate current squared
        P_avg += current * voltage; // Accumulate power
      }

      // Final calculations
      V_rms = sqrt(V_rms / numSamples); // Calculate RMS voltage
      I_rms = sqrt(I_rms / numSamples); // Calculate RMS current
      P_avg = P_avg / numSamples; // Calculate average power
      PF = P_avg / (V_rms * I_rms); // Calculate power factor
      // Ensure power factor stays within valid range [-1, 1]
      if(PF >= 0.99) {
        PF = 1;
      } else if(PF <= -1) {
        PF = -1;
      }
      thetaO = acos(PF); // Calculate phase angle
      thetaN = acos(PFreq); // Calculate nominal phase angle
      Qc = P_avg * (tan(thetaO) - tan(thetaN)); // Calculate reactive power
      // Reset the sample index for the next capture
      sampleIndex = 0;
      interrupts(); // Re-enable interrupts
    }
    // Display measurements on LCD
    if (millis() - timer >= 2000) {
      lcd.setCursor(0, 0);
      lcd.print("PF: ");
      lcd.print(PF);
      lcd.setCursor(0, 1);
      lcd.print("PFreq: ");
      lcd.print(PFreq);
      timer = millis();
    }
    // Adjust power factor frequency based on user input
    if (RightIsPressed()) {
      PFreq += 0.05;
      if (RightIsPressed())
        delay(300);
      if (PFreq > 1)
        PFreq = 1;
    }
    if (LeftIsPressed()) {
      PFreq -= 0.05;
      if (LeftIsPressed())
        delay(300);
      if (PFreq < 0)
        PFreq = 0;
    }
    if (UpIsPressed()) {
      PFreq += 0.1;
      if (UpIsPressed())
        delay(300);
      if (PFreq > 1)
        PFreq = 1;
    }
    if (DownIsPressed()) {
      PFreq -= 0.1;
      if (DownIsPressed())
        delay(300);
      if (PFreq < 0)
        PFreq = 0;
    }
    delay(500);
    Changebutton(); // Check for button press to change mode
  }
}

void Mode3() {
  while (SelectIsPressed()) {} // Wait for the Select button to be released
  while (modcount == 3) {
    // Read the signals
    if (sampleIndex >= numSamples) {
      noInterrupts(); // Temporarily disable interrupts to safely access samples

      // Process the collected samples
      for (int i = 0; i < numSamples; i++) {
        // Convert ADC values to voltages (assuming 5V reference and centered around 2.5V)
        float voltage = ((samples1[i] * 5.0 / 1023.0) - 2.5) * 300; // Apply gain of 300
        float current = ((samples2[i] * 5.0 / 1023.0) - 2.5) * 100; // Apply gain of 100
        V_rms += voltage * voltage; // Accumulate voltage squared
        I_rms += current * current; // Accumulate current squared
        P_avg += current * voltage; // Accumulate power
      }
      // Final calculations
      V_rms = sqrt(V_rms / numSamples); // Calculate RMS voltage
      I_rms = sqrt(I_rms / numSamples); // Calculate RMS current
      P_avg = P_avg / numSamples; // Calculate average power
      PF = P_avg / (V_rms * I_rms); // Calculate power factor
      thetaO = acos(PF); // Calculate phase angle
      thetaN = acos(PFreq); // Calculate nominal phase angle
      Q = P_avg*tan(thetaO); // Calculate reactive power
      Qc = P_avg * (tan(thetaO) - tan(thetaN)); // Calculate reactive power compensation
      // Reset the sample index for the next capture
      sampleIndex = 0;
      interrupts(); // Re-enable interrupts
    }
    // Check if compensation is needed based on power factor
    if (PF >= 1 || PF > PFreq) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No compensation");
      lcd.setCursor(0, 1);
      lcd.print("needed");
    } else {
      // Perform compensation if needed
      if (millis() - timer >= 3000) {
        BEST(numbers, Qc); // Find best combination of capacitors
        Q = Qc + Q; // Update total reactive power
        timer = millis(); // Reset timer
      }
    }
    delay(500);
    Changebutton(); // Check for button press to change mode
  }
}

// Function to handle button presses and change mode accordingly
void Changebutton() {
  if (SelectIsPressed()) {
    modcount++;
  }
  if (modcount > 3)
    modcount = 1;
}

// Function to check if Select button is pressed
int SelectIsPressed() {
  button = analogRead(A0); // Read analog value from pin A0
  if (button > 700 && button < 990)
    return 1;
  else
    return 0;
}

// Function to check if Left button is pressed
int LeftIsPressed() {
  button = analogRead(A0); // Read analog value from pin A0
  if (button > 400 && button < 650)
    return 1;
  else
    return 0;
}

// Function to check if Down button is pressed
int DownIsPressed() {
  button = analogRead(A0); // Read analog value from pin A0
  if (button > 260 && button < 350)
    return 1;
  else
    return 0;
}

// Function to check if Up button is pressed
int UpIsPressed() {
  button = analogRead(A0); // Read analog value from pin A0
  if (button > 100 && button < 200)
    return 1;
  else
    return 0;
}

// Function to check if Right button is pressed
int RightIsPressed() {
  button = analogRead(A0); // Read analog value from pin A0
  if (button < 50)
    return 1;
  else
    return 0;
}

// Interrupt Service Routine to collect ADC samples
ISR(TIMER1_COMPA_vect) {
  if (sampleIndex < numSamples) {
    samples1[sampleIndex] = analogRead(A1); // Read and store sample from signal 1
    samples2[sampleIndex] = analogRead(A2); // Read and store sample from signal 2
    sampleIndex++;
  }
}

// Function to find the best combination of capacitors for reactive power compensation
void BEST(float arr[], float Qcom) {
  if (abs(Qerror - Qcom) > .06 * Qcom) {
    float TARGET_SUM = Qcom / 1000; // Target sum for reactive power compensation
    int totalCombinations = 1 << CAP_SIZE; // Total number of combinations (2^CAP_SIZE)

    // Iterate through all possible combinations
    for (int i = 1; i < totalCombinations; i++) {
      int currentSum = 0;
      int currentCount = 0;
      int currentCombination[CAP_SIZE] = { 0 };

      // Check each bit of the combination
      for (int j = 0; j < CAP_SIZE; j++) {
        if (i & (1 << j)) { // Only consider unused elements
          currentSum += arr[j]; // Accumulate sum
          currentCount++; // Increment count
          currentCombination[j] = 1; // Mark as used
        }
      }

      // Update the best combination if the current one is better
      if (currentSum >= TARGET_SUM) {
        if ((bestSum == 0) || (currentSum < bestSum) || (currentSum == bestSum && currentCount < bestCount)) {
          bestSum = currentSum;
          bestCount = currentCount;
          memcpy(bestCombination, currentCombination, sizeof(bestCombination));
        }
      }
    }

    bestSum *= 1000; // Scale the sum back to original units

    if (bestSum == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Steps aren't");
      lcd.setCursor(0, 1);
      lcd.print("enough");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Qc = ");
      lcd.print(Qcom);
      lcd.setCursor(0, 1);
      lcd.print("Qadd = ");
      lcd.print(bestSum);

      // Light up LEDs corresponding to selected capacitors
      for (int i = 0; i < CAP_SIZE; i++) {
        if (bestCombination[i] == 1) {
          if (i == 0) {
            digitalWrite(3, HIGH);
          } else if (i == 1) {
            digitalWrite(2, HIGH);
          } else if (i == 2) {
            digitalWrite(1, HIGH);
          } else if (i == 3) {
            digitalWrite(A3, HIGH);
          } else if (i == 4) {
            digitalWrite(A4, HIGH);
          } else if (i == 5) {
            digitalWrite(A5, HIGH);
          }
          arr[i] = 0;
        }
      }
    }
  }
  Qerror = Qcom; // Update previous Q error for comparison
  bestSum = 0; // Reset best sum
}
