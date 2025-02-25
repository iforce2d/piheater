# Raspberry Pi oven/hotplate controller

This is a Qt application for Raspberry Pi to read a MAX31855 thermocouple and control PWM and digital outputs. A sequence of temperature ramps can be configured as a preset 'profile', to define a target temperature that changes over time. There is no proper (eg. PID) control loop between the inputs, just a simple on/off toggle based on a temperature range.

Should run ok on Pi 3 and 4 (not 5).

### Setup

Not sure if all these packages are strictly necessary... 

    sudo apt install qtcreator qmake qt6-tools-dev-tools qt6-base-dev qt6-tools-dev
    cd src
    qmake
    make
    sudo ./reflowController

### Pins

![alt text](https://www.iforce2d.net/tmp/piheaterPinout.png)

### Customizing

Look at the HeatProfile::tick() function to modify the interaction between the input and outputs. Currently it just sets the 'relay' output high and PWM output to 15% when the temperature is more than 2 degrees below the target, and turns these off (and sets the 'cooling' output high) when more than 2 degrees above the target. This is probably not very useful for anything practical, so be sure to implement your own behavior.


<br>
<br>
<br>
<br>
<br>
<br>
