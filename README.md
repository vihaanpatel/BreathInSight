BreathInSight

A low-cost, non-invasive respiratory health monitoring device

Overview

BreathInSight is a portable system designed to detect early indicators of respiratory conditions (e.g., asthma, COPD, lung disease) using exhaled breath analysis.

The device integrates:

VOC sensing (airborne biomarkers)
Nitric oxide detection (inflammation indicator)
Airflow measurement (lung function)

The goal is to provide a low-cost, accessible alternative to traditional diagnostics like spirometry or CT imaging, especially in low-resource settings.

Why I Built This

Respiratory diseases are often diagnosed late due to cost and accessibility barriers. I wanted to build a system that could:

Be used at home or in clinics without specialized equipment
Combine multiple biomarkers into one device
Provide real-time feedback instead of delayed lab results

System Architecture

Core components:

Raspberry Pi 5 (processing + control)
VOC sensor (e.g., MQ-138 / SGP41)
Flow sensor (SFM series)
OLED display (I2C)
Power system (portable battery)

Features:

Real-time breath analysis
Multi-sensor fusion (VOC + NOx + airflow)
On-device risk classification
Portable + low-cost design

How It Works

User exhales into the device
Sensors collect airflow + chemical data
Data is processed in real time
Risk indicators are displayed on screen

Tech Stack

Python (data processing + sensor integration)
I2C communication (sensor interfacing)
Embedded systems (Raspberry Pi)
CAD (Fusion 360 and Onshape for enclosure)

Current Status

Functional prototype built
Sensor integration complete
Calibration and accuracy improvements ongoing

Next Steps
Improve NOx sensing reliability
Train more robust classification model
Miniaturize hardware
Clinical validation

Author
Vihaan Patel
