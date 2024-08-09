


int analogSkin; // variable to store analog reading from skin sensor
float voltageSkin; // variable to store voltage from skin sensor
float thresholdSkin; // variable to store threshold for skin sensor
float thresholdHeart; // variable to store threshold for heart sensor
float sumSkin = 0; // variable to accumulate sum of skin sensor readings
float sumHeart = 0; // variable to accumulate sum of heart sensor readings
int countBPM = 0; // variable to count heartbeats - we count hearbeats in 15 seconds and multiply by 4 
int count = 0; // variable to count stressed states - if greater than a threshold then its considered a stressful situation. we use a counter to ensure that we don't consider noise as a stressful response
int analogHeart; // variable to store analog reading from heart sensor
float voltageHeart; // variable to store voltage from heart sensor
int BPM; // variable to store calculated BPM
int BPMThreshold; // variable to store BPM threshold

//Please install the following libraries: Adafruit BusIO, Adafruit GFX Library, Adafruit SSD1306, SparkFun MAX3010x Pulse and Proximity

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);

// Initialize particle sensor
MAX30105 particleSensor;

void setup() {
  Serial.begin(115200); // start serial communication
  delay(10000); // wait for 10 seconds to allow for user to prepare themselves
  Serial.println("Wait to get started");
  

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Initialize particle sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  particleSensor.setup(); // Setup particle sensor
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display Welcome text
  display.println("Welcome!"); 
  display.println("Please wait for your");
  display.println("threshold"); 
  display.println("calculations.");
  display.display();
  delay (1000);
  display.clearDisplay();



  // Calibration phase to calculate thresholds for skin and heart sensors
  for (int i = 0; i < 1000; i++) {
    analogSkin = analogRead(A0); // read analog value from skin sensor
    analogHeart = analogRead(A1); // read analog value from heart sensor
    voltageSkin = analogSkin * (5.0 / 1023.0); // convert analog reading to voltage for skin sensor
    voltageHeart = analogHeart * (5.0 / 1023.0); // convert analog reading to voltage for heart sensor
    sumSkin += voltageSkin; // accumulate sum of skin sensor readings
    sumHeart += voltageHeart; // accumulate sum of heart sensor readings



    delay(5); // delay for stability
  }
  thresholdSkin = (sumSkin / 1000); // calculate threshold for skin sensor
  thresholdHeart = sumHeart / 1000; // calculate threshold for heart sensor

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display theshold values 
  display.print("threshold Skin = ");
  display.println(thresholdSkin);
  display.print("threshold Heart = ");
  display.println(thresholdHeart);
  display.display();
  delay (5000);
  display.clearDisplay();

  
  // Wait for user to get ready for BPM measurements
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display text about starting BPM measures
  display.println("Wait to get BPM ");
  display.println("measurements.");
  display.display();
  delay(5000);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Collecting");
  display.println("measurements...");
  display.display();
  
  
  // Measure BPM for 15 seconds
  unsigned long startTime = millis();
  countBPM = 0;
  while (millis() - startTime < 15000) {

    analogHeart = analogRead(A1); // read analog value from heart sensor
    voltageHeart = analogHeart * (5.0 / 1023.0); // convert analog reading to voltage for heart sensor
    if (voltageHeart > (thresholdHeart + 0.10)) { // A spike in voltage reading indicates the flow of hemoglobin cells which changes the light reflection - we use this to indicate a heartbeat we add 0.10 to this value account for error.  This error was found through experimentation
      countBPM = countBPM + 1; // increment heartbeat count
      digitalWrite(10, HIGH); // turn on LED to visually indicate hearbeat
      delay(500); // wait - ensure we can get more stable readings 
    }
    digitalWrite(10, LOW); // turn off LED
  }
  BPMThreshold = countBPM * 4; // calculate BPM threshold
  message (BPMThreshold, thresholdSkin, -999, "Threshold");
  BPM = BPMThreshold; // set BPM value
  
  // Prompt user if BPM is too low and recalibrate
  while (BPMThreshold < 60) {

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    //If there are errors in BPM values (below 60), inform user
    display.println("Value is too low.");
    display.println("Please reposition");
    display.println("your finger and ");
    display.println("try again.");
    display.display();

    startTime = millis();
    while (millis() - startTime < 5000) {
      analogHeart = analogRead(A1); // read analog value from heart sensor
      voltageHeart = analogHeart * (5.0 / 1023.0); // convert analog reading to voltage for heart sensor
      if (voltageHeart > (thresholdHeart + 0.10)) {
        digitalWrite(10, HIGH); // turn on LED
        delay(500); // wait
      }
      digitalWrite(10, LOW); // turn off LED
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Inform user that in 5 seconds new readings will be gathered 
    display.println("Wait 5 seconds");
    display.println("before attempting to");
    display.println("recollecting values.");
    display.display();
    delay(5000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Inform users about recollecting results
    display.println("Recollecting results");
    display.println("BPM Measures");
    display.display();
    startTime = millis();
    countBPM = 0;
    while (millis() - startTime < 15000) {
      analogHeart = analogRead(A1); // read analog value from heart sensor
      voltageHeart = analogHeart * (5.0 / 1023.0); // convert analog reading to voltage for heart sensor
      if (voltageHeart > (thresholdHeart + 0.10)) {
        countBPM = countBPM + 1; // increment heartbeat count - this is the number of hearbeats in 15 seconds 
        digitalWrite(10, HIGH); // turn on LED
        delay(500); // wait
      }
      digitalWrite(10, LOW); // turn off LED
    }
    BPMThreshold = countBPM * 4; // calculate BPM threshold

    message (BPMThreshold, thresholdSkin,-999,"Threshold" ); //Call to method made by Connor Mark 

  }
}

void loop() {
  delay(2000);
  display.clearDisplay();
  // Main loop to measure skin sensor and heart sensor values
  unsigned long startTime = millis();
  countBPM = 0;

  // Read SPO2 and heart rate
  uint32_t irBuffer[100]; //infrared LED sensor data
  uint32_t redBuffer[100]; //red LED sensor data
  int32_t bufferLength = 100; // data length
  int32_t spo2; // SPO2 value
  int8_t validSPO2; // indicator for valid SPO2
  int32_t heartRate; // heart rate value
  int8_t validHeartRate; // indicator for valid heart rate

  byte i =0;

  while (millis() - startTime < 15000) {
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate); //Code necessary for oxygen saturation - note that oxygen saturation is not used for classifiying stress but rather as an additional measure which was asked by users we tested with so they can determine the impact of stress in their oxygenation

     while (!particleSensor.available()) {
      particleSensor.check(); // Check the sensor for new data
    }
  
    analogSkin = analogRead(A0); // read analog value from skin sensor
    analogHeart = analogRead(A1); // read analog value from heart sensor
    voltageSkin = analogSkin * (5.0 / 1023.0); // convert analog reading to voltage for skin sensor
    voltageHeart = analogHeart * (5.0 / 1023.0); // convert analog reading to voltage for heart sensor

    //Code necessary for proper gathering of spo2 data
    if (i<bufferLength){
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); // Move to the next sample
    irBuffer[i] = particleSensor.getIR();
    i++;
    }
    if ((millis() - startTime < 15000) && (i == bufferLength)){
      i =0; //i is reset to zero 
    }

    
    if (voltageHeart > (thresholdHeart + 0.10)) {
      countBPM = countBPM + 1; // increment heartbeat count
      digitalWrite(10, HIGH); // turn on LED
      delay(500); // wait
      digitalWrite(10, LOW); // turn off LED
    }
    if ((voltageSkin > (thresholdSkin + 0.05) && (BPM > BPMThreshold))) { //iff voltage of the skin is above the threshold this indicates change in conductance of the body due to sweat which is considered a stress response. We add 0.05 to the threshold to acocunt for error. This error was found through experimentation
      count = count + 1; // increment stress count
      if (count >= 10) {
        message(BPM,voltageSkin,spo2, "Stressed");
        digitalWrite(11, HIGH); // turn on LED - RED LED indicates stress response
        digitalWrite(12, LOW); // turn off LED
        digitalWrite(13, LOW); // turn off LED
        count = 0; // reset count
      }
    } else if (((voltageSkin > (thresholdSkin + 0.05) && (BPM <= BPMThreshold)) || ((voltageSkin <= (thresholdSkin + 0.05) && BPM > BPMThreshold)))) {
      count = 0; // reset count
      message(BPM,voltageSkin, spo2, "Semi-stressed");
      digitalWrite(11, LOW); // turn off LED
      digitalWrite(12, HIGH); // turn on LED - YELLOW LED indicates possibility of entering stress response
      digitalWrite(13, LOW); // turn off LED
    } else {
      count = 0; // reset count
      message(BPM,voltageSkin,spo2, "Calm");
      digitalWrite(11, LOW); // turn off LED
      digitalWrite(12, LOW); // turn off LED
      digitalWrite(13, HIGH); // turn on LED - GREEN LED indicates calm state
    }
    
  }
  BPM = countBPM * 4; // calculate BPM
   delay(10); // wait
   


  
}



void message (int heart, double skin, int spo2, String status){
  display.clearDisplay(); //Clearing display
  display.setTextSize(1); //Setting size of text
  display.setTextColor(WHITE); //Setting colour
  display.setCursor(0, 10); //Setting position
  // Setting the information displayed to user 
  display.print("BPM: "); 
  display.println(heart);
  display.print("/");
  display.println(BPMThreshold);
  display.print ("Skin Voltage: ");
  display.print(skin);
  display.print("/");
  display.println(thresholdSkin);
  display.print("SPO2: ");
  display.println(spo2);
  display.print("Status: "); 
  display.println(status);
  display.display();
}
