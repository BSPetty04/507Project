# Gearbox Controller Repository

This repository contains the firmware and documentation assets for the STM32F411-based gearbox controller term project.

## Documentation

Open `../index.html` after unzipping the portfolio package, or host the folder with GitHub Pages. The documentation includes mechanical design, custom PCB/schematic overview, firmware architecture, PI speed control, safety interlocks, prototype photos, and bring-up plans.

## Firmware Structure

- `Core/Src/app.c` - top-level state machine and closed-loop PI speed controller
- `Core/Src/motor.c` - PWM and direction control
- `Core/Src/actuator.c` - actuator lock/unlock and fault handling
- `Core/Src/buttons.c` - debounced button events
- `Core/Src/encoders.c` - timer encoder RPM calculations
- `Core/Src/ui.c` - display update interface

## Current Hardware Status

The custom PCB schematic and layout were completed. Physical PCB validation is listed as a next step because the fabricated board had not arrived at the time of documentation.
