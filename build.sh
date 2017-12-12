#!/bin/bash

arduino-builder -hardware /arduino-1.8.5/hardware -tools /arduino-1.8.5/tools -tools /arduino-1.8.5/hardware/tools/ -fqbn adafruit:avr:trinket3 -libraries /lib-thirdparty/ -tools /arduino-1.8.5/tools-builder/ -hardware /hardware-thirdparty/ -verbose ./goggles1.ino
