# VEX V5 Competition Code

This repository contains the code for our 2026 VEX V5 competition robot using PROS and LemLib.

## Features
- Odometry tracking
- Distance Sensor Tracking
- Autonomous path following
- Custom LVGL auton selector
- Driver control system
  
## Setup
1. Install VS code
2. Dowload the PROS extension
3. Download the Clangd extension
4. Download the project
5. Open the project in VS code as a pros file
6. Install LemLib by pasting in the terminal:
  pros c add-depot LemLib https://raw.githubusercontent.com/LemLib/LemLib/depot/stable.json 
  pros c apply LemLib

## Learning 
To best use this code you should have a basic understanding of how it works you can use these websites to help you
- [PROS Documentation](https://wiki.purduesigbots.com/software/odometry)
- [LemLib Documentation](https://lemlib.readthedocs.io/en/stable/tutorials/1_getting_started.html)
- [LemLib Movements in Depth](https://www.aadishv.dev/move2pointx)

## File Structure
**src/**
 - main.cpp
 - auto.cpp
 - config.cpp
 - reset.cpp
 - Unity_Logo.c #for auton selector
 - Unity_Logo_red.c #for auton selector

**include/**
 - auton.h
 - config.h
 - main.h
 - reset.h

## Usage
Use the LemLib functions to create autonomous paths 
Select autonomous routine on brain screen before match.
Use the UC Logo to change what color you are

## Custom.py
This is the only python file it is not connected to any of the other documents.
It uses its own custom odometry and PID drive it is very simple. We will work on this
throughout the year and try to improve it so we can possibly use it for match use.

## Author
Blue Crew 

  <img width="482.5" height="341.25" alt="Unity_Logo" src="https://github.com/user-attachments/assets/218b5796-5c3d-4b36-aa98-1f600187c1dc" />



