# GS232emu

Copyright (c) 2012-2022 by LA1FTA Morten Johansen <morten@bzzt.no>

## Description

Firmware that emulates a Yaesu GS-232A rotor controller. Drives a 3-phase motor as a stepper via H-bride drivers.

Quality of emulation is not yet verified, but the command interface works via hamlib. Supports rotation to specific azimuth, fetching current position and speed control.

Tested with: `rotctld -m 601 -r /dev/ttyACM0 -s 9600`

Lacks some work for supporting the commands related to elevation.

## Build instructions

* Install [PlatformIO](https://platformio.org/)
* Enter `firmware` directory
* Build: `platformio run`
* Upload: `platformio run -t upload`

PlatformIO will handle all dependencies.

## Future might bring

* Custom PCB
* Elevation support
* Support for more motor types and sensors

## License

This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License. To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
