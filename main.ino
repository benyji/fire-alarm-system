
/*
$$$$$$$\  $$$$$$$$\ $$\   $$\    $$$$$\  $$$$$$\  $$\      $$\ $$$$$$\ $$\   $$\ 
$$  __$$\ $$  _____|$$$\  $$ |   \__$$ |$$  __$$\ $$$\    $$$ |\_$$  _|$$$\  $$ |
$$ |  $$ |$$ |      $$$$\ $$ |      $$ |$$ /  $$ |$$$$\  $$$$ |  $$ |  $$$$\ $$ |
$$$$$$$\ |$$$$$\    $$ $$\$$ |      $$ |$$$$$$$$ |$$\$$\$$ $$ |  $$ |  $$ $$\$$ |
$$  __$$\ $$  __|   $$ \$$$$ |$$\   $$ |$$  __$$ |$$ \$$$  $$ |  $$ |  $$ \$$$$ |
$$ |  $$ |$$ |      $$ |\$$$ |$$ |  $$ |$$ |  $$ |$$ |\$  /$$ |  $$ |  $$ |\$$$ |
$$$$$$$  |$$$$$$$$\ $$ | \$$ |\$$$$$$  |$$ |  $$ |$$ | \_/ $$ |$$$$$$\ $$ | \$$ |
\_______/ \________|\__|  \__| \______/ \__|  \__|\__|     \__|\______|\__|  \__|
    
                                   MAIN
                  Project: Arduino Based Fire Alarm System
                  Last edited: 01/29/2021

                  Benjamin 
 */

/*
---------------------------------------------------------------------------------
                Section Start: Initialization
*/
 
// Referenced Libraries
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// Definitions for NeoPixel (Lighting System)
#define JEWEL1 13
#define JEWEL2 7
#define JEWEL3 A0

#define NUMPIXELS 7

// Initialize the library in reference to pins connected to Arduino
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, JEWEL1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, JEWEL2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, JEWEL3, NEO_GRB + NEO_KHZ800);


// LEDs Input On Arduino Board
int LED_GREEN = 10;
int LED_RED = 9;
int LED_ORANGE = 8;

// Buttons Inputs On Arduino Board
int BUTTON_ONE = A5;
int BUTTON_TWO = A4;

// Neopixel Configuration
int jewel_delayval = 1; // Timing delay in milliseconds


// Gas Sensor Input & Configuration
int GAS_SENSOR = A2;
int GAS_SENSOR_STATUS = 0;
int GAS_SENSOR_SENSITIVITY = 750;

// Temperature Sensor Input & Configuration
int TEMPERATURE_SENSOR = A1;
float temperature = 0;
float celsius = 0;
float voltage = 0;

// Piezo Buzzer Input
int PANEL_ALARM = 6;

// Statuses & Debounces used in code
bool SYSTEM_STATUS; // false: disarmed, true: armed.
bool SILENT_MODE = false;

int BUTTON_ONE_STATUS = 0;
int BUTTON_TWO_STATUS = 0;

// Debounces are used for temporary storage of values
bool BUTTON_DEBOUNCE = false;
bool BUTTON_TWO_DEBOUNCE = false;
bool ACTIVATION_DEBOUNCE = false;

int count = 4; // Used in For loops

// Default settings for Fire Alarm System upon first start & reboot
void setup() {
  // First action - Turn on LCD and Display Initialization Text
  lcd.begin(16, 2);
  lcd.print("System Booting..");

  //Initialization
  loadingScreen(); // All inputs and configurations occur in this function, helps keep code organized

 // Serial.begin(9600); // Acted as a debuging console, no longer needed

  lightDeactivation(); // Incase user pressed the reset switch whilst an Alarm Activation was in progress

}

/*
---------------------------------------------------------------------------------
                Section Start: Looped Functions, Listeners, Events
*/

