# What's this?
An open IoT thermostat firmware for ESP8266 and [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos/), a nice FreeRTOS implementation for this microcontoller.
This my first hardware/embeddded software project using the ESP8266, so it's intended to expolit as many features of this MCU, so I decided to make it public as it may be useful for others too.

# Features of version v1.0
* Two Dallas temperature sensors for inside and outside temperatures.
* The device acts as a WiFi AP and hosts a lightweight HTTP/CGI server,
which provides a convient, mobile optimized website for controlling the thermostat
and check the system's status. It also features a log, which stores the temperature
in 10-minute intervals and all the events (new temperature set, relay turns on/off).
* The project originally featured four, 7-segment displays for display and a
a tactile button and a potentiometer for user input, before I've finished the
webserver module. The device I currently use in my home lacks this display
and the relevant parts are not compiled into the firmware.
* Each hardware module is controlled by concurrent RTOS tasks. The main task
gathers temperature data (from sensors and optionally from user input) using the
RTOS task notification API and notifies subtasks (eg. display, server) to take specific actions.

# New features of the "bleeding edge" version
* Hysteresis is now can be adjusted from the user interface
* The firmware now can be compiled in two modes: *server* and *client*.
Client stands for a device which is identical to the thermostat (server), but
without the relay module. It is used for outside temperature measurement,
so the second sensor is not wire-connected anymore. It acts as a WiFi station
and connects to the AP of the thermostat every 10 minutes and sends it's
temperature reading as a HTTP request.
To compile it as a client, just define **IS_CLIENT 1**

# How to compile?
For the ease of use, I simply created and maintained this project in the *examples* folder
of [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos/). So to download and build it
just cd into that folder and:

**git clone --recursive https://github.com/abprogramming/esp8266-thermostat.git**
**cd esp8266-thermostat**
**make html && make flash -j8**

# What is needed for the hardware?
I'm using Wemos D1 and D1 Mini boards for developing, but I guess other ESP8266 boards will also do
the job if they have enough GPIO pins available. Besides that you'll need:
- Two Dallas DS18B20
