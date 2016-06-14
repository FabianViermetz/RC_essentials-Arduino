# RC Essentials

The Goal of this Project is to build a PWM to PPM Converter with an Arduino Pro Mini or similar Boards.

This helps to reduce wiring in RC-Models and increase the range of Functions for controllers like Naze, Multiwii and so on.


i managed to push the limits of an ATMega328 and added 18CH PWM input and output which makes it perfect for a Hexapod servo controller and many other projects that require more PWM than usual recievers/converters can give.

## Features:
### Input
* 18CH PWM
* 8CH PPM
* 18CH SBUS (you might need an inverter)

You can add own inputs via
* Standard Serial UART
* I2C


### Output
* 18CH PWM
* xCH PPM
* 18CH SBUS (again you might need an inverter)

You can add own outputs via
*Standard Serial UART
*I2C

### General
* PWM-input smoother/filter
* offset for all channels

There is still room for custom code e.g. for signal-modifications during conversion.

Please use this with causion since not every option/combination is tested yet!