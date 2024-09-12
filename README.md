# 3-D-Map-Generator-
This is a project I completed during my Microprocessor System Process Course. This project designed a system to map 3D models of spaces using a VL531LX ToF Sensor, MSP432E401Y microcontroller, and stepper motor. I collected distance data at intervals during 360° rotation, processed it with Python to derive x, y, z coordinates, and used Open3D to generate 3D models from multiple frames.

During this project I learned how to complete the following: 
•	Integrated a stepper motor with a VL53L1X time-of-flight sensor using infrared laser light to obtain 360-degree distance measurements.
•	Implemented a C++ program using Kiel to configure digital I/O pins on an MSP432E401Y microcontroller for reading LiDAR sensor data through I2C serial communication.
•	Developed a Python script to visualize time-of-flight sensor measurement data collected through UART communication from the MSP432E401Y in a 3D graphical representation using the comprehensive open3D library.

