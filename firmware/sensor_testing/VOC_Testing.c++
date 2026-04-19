#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define i2c_Address 0x3C

// Initialize VOC sensor pin
const int VOC_SENSOR_PIN = 0;  // Analog pin for VOC sensor

// Initialize display object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Update the risk thresholds
#define R0 10000  // Sensor resistance in clean air

// Update the threshold constants to be explicitly for acetone
const float ACETONE_COPD_LOW = 0.24;    // Acetone lower threshold in ppm for COPD
const float ACETONE_COPD_HIGH = 0.76;   // Acetone upper threshold in ppm for COPD
const float ACETONE_ASTHMA = 2335;      // Acetone threshold in ppb for asthma (reverted back)
const float ACETONE_CANCER_LOW = 0.4;   // Acetone lower threshold in ppm for cancer
const float ACETONE_CANCER_HIGH = 0.7;  // Acetone upper threshold in ppm for cancer

// Add these global variables at the top with other constants
float baselineReading = 0;
const float STANDARD_PPB = 550.0;  // Our desired baseline reading

// Function declarations
void displayAcetoneRisk();
void printVOCLevels();
String getVOCRisk(float value, float threshold);
String getCancerRisk(float voc_ppm);
String getCOPDRisk(float voc_ppm);
void calibrateSensor();

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize display
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
  
  // Add calibration
  calibrateSensor();
}

void loop() {
  displayAcetoneRisk();  // Display acetone risk on OLED
  printVOCLevels();      // Print VOC levels to Serial monitor
  delay(2000);           // Wait 2 seconds between readings
}

// Function to display risk level based on acetone
void displayAcetoneRisk() {
  int analogValue = analogRead(VOC_SENSOR_PIN);
  float voltage = analogValue * (5.0 / 1023.0);
  
  // Calculate sensor resistance (Rs)
  float Rs = (5.0 - voltage) * R0 / voltage;
  float ratio = Rs / R0;
  
  // Convert to acetone concentrations
  float acetonePPB = 1000.0 / (ratio - 0.2);
  float acetonePPM = acetonePPB / 1000.0;

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.print("Acetone: ");
  display.print(acetonePPB, 1);
  display.println(" ppb");
  
  display.println("\nRisk Levels:");
  
  display.print("COPD: ");
  display.println(getCOPDRisk(acetonePPM));
  
  display.print("Asthma: ");
  display.println(getAsthmaRisk(acetonePPB));
  
  display.print("Cancer: ");
  display.println(getCancerRisk(acetonePPM));
  
  display.display();
}

// Function to print VOC levels to Serial monitor
void printVOCLevels() {
  int vocRawValue = analogRead(VOC_SENSOR_PIN);
  float voltage = vocRawValue * (5.0 / 1023.0);
  
  // Calculate acetone first using our calibrated method
  float Rs = (5.0 - voltage) * R0 / voltage;
  float ratio = Rs / R0;
  float acetonePPB = 1000.0 / (ratio - 0.2);
  float acetonePPM = acetonePPB / 1000.0;
  
  // Estimate other VOCs (these are rough estimates for demonstration)
  float formaldehydePPM = acetonePPM * 0.5;  // Example ratio
  float toluenePPM = acetonePPM * 1.5;       // Example ratio
  
  Serial.println("\n=== VOC Levels ===");
  Serial.print("Acetone: ");
  Serial.print(acetonePPM, 2);
  Serial.println(" PPM");
  Serial.print("Formaldehyde: ");
  Serial.print(formaldehydePPM, 2);
  Serial.println(" PPM (estimated)");
  Serial.print("Toluene: ");
  Serial.print(toluenePPM, 2);
  Serial.println(" PPM (estimated)");
  Serial.println("================");
}

// Add these helper functions from sensors.c++
String getVOCRisk(float value, float threshold) {
  return (value >= threshold) ? "High" : "Low";
}

String getCancerRisk(float acetonePPM) {
  if (acetonePPM >= ACETONE_CANCER_LOW && acetonePPM <= ACETONE_CANCER_HIGH) {
    return "High";
  }
  return "Low";
}

// Update the getCOPDRisk function to handle range checks
String getCOPDRisk(float acetonePPM) {
  if (acetonePPM >= ACETONE_COPD_LOW && acetonePPM <= ACETONE_COPD_HIGH) {
    return "High";
  }
  return "Low";
}

// Update the risk assessment functions to be acetone-specific
String getAsthmaRisk(float acetonePPB) {
  return (acetonePPB >= ACETONE_ASTHMA) ? "High" : "Low";
}

// Add this calibration function
void calibrateSensor() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println("Calibrating...");
  display.display();
  
  // Take average of multiple readings for stability
  float sum = 0;
  const int numReadings = 10;
  
  for(int i = 0; i < numReadings; i++) {
    int analogValue = analogRead(VOC_SENSOR_PIN);
    float voltage = analogValue * (5.0 / 1023.0);
    float Rs = (5.0 - voltage) * R0 / voltage;
    float ratio = Rs / R0;
    float ppb = 1000.0 / (ratio - 0.2);
    sum += ppb;
    delay(100);
  }
  
  float initialReading = sum / numReadings;
  baselineReading = initialReading - STANDARD_PPB;  // Calculate offset
  
  display.clearDisplay();
  display.println("Calibration done!");
  display.println();
  display.print("Offset: ");
  display.print(baselineReading, 0);
  display.println(" ppb");
  display.display();
  delay(2000);
}


