# Architecture

BreathInSight is organized into three main subsystems:

## 1. Sensor Layer
Collects raw respiratory and chemical data from airflow and VOC/NOx-related sensors.

## 2. Processing Layer
Uses the Raspberry Pi to read sensor values, process measurements, and apply device logic.

## 3. Output Layer
Displays information to the user through the OLED screen.

## Data Flow
1. User exhales into the device
2. Sensors capture airflow and breath-related measurements
3. The Raspberry Pi processes the data
4. Results are displayed on-screen
