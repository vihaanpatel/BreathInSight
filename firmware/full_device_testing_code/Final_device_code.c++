#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <SD.h>
#include <SPI.h>
#include <Encoder.h>

// OLED display settings
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define i2c_Address 0x3C

// Pin definitions
#define VOC_PIN A0
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4
#define SD_CS 10
#define AIRFLOW_SENSOR_PIN A1
#define SAMPLES_COUNT 10

// Replace vector include with array size definition
#define MAX_COMBS 50  // Reduced maximum combinations

// Add these near the top with other #define statements
#define CM_TO_INCHES 0.393701  // Conversion factor from cm to inches

// Forward declare all structs
struct ThresholdLevels {
    float predicted;
    float threshold80;
    float threshold70;
    float threshold60;
    float threshold50;
    float threshold35;
};

struct GlobalVars {
    int height = 60;
    int age = 30;
    bool isMale = true;
    bool testing = false;
    float maxFlow = 0;
    float maxVOC = 0;
    ThresholdLevels thresholds;
};

struct Combination {
    int age;
    bool male;
    int height;
};

// Initialize display object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// VOC sensor constants
#define R0 10000  // Sensor resistance in clean air

// VOC thresholds (acetone)
const float ACETONE_COPD_LOW = 0.24;    
const float ACETONE_COPD_HIGH = 0.76;   
const float ACETONE_ASTHMA = 2335;      
const float ACETONE_CANCER_LOW = 0.4;   
const float ACETONE_CANCER_HIGH = 0.7;  

// Temporary airflow thresholds (placeholder values)
const float AIRFLOW_COPD = 2.5;     // m/s
const float AIRFLOW_ASTHMA = 3.0;   // m/s
const float AIRFLOW_CANCER = 2.0;   // m/s

// Calibration variables
float baselineReading = 0;
const float STANDARD_PPB = 550.0;
float baselineFlow = 0;

// Menu States
enum MenuState {M_HEIGHT, M_GENDER, M_AGE, M_TEST, M_RESULTS} currentMenu = M_HEIGHT;

// Global Variables
Encoder encoder(ENC_CLK, ENC_DT);
GlobalVars g;
Combination combs[MAX_COMBS];
uint8_t numCombs = 0;

void setup() {
    // Initialize Serial for debugging
    Serial.begin(9600);
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize OLED with proper delay
    display.begin(i2c_Address, true);
    delay(250);  // Give display time to initialize
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("Initializing...");
    display.display();
    delay(1000);
    
    calibrateSensors();
    
    // Initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card Init Failed!");
        displayError("SD Card Init Failed!");
        while (1);
    }
    Serial.println("SD card initialized successfully");
    
    // Initialize encoder button
    pinMode(ENC_SW, INPUT_PULLUP);
    
    // Initialize VOC sensor
    pinMode(VOC_PIN, INPUT);
    
    Serial.println("Setting up parameter files...");
    setupParameterFiles();
    Serial.println("Parameter files setup complete");
    
    // Show welcome screen
    Serial.println("Showing welcome screen");
    displayWelcome();
    Serial.println("Setup complete!");
}

void loop() {
    static long oldPos = -999;
    long newPos = encoder.read();
    
    if (newPos != oldPos) {
        int diff = (newPos - oldPos)/4;
        switch(currentMenu) {
            case M_HEIGHT: g.height = constrain(g.height + diff, 48, 84); break;
            case M_GENDER: g.isMale = !g.isMale; break;
            case M_AGE: g.age = constrain(g.age + diff, 5, 100); break;
        }
        oldPos = newPos;
    }
    
    if (!digitalRead(ENC_SW)) {
        delay(50);
        if (!digitalRead(ENC_SW)) {
            handleButton();
            while(!digitalRead(ENC_SW));
        }
    }
    
    if (g.testing) {
        float flow = getFlow();
        float voc = analogRead(VOC_PIN) * (5.0 / 1023.0);
        if (flow > g.maxFlow) g.maxFlow = flow;
        if (voc > g.maxVOC) g.maxVOC = voc;
    }
    
    updateDisplay();
    delay(50);
}

void calibrateSensors() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("Calibrating...");
    display.display();
    
    // Calibrate VOC sensor
    float vocSum = 0;
    // Calibrate airflow sensor
    float airflowSum = 0;
    const int numReadings = 10;
    
    for(int i = 0; i < numReadings; i++) {
        // VOC calibration
        int vocValue = analogRead(VOC_PIN);
        float vocVoltage = vocValue * (5.0 / 1023.0);
        float Rs = (5.0 - vocVoltage) * R0 / vocVoltage;
        float ratio = Rs / R0;
        float ppb = 1000.0 / (ratio - 0.2);
        vocSum += ppb;
        
        // Airflow calibration
        airflowSum += getSmoothedAirflow();
        delay(100);
    }
    
    baselineReading = (vocSum / numReadings) - STANDARD_PPB;
    baselineFlow = airflowSum / numReadings;
    
    display.clearDisplay();
    display.println("Calibration done!");
    display.display();
    delay(2000);
}

