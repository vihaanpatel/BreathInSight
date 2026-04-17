# BreathInSight

A low-cost, non-invasive respiratory health monitoring device that uses breath analysis to detect potential indicators of respiratory disease.

## Overview
BreathInSight is a portable prototype designed to combine multiple sensing methods into one device. It integrates:
- VOC sensing
- Nitric oxide / NOx-related sensing
- Airflow measurement

The goal is to create a more accessible and affordable respiratory screening tool, especially for settings where traditional diagnostics are expensive or difficult to access.

## Why I Built This
Many respiratory conditions are detected late because clinical testing can be expensive, inaccessible, or inconvenient. I wanted to build a device that could provide real-time information from a simple breath sample using a portable embedded system.

## Features
- Real-time breath measurement
- Multi-sensor integration
- On-device display output
- Portable hardware architecture
- Iterative low-cost prototype design

## System Architecture
The current prototype uses:
- Raspberry Pi 5
- VOC sensor
- Flow sensor
- OLED display
- Portable power source

## Repository Structure
- `hardware/` – wiring diagrams, schematics, and bill of materials
- `firmware/` – main code, sensor interfaces, and display logic
- `docs/` – architecture, calibration, and testing notes
- `enclosure/` – CAD and STL files
- `media/` – photos and demo assets

## Current Status
- Functional prototype built
- Sensor integration in progress
- Calibration/testing ongoing
- Future work includes improving reliability and refining classification logic

## Author
[Your Name]
