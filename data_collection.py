# Student name: Nuha
# Python version: 3.8

import serial   
import math

ser = serial.Serial('COM3',115200) # Change COM Port #
ser.open

f = open(r"add File path here", "w") # Change name of file 
i = 0
x = 0 # initial x-displacement (mm)
increment = 2000 # x-increment in mm // 200 cm of distance between each frame
data_collected = False

while (1):  
    s = ser.readline()
    data = s.decode("utf-8").strip()  # Decode and strip newline/carriage return) # Decodes byte input from UART into string 
    ##a = a[0:-2] # Removes carriage return and newline from string
    if (data.isdigit() == True):
        data_collected = True
        angle = (i/32)*2*math.pi # Obtain angle based on motor rotation
        b = int(data)
        y = b*math.cos(angle) # Calculate y
        z = b*math.sin(angle) # Calculate z
        f.write('{} {} {}\n'.format(x,y,z)) # Write data to .xyz file
        i += 1
    if (data.isdigit() == False and data_collected == True): 
        f.close() 
        f = open(r"add file path here", "a")
    if i == 32: # 3D 
        i = 0
        x = x + increment
    print(data, x)