void displaySensorData() {
    // Get VOC readings
    int vocValue = analogRead(VOC_PIN);
    float vocVoltage = vocValue * (5.0 / 1023.0);
    float Rs = (5.0 - vocVoltage) * R0 / vocVoltage;
    float ratio = Rs / R0;
    float acetonePPB = 1000.0 / (ratio - 0.2);
    float acetonePPM = acetonePPB / 1000.0;
    
    // Get airflow reading
    float airflow = getSmoothedAirflow();
    
    // Display data
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    
    display.println("Sensor Readings:");
    display.print("VOC: ");
    display.print(acetonePPB, 1);
    display.println(" ppb");
    display.print("Airflow: ");
    display.print(airflow, 3);
    display.println(" m/s");
    
    display.println("\nRisk Levels:");
    
    // Combined risk assessment
    display.print("COPD: ");
    display.println(getCombinedRisk(getCOPDRisk(acetonePPM), 
                                    getAirflowCOPDRisk(airflow)));
    
    display.print("Asthma: ");
    display.println(getCombinedRisk(getAsthmaRisk(acetonePPB),
                                    getAirflowAsthmaRisk(airflow)));
    
    display.print("Cancer: ");
    display.println(getCombinedRisk(getCancerRisk(acetonePPM),
                                    getAirflowCancerRisk(airflow)));
    
    display.display();
}

float getSmoothedAirflow() {
    float sum = 0;
    
    for(int i = 0; i < SAMPLES_COUNT; i++) {
        int rawValue = analogRead(AIRFLOW_SENSOR_PIN);
        float voltage = rawValue * (5.0 / 1023.0);
        
        // Convert voltage to flow velocity using polynomial approximation
        // Using coefficients from manual for D6F-V03A1
        float x = voltage;
        float flow = (-2.62534 * pow(x, 6)) +
                     (20.87142 * pow(x, 5)) +
                     (-68.14970 * pow(x, 4)) +
                     (117.16178 * pow(x, 3)) +
                     (-111.95726 * pow(x, 2)) +
                     (58.03388 * x) +
                     (-12.00028);
        
        // Ensure non-negative values
        flow = max(0.0, flow);
        sum += flow;
        delay(10);
    }
    
    return sum / SAMPLES_COUNT;
}

String getCOPDRisk(float acetonePPM) {
    if (acetonePPM >= ACETONE_COPD_LOW && acetonePPM <= ACETONE_COPD_HIGH) {
        return "High";
    }
    return "Low";
}

String getAsthmaRisk(float acetonePPB) {
    return (acetonePPB >= ACETONE_ASTHMA) ? "High" : "Low";
}

String getCancerRisk(float acetonePPM) {
    if (acetonePPM >= ACETONE_CANCER_LOW && acetonePPM <= ACETONE_CANCER_HIGH) {
        return "High";
    }
    return "Low";
}

String getAirflowCOPDRisk(float airflow) {
    return (airflow < AIRFLOW_COPD) ? "High" : "Low";
}

String getAirflowAsthmaRisk(float airflow) {
    return (airflow < AIRFLOW_ASTHMA) ? "High" : "Low";
}

String getAirflowCancerRisk(float airflow) {
    return (airflow < AIRFLOW_CANCER) ? "High" : "Low";
}

String getCombinedRisk(String vocRisk, String airflowRisk) {
    if (vocRisk == "High" && airflowRisk == "High") {
        return "Very High";
    } else if (vocRisk == "High" || airflowRisk == "High") {
        return "High";
    }
    return "Low";
}

void handleButton() {
    switch(currentMenu) {
        case M_HEIGHT:
            currentMenu = M_GENDER;
            break;
        case M_GENDER:
            currentMenu = M_AGE;
            break;
        case M_AGE:
            if (isValid()) {
                currentMenu = M_TEST;
            } else {
                currentMenu = M_HEIGHT;
            }
            break;
        case M_TEST:
            if (!g.testing) {
                g.testing = true;
                g.maxFlow = g.maxVOC = 0;
            } else {
                g.testing = false;
                currentMenu = M_RESULTS;
            }
            break;
        case M_RESULTS:
            currentMenu = M_HEIGHT;
            break;
    }
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    
    switch(currentMenu) {
        case M_HEIGHT:
            display.println("Set Height:");
            display.print(g.height);
            display.println(" inches");
            // Show valid range
            display.println("\nValid heights for");
            display.print("Age ");
            display.print(g.age);
            display.print(g.isMale ? " M:" : " F:");
            displayValidHeights();
            break;
            
        case M_GENDER:
            display.println("Set Gender:");
            display.println(g.isMale ? "Male" : "Female");
            break;
            
        case M_AGE:
            display.println("Set Age:");
            display.println(g.age);
            break;
            
        case M_TEST:
            if (g.testing) {
                display.println("Testing...");
                display.print("Flow: ");
                display.println(g.maxFlow, 2);  // Show 2 decimal places
                display.print("VOC: ");
                display.println(g.maxVOC, 2);
            } else {
                display.println("Press to start test");
                display.println("\nCurrent settings:");
                display.print("Age: ");
                display.println(g.age);
                display.print("Gender: ");
                display.println(g.isMale ? "Male" : "Female");
                display.print("Height: ");
                display.print(g.height);
                display.println(" in");
            }
            break;
            
        case M_RESULTS:
            display.println("Results:");
            display.print("Max Flow: ");
            display.println(g.maxFlow, 2);
            display.print("Max VOC: ");
            display.println(g.maxVOC, 2);
            displayDiagnosis();
            break;
    }
    
    display.display();  // Make sure we're calling display() after all drawing
}

