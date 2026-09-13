# Slab Clock

Slab Clock is a 4 key alaram clock with a screen to display time and a buzzer for alarms. 

## Features:
- 3D printed body designed to be chunky like a slab
- 2.25in TFT Screen
- Alarm buzzer
- 4 Keys 


## CAD Model:
It has 2 separate printed pieces. The base where the PCB sits, and the top cover. The project name and my name is engraved on the top plate as well. It also has the screen cutout on the side rather than the front.

<img src=Assets/body2.PNG alt="Model" />

Made in Fusion360. 


## PCB
Here's my PCB! It was made in KiCad.

Schematic
<img src=Assets/sch.PNG alt="Schematic" width = 300/>

PCB
<img src=Assets/pcb.PNG alt="Schematic" width =300 />

## Firmware Overview
This project is coded in C++ and ran on the arduino IDE. The firmware is not 100% written by me. I adapted the code from https://github.com/PieSquared/TTTsaboard/blob/main/Firmware/FW.ino#L47C4-L49C7
