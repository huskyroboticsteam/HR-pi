# Instrumentation Control System
Control system for Husky Robotics Science box. Gathers sensor data and controls actuators.
Uses a custom SPI interface between raspberry pi and FPGA (Look in functions.c in C_Code and SPI.sv in verilog for more info)

## File Structure:

```
C_Code -- All C Code to control the Raspberry Pi
    
    FPGATesting -- Code for testing FPGA connection
    
    SystemsTesting -- Code for testing the system
    
    Testing -- Overall testing code
    
    Webcam -- Interfaces with cameras. Also contains Spectrometer code
    
    archive -- Unused code kept for reference
    
    sdp_i2c -- i2c interface for pressure sensor
    
Executables -- Contains all executables for the system

calibration_data -- Contains raw csv calibration files

output_data -- Contains raw sensor output data

verilog -- Contains all files for the FPGA
```

