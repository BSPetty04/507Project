# STM32 Gearbox Controller Term Project

This repository contains the firmware, documentation, and portfolio website for an STM32-based gearbox controller term project. The project combines a custom mechanical gearbox assembly, a custom STM32F411 controller PCB design, encoder speed feedback, actuator locking, user pushbuttons, a TFT display interface, and safety interlocks.

## Portfolio Website

Full project documentation and portfolio website:

[Open the project portfolio website](https://bspetty04.github.io/507Project/)

## Demo Video

A short demonstration video of the gearbox controller prototype is available on YouTube:

[Watch the prototype demo video](https://youtu.be/BHHqBvRPr1w)

## Project Overview

The goal of this project was to design and document a motorized gearbox demonstration system with embedded control. The system is built around an STM32F411 microcontroller and controls a DC motor, actuator/locking mechanism, magnetic door switch, input and output shaft encoders, user buttons, and TFT display.

The main control intent is a safety-supervised speed controller. The user selects an output shaft speed, the firmware reads encoder feedback, and the motor PWM is adjusted using a PI control approach. Safety logic prevents the gearbox from running when the enclosure is open and prevents the actuator lock from moving while the drivetrain is spinning.

## Key Features

- STM32F411-based embedded controller
- Custom PCB schematic and layout design
- 24 V powered system with regulated logic supplies
- DC motor PWM control
- Linear actuator lock/unlock control
- Magnetic door/limit switch safety interlock
- Input shaft and output shaft encoder speed measurement
- Closed-loop PI output shaft speed control
- TFT display for user feedback
- Three-button user interface
- Mechanical gearbox enclosure designed in SolidWorks
- CAD renders, prototype photos, schematics, and firmware documentation

## Control System

The project intelligence is based on three main behaviors:

1. **Closed-loop PI speed control**  
   The selected target output shaft RPM is compared against measured output shaft RPM from the encoder. The speed error is used by a PI controller to adjust motor PWM duty cycle.

2. **Safety interlocks**  
   The motor can only run when the door or magnetic switch indicates that the enclosure is closed. If the door opens while the motor is running, the firmware stops the motor. The actuator lock/unlock function is disabled while the gearbox is running.

3. **User interaction and feedback**  
   The buttons allow the user to start/stop the gearbox, lock/unlock the actuator, and cycle through selectable speed presets. The TFT display is intended to show target RPM, measured input/output RPM, lock state, door state, and fault state.

## Repository Structure

```text
.
├── index.html              # Portfolio homepage
├── hardware.html           # Mechanical hardware documentation
├── electronics.html        # PCB, schematic, and electrical documentation
├── software.html           # Firmware/software architecture
├── control.html            # PI control and safety logic documentation
├── media.html              # Photos, renders, and attachments
├── repository.html         # Repository and build notes
├── style.css               # Website styling
├── assets/                 # Images, schematic renders, and documentation assets
├── repo/                   # Firmware/source-code package
├── README.md               # This file
├── Doxyfile                # Doxygen configuration
└── .gitignore              # Git ignore rules