void loop() {

  // If System is Armed, activate listeners (Smoke detector & Temperature Sensor)
  if (SYSTEM_STATUS == true) {

    lcd.setCursor(0, 0);
    lcd.print("System Armed    "); // Display Armed Screen
    lcd.setCursor(0, 1);
    if (SILENT_MODE != true) {
      lcd.print("                "); // Clear the LCD (lcd.clear is too buggy, rather use my own)
    } else {
      lcd.print("Silent Mode: ON"); // Display User Silent Mode Selection Screen
    }

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH); // RED led represents System Armed
/*
    GAS SENSOR/DETECTOR LISTENER - Responsible for handling all tasks when smoke/gas is detected by the module
---------------------------------------------------------------------------------------------------------------
*/
    GAS_SENSOR_STATUS = analogRead(GAS_SENSOR); // Consistently checks the Smoke Detectors Status 
    if (GAS_SENSOR_STATUS > GAS_SENSOR_SENSITIVITY) { // If the gas (smoke) concentration passes the threshold, run code below
      digitalWrite(LED_ORANGE, HIGH); // Orange LED represents a Event has been activated
      digitalWrite(LED_RED, LOW);
      if (ACTIVATION_DEBOUNCE != true) { // If this is the first Event ran, wait 1 second and display the amber alert LED
        delay(1000);
        ACTIVATION_DEBOUNCE = true;
      }

      digitalWrite(LED_ORANGE, LOW);
      digitalWrite(LED_RED, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("ALARM ACTIVATION"); // Display notice of Alarm Activation
      lcd.setCursor(0, 1);
      lcd.print("SMOKE DETECTED "); // Display type of detection
      
      if (SILENT_MODE != true) { // If User Selection is NOT set for silent alarm, run Siren & NeoPixels
        alarmActivation();
      } else {
        alarmActivation_without_siren(); // If User Selection IS set for silent alarm, run ONLY NeoPixels
      }
    }
/*
---------------------------------------------------------------------------------------------------------------
    TEMPERATURE SENSOR/DETECTOR LISTENER - Responsible for handling all tasks when temperature exceeds threshold

*/
    temperature = analogRead(TEMPERATURE_SENSOR); // Consistently checks the temperature around the module for hazards
    voltage = temperature * 5000 / 1024; // Voltage in relation to temperature
    voltage = voltage - 500; // Voltage calculation
    celsius = voltage / 10; // Celsius conversion for easy debug
    if (celsius >= 100) // Threshold
    {
      // Below code was previously explained in previous event
      digitalWrite(LED_ORANGE, HIGH);
      digitalWrite(LED_RED, LOW);
      
      if (ACTIVATION_DEBOUNCE != true) {
        delay(1000);
        ACTIVATION_DEBOUNCE = true;
      }

      digitalWrite(LED_ORANGE, LOW);
      digitalWrite(LED_RED, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("ALARM ACTIVATION"); 
      lcd.setCursor(0, 1);
      lcd.print("HEAT DETECTED  "); // Extra spaces to cancel out extra letters
      
      if (SILENT_MODE != true) {
        alarmActivation();
      } else {
        alarmActivation_without_siren();
      }
    }
/*
---------------------------------------------------------------------------------------------------------------
*/

    // If System is Disarmed, Notify user and await input
  } else {

    lcd.setCursor(0, 0);
    lcd.print("System Disarmed ");
    lcd.setCursor(0, 1);
    lcd.print("                "); 

    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    
    ACTIVATION_DEBOUNCE = false; // Cancel out outstanding debounces from when system was armed 

  }

  /*
---------------------------------------------------------------------------------
                Section Start: Buttons & User input
*/

  // Button (1), Responsible for Arming and Disarming the System
  BUTTON_ONE_STATUS = digitalRead(BUTTON_ONE); // Check for inputs

  if (BUTTON_ONE_STATUS == HIGH) { // If button is being held down right now
    if (BUTTON_DEBOUNCE != true) { // If the button was not recently held down (to prevent unintentional double clicks)
      BUTTON_DEBOUNCE = true;
      if (SYSTEM_STATUS == true) {  // If system is currently armed, disarm it
        SYSTEM_STATUS = false;
      } else {
        SYSTEM_STATUS = true; // Viceversa
      }
    }
    
  } else { // When the User unholds the button, allow for future presses
    BUTTON_DEBOUNCE = false;
  }

  // Button (2), Responsible for Activating/Deactivating Silent Mode
  BUTTON_TWO_STATUS = digitalRead(BUTTON_TWO);

  if (BUTTON_TWO_STATUS == HIGH) {
    if (SYSTEM_STATUS == true) {
      if (BUTTON_TWO_DEBOUNCE != true) {
        BUTTON_TWO_DEBOUNCE = true;
        if (SILENT_MODE != true) {
          SILENT_MODE = true;
        } else {
          SILENT_MODE = false;
        }
      }
    }
  } else {
    BUTTON_TWO_DEBOUNCE = false;
  }

  delay(10); // Delay at the end of the loop to prevent system overload

}

  /*
---------------------------------------------------------------------------------
                Section Start: Custom Functions
*/

// Alarm Lighting and Siren Handler when Silent Mode is FALSE
void alarmActivation() {
// I found a For loop to be too buggy and was better off doing the loop manually

  // The Alarm Pattern is as follows:
  tone(PANEL_ALARM, 2000);  // Activate Siren at 2000hz
  lightDeactivation(); // Disable the lighting
  delay(1000); // Delay of 1 second until Siren turned off
  noTone(PANEL_ALARM); // Disable Siren
  lightActivation(); // Activate Lighting
  delay(500); // Lighting activated for half a second
  tone(PANEL_ALARM, 2000); // Repeat
  lightDeactivation();
  delay(1000);
  noTone(PANEL_ALARM);
  lightActivation();
  delay(500);
  tone(PANEL_ALARM, 2000);
  lightDeactivation();
  delay(1000);
  noTone(PANEL_ALARM);
  // Run Lighting animation once without Siren (Prevents siren from being too annoying)
  lightActivation();
  delay (500);
  lightDeactivation();
  delay (1000);
  lightActivation();
  delay (500);
  lightDeactivation();
  delay (1000);
}

// Alarm Lighting and Siren Handler when Silent Mode is TRUE
void alarmActivation_without_siren() {
  // Same as previous handler, however no siren
  lightDeactivation();
  delay(1000);
  lightActivation();
  delay(500);
  lightDeactivation();
  delay(1000);
  lightActivation();
  delay(500);
  lightDeactivation();
  delay(1000);
  lightActivation();
  delay (500);
  lightDeactivation();
  delay (1000);
  lightActivation();
  delay (500);
  lightDeactivation();
  delay (1000);
}

// Function responsible for DISABLING lighting (Setting each LED off)
void lightDeactivation() {
  pixels1.fill((0, 0, 0)); 
  pixels2.fill((0, 0, 0));
  pixels3.fill((0, 0, 0));

  pixels1.show();
  pixels2.show();
  pixels3.show();
}

// Function responsible for ENABLING lighting (Lighting Animation Handler)
void lightActivation() {
  for (int i = 0; i < NUMPIXELS; i++) { // Run for each individual LED in each individual NeoPixel Jewel
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels1.setPixelColor(i, pixels1.Color(255, 0, 0)); //pixels1 represents the first NeoPixel Jewel, pixels2 represents the second, etc
    pixels2.setPixelColor(i, pixels2.Color(255, 0, 0));
    pixels3.setPixelColor(i, pixels3.Color(255, 0, 0));

    // This sends the updated pixel color to the hardware.
    pixels1.show();
    pixels2.show();
    pixels3.show();

    // Delay for a period of time (in milliseconds).
    delay(jewel_delayval);
  }

}

//Loading Screen Animation Handler (Not necessary however it is more tidy then putting all in 'void setup()'. 
void loadingScreen() {

  delay(2000);
  //LCD
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Initializing...");

  //LED SETUP
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);

  //LED TEST
  digitalWrite(LED_ORANGE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);

  delay(5000);
  //LED DISABLE
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_ORANGE, LOW);

  delay(500);
  //SHOWCASE ORANGE LED
  digitalWrite(LED_ORANGE, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("LIGHT MODULES OK");

  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("SMOKE MODULES OK");

  delay(1000);
  //BUTTONS SETUP
  pinMode(BUTTON_ONE, INPUT);
  lcd.setCursor(0, 1);
  lcd.print("INPUT MODULES OK");

  delay(1000);
  // Piezo Buzzer does not require setup 
  lcd.setCursor(0, 1);
  lcd.print("SOUND MODULES OK");
  
  delay(1000);
  pixels1.begin();
  pixels2.begin();
  pixels3.begin();
  lcd.setCursor(0, 1);
  lcd.print("PIXEL MODULES OK");
  
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.clear();
  
  delay(2000);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_ORANGE, LOW);

  SYSTEM_STATUS = false;
}

/* 
 end of code! Thank you for taking an interest in my project. 
 Signed: Benjamin
 Date: 01/29/2021
 */
