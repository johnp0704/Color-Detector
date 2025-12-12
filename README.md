# Color-Detector
Final project for Optoelectronics at UVM. A color detector utilizing an addressable RGB LED and a light-dependent resistor. The inspiration and much of the ideation of this project is credited to HackMakeMod: https://hackmakemod.com/blogs/projects/ldr-color-checker

### Circuit Diagram
<img width="1607" height="811" alt="Color_Detector_Circuit_Diagram" src="https://github.com/user-attachments/assets/c1e1cffc-c9ba-47aa-a5e7-fabfda8cb996" />



### Parts List
* Arduino UNO or equivalent microcontroller
* Push Button
* 3x 2N7000 MOSFET
* 3x 220Ohm Resistors
* 1MOhm Resistor
* 1 LED Segment from a 5050 Addressable LED Strip (other LEDs of a similar form factor can be used)
* 10k LDR (photocell)
* LCD 16x2 Display with I2C Backpack
* 3D Printed Shroud - file can be downloaded here (modify as needed): https://hackmakemod.com/blogs/projects/ldr-color-checker)

### Code
This repository contains two scripts:

#### Color_List_Parser.py
This Python code takes a list of color names and RGB values and formats it into a useable library for the Arduino code. 

#### Color_Detector_v2.ino
This C++ code runs the Arduino UNO to be used for scanning colors.

### Use
With the circuit built and code implemented, usage of this device is simple. 

1) On startup, the device will enter calibration mode. First, place the detector on a pure white surface (such as a piece of paper) and press the button to scan. The LCD will display its readings then display when it is ready for the next scan. For the next scan, place the detector on a "pure" black surface (as close as you can find will suffice) and press the button to run the scan. The LCD will display its readings, then exit calibration mode.
2) To scan a color, place the detector on the surface you want to measure, and press the button. The LCD will display the read RGB values and the best-matching color name.
3) To re-enter calirbation mode, double-click the button in quick succession. 

### Documentation
For further documentation, see the "LDR_Color_Detector_Report.pdf" file. 
