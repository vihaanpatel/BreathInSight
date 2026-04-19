// Pin definition
#define SENSOR_PIN A1
#define SAMPLES_COUNT 10  // Number of samples to average

// Variables for peak detection
float baselineFlow = 0;
float peakFlow = 0;
float lastFlow = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("Omron D6F-V03A1 Airflow Sensor Test");
  Serial.println("-----------------------------------");
  
  // Establish baseline reading
  float sum = 0;
  for(int i = 0; i < 10; i++) {
    sum += getSmoothedReading();
    delay(100);
  }
  baselineFlow = sum / 10;
  Serial.print("Baseline established at: ");
  Serial.println(baselineFlow, 3);
}

void loop() {
  // Get smoothed reading
  float airflow = getSmoothedReading();
  float voltage = (airflow / (3.0/4.0)) + 1.0;  // Convert back to voltage for display
  
  // Update peak if new reading is higher
  if(airflow > peakFlow) {
    peakFlow = airflow;
  }
  
  // Detect significant changes (>1% from baseline)
  float changeFromBaseline = ((airflow - baselineFlow) / baselineFlow) * 100.0;
  String flowStatus = "STABLE";
  if(abs(changeFromBaseline) > 1.0) {
    if(airflow > lastFlow) {
      flowStatus = "INCREASING";
    } else {
      flowStatus = "DECREASING";
    }
  }
  
  // Print detailed debug information
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V | Flow: ");
  Serial.print(airflow, 3);
  Serial.print(" m/s | Change: ");
  Serial.print(changeFromBaseline, 1);
  Serial.print("% | Status: ");
  Serial.print(flowStatus);
  Serial.print(" | Peak: ");
  Serial.println(peakFlow, 3);
  
  // Reset peak after 3 seconds of stable readings
  if(flowStatus == "STABLE") {
    static int stableCount = 0;
    stableCount++;
    if(stableCount > 3) {
      peakFlow = airflow;
      stableCount = 0;
    }
  }
  
  lastFlow = airflow;
  delay(100);  // Faster sampling rate
}

float getSmoothedReading() {
  float sum = 0;
  // Take multiple samples
  for(int i = 0; i < SAMPLES_COUNT; i++) {
    int rawValue = analogRead(SENSOR_PIN);
    float voltage = rawValue * (5.0 / 1023.0);
    float measurement = (voltage - 1.0) * (3.0 / 4.0);
    sum += measurement;
    delay(10);
  }
  return sum / SAMPLES_COUNT;
}
