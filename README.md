# What's this?
An open IoT thermostat firmware for ESP8266 and [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos/), a really nice FreeRTOS implementation for this microcontoller.
This my first hardware/embeddded software project using the ESP8266, so it's intended to expolit as many features of this MCU, so I decided to make it public as it may be useful for others too.

# Features
* Two Dallas temperature sensors for inside and outside temperatures.
* The device acts as a WiFi AP and hosts a lightweight HTTP/CGI server,
which provides a convient, mobile optimized website for controlling the thermostat
and check the system's status. It also features a log, which stores the temperature
in 10-minute intervals and all the events (new temperature set, relay turns on/off).
* The project originally featured four, 7-segment displays for display and a
a tactile button and a potentiometer for user input, before I've finished the
webserver module. The version I currently use in my home does not have the display
board connected and the relevant parts are not compiled into the firmware.
* Each hardware module is controlled by concurrent RTOS tasks. The main task
gathers temperature data (from sensors and optionally from user input) using the
RTOS task notification API and notifies subtasks (eg. display, server) to take specific actions.