void displayDiagnosis() {
    // Add your diagnosis logic here based on g.maxFlow and g.maxVOC
    // compared against currentParams thresholds
}

bool isValid() {
    for (int i = 0; i < numCombs; i++) {
        if (combs[i].age == g.age && 
            combs[i].male == g.isMale && 
            combs[i].height == g.height) {
            return true;
        }
    }
    return false;
}

float getFlow() {
    float sum = 0;
    for(int i = 0; i < 10; i++) {
        int raw = analogRead(VOC_PIN);
        float v = raw * (5.0 / 1023.0);
        float x = v;
        sum += (-2.62534 * pow(x,6) + 20.87142 * pow(x,5) - 
                68.1497 * pow(x,4) + 117.16178 * pow(x,3) - 
                111.95726 * pow(x,2) + 58.03388 * x - 12.00028);
        delay(10);
    }
    return max(0.0, sum / 10.0);
}

void displayWelcome() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("Breath Analysis");
    display.println("Device v1.0");
    display.display();
    delay(2000);
}

// Modified setupParameterFiles function
void setupParameterFiles() {
    File csvFile = SD.open("PARAMS.CSV");
    if (!csvFile) {
        displayError("CSV file not found!");
        return;
    }

    // First, delete any existing parameter files
    File root = SD.open("/");
    while (File entry = root.openNextFile()) {
        String name = entry.name();
        if (name.endsWith(".txt")) {
            SD.remove(name);
        }
        entry.close();
    }
    root.close();

    // Read CSV header (skip first line if it contains headers)
    csvFile.readStringUntil('\n');

    // Process each line
    while (csvFile.available() && numCombs < MAX_COMBS) {
        String line = csvFile.readStringUntil('\n');
        
        // Skip empty lines
        if (line.length() < 3) continue;
        
        // Parse CSV line
        String gender = line.substring(0, line.indexOf(','));
        line = line.substring(line.indexOf(',') + 1);
        
        int age = line.substring(0, line.indexOf(',')).toInt();
        line = line.substring(line.indexOf(',') + 1);
        
        float heightCm = line.substring(0, line.indexOf(',')).toFloat();
        int heightInches = round(heightCm * CM_TO_INCHES);
        line = line.substring(line.indexOf(',') + 1);
        
        // Check if we have valid predicted value
        float predicted = line.substring(0, line.indexOf(',')).toFloat();
        if (predicted <= 0) continue;  // Skip if no valid prediction

        // Store valid combination
        combs[numCombs].age = age;
        combs[numCombs].male = (gender.equals("M") || gender.equals("Male"));
        combs[numCombs].height = heightInches;
        numCombs++;

        // Get all FEV1 values
        ThresholdLevels thresholds;
        thresholds.predicted = predicted;
        line = line.substring(line.indexOf(',') + 1);
        
        thresholds.threshold80 = line.substring(0, line.indexOf(',')).toFloat();
        line = line.substring(line.indexOf(',') + 1);
        
        thresholds.threshold70 = line.substring(0, line.indexOf(',')).toFloat();
        line = line.substring(line.indexOf(',') + 1);
        
        thresholds.threshold60 = line.substring(0, line.indexOf(',')).toFloat();
        line = line.substring(line.indexOf(',') + 1);
        
        thresholds.threshold50 = line.substring(0, line.indexOf(',')).toFloat();
        line = line.substring(line.indexOf(',') + 1);
        
        thresholds.threshold35 = line.toFloat();

        // Create parameter file name
        String fileName = String(age) + 
                         (combs[numCombs-1].male ? "M" : "F") + 
                         String(heightInches) + ".txt";

        // Write parameters to new file
        File paramFile = SD.open(fileName, FILE_WRITE);
        if (paramFile) {
            paramFile.println(thresholds.predicted);
            paramFile.println(thresholds.threshold80);
            paramFile.println(thresholds.threshold70);
            paramFile.println(thresholds.threshold60);
            paramFile.println(thresholds.threshold50);
            paramFile.println(thresholds.threshold35);
            paramFile.close();
        }
    }
    
    csvFile.close();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("Parameters loaded");
    display.print(numCombs);
    display.println(" combinations");
    display.display();
    delay(2000);
}

// Modify displayValidHeights function
void displayValidHeights() {
    bool found = false;
    for (int i = 0; i < numCombs; i++) {
        if (combs[i].age == g.age && combs[i].male == g.isMale) {
            if (!found) {
                display.println();
                found = true;
            }
            display.print(combs[i].height);
            display.print(" ");
        }
    }
    if (!found) {
        display.println("\nNo valid heights");
    }
}

void displayError(const char* error) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("ERROR:");
    display.println(error);
    display.display();
}