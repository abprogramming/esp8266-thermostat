# What's this?
This is one of the many hardware/embedded software projects I've started building. I've decided to make public this one, since it's almost finished, so some ideas may be useful for others too - and maybe it's also an incentive for me to finish it... :-) Anyways, I use github for version control of my personal projects (in private repos) and maybe I can give back something to the community. 

It is one of the most basic DIY projects, which is built by many hobbyists: a thermostat, based on the ESP8266 MCU and [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos/), a really nice FreeRTOS implementation for this microcrontoller.

# Features
* Two Dallas temperature sensors for inside and outside temperatures.
I'm using one in the TO-92 package for the inside and one in the
waterproof version for the outside.
* Displaying temperature on four, 7-segment displays.
Originally I've bought an OLED display, but then I've found two new
7-segments in red colours at home and decided to use them to achieve
a more 'oldschool' look. Also I've found writing a driver for them 
more challenging with some good old bitbanging, so I've bought some
shift registers at the local electronic store and two more 7-segments
in yellow. I'm using them for displaying the outside temperature,
for the minus sign I've added a square-shaped LED. The fractional
part is displayed with a half-degree precision and it is displayed
by turning the decimal point on the second digit on.
* The user interface consists of a tactile button and a potentiometer.
By pressing the button the display is turned on to show the actual temperatures
and also an xTimer starts. If there are no further actions taken, the display
turns off after the timer expires. But if the user turns the knob of the potentiometer
the program enters the set mode. The potentiometer value is converted to
actual temperature values by ADC - the main reason for that choice is to
test and utilize as many features of the ESP8266 in a single project as I can.
* Each hardware module is controlled by concurrent RTOS tasks. The main task
gathers temperature data (from sensors and optionally from user input) using the
RTOS task notification API and notifies subtasks (eg. display) to take specific actions.
The main task is also responsible for the whole application's core functionality:
decide when to turn the relay on or off.

# Project status and further plans
* The program is 95% finished.
* The hardware prototype is built on a stripboard during Aug 2020.
In the following weeks I'll need to test and debug
with the software.
* Design and etch a DIY PCB for the final product
* If everything works I want to implement a minimal webserver, exploiting the
WiFi features of ESP8266, it's main strength. On a simple interface users will be allowed
to set temperatures and also to tweak with other settings, eg. hysteresis.

* Then I want to write my own Dallas 1-wire protocol driver. I've used the one which comes with
[esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos/), which is really nice and I could
achieve results fast, but it's a bit way too high level for me.

# Hardware prototype
<img src="https://github.com/abprogramming/esp8266-thermostat/blob/master/doc/mainpanel.png" width="500"><img src="https://github.com/abprogramming/esp8266-thermostat/blob/master/doc/userpanel.png" width="500">
