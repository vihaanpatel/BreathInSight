#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define i2c_Address 0x3C

#define VOC_SENSOR_PIN A0
#define R0 10000  // Sensor resistance in clean air

// Risk thresholds
float COPD_VOC_THRESHOLD = 0.50;
float ASTHMA_VOC_THRESHOLD = 2335;
float LUNG_CANCER_VOC_LOW = 0.4;
float LUNG_CANCER_VOC_HIGH = 0.7;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  Serial.println("Starting VOC Monitor...");
  delay(250);
  
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
}

void loop() {
  // Read VOC values
  float voc_ppb = readVOC();
  float voc_ppm = voc_ppb / 1000.0;
  
  // Update display
  updateDisplay(voc_ppb, voc_ppm);
  
  // Debug output
  printDebug(voc_ppb, voc_ppm);
  
  delay(1000);
}

float readVOC() {
  int analogValue = analogRead(VOC_SENSOR_PIN);
  float voltage = analogValue * (5.0 / 1023.0);
  
  // Calculate sensor resistance (Rs)
  float Rs = (5.0 - voltage) * R0 / voltage;
  float ratio = Rs / R0;
  
  // Convert to ppb
  float ppb = 1000.0 / (ratio - 0.2);
  return ppb;
}

void updateDisplay(float voc_ppb, float voc_ppm) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  // VOC Readings
  display.setCursor(0,0);
  display.print("VOC: ");
  display.print(voc_ppb, 1);
  display.println(" ppb");
  
  // Risk Levels
  display.println("\nRisk Levels:");
  
  display.print("COPD: ");
  display.println(getVOCRisk(voc_ppm, COPD_VOC_THRESHOLD));
  
  display.print("Asthma: ");
  display.println(getVOCRisk(voc_ppb, ASTHMA_VOC_THRESHOLD));
  
  display.print("Cancer: ");
  display.println(getCancerRisk(voc_ppm));
  
  display.display();
}

String getVOCRisk(float value, float threshold) {
  return (value >= threshold) ? "High" : "Low";
}

String getCancerRisk(float voc_ppm) {
  if (voc_ppm >= LUNG_CANCER_VOC_LOW && voc_ppm <= LUNG_CANCER_VOC_HIGH) {
    return "High";
  }
  return "Low";
}

void printDebug(float voc_ppb, float voc_ppm) {
  Serial.println("\n--- Debug Data ---");
  Serial.print("VOC (ppb): "); Serial.println(voc_ppb);
  Serial.print("VOC (ppm): "); Serial.println(voc_ppm);
  Serial.println("Risk Levels:");
  Serial.print("COPD: "); Serial.println(getVOCRisk(voc_ppm, COPD_VOC_THRESHOLD));
  Serial.print("Asthma: "); Serial.println(getVOCRisk(voc_ppb, ASTHMA_VOC_THRESHOLD));
  Serial.print("Cancer: "); Serial.println(getCancerRisk(voc_ppm));
  Serial.println("---------------");
}
