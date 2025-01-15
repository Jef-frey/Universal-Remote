# Universal-Remote (WIP)
The goal of this project is to build an universal remote controller for multiple different devices, only devices that use NEC Infrared(IR) Protocol are supported. The project is based on IRLib2 by Chris Young, more information can be found on https://github.com/cyborg5/IRLib2/tree/39168cf62fbeba02a49755d7f0ac3173e620c0e5/IRLib2. Explanation on NEC IR protocol can be found on the IRLib2 manual. The device is completed but this guide is still a work in progress, details to the PCB layout will be added.
![alt_text](https://github.com/Jef-frey/Universal-Remote/blob/46fbb8ee40a13d47b7df26cc8d72dc33309e8b31/Microcontroller/remoteFront.jpg?raw=true)![alt_text](https://github.com/Jef-frey/Universal-Remote/blob/46fbb8ee40a13d47b7df26cc8d72dc33309e8b31/Microcontroller/remoteBack.jpg?raw=true)

# Requirements
## Hardware
1x PIC16F15344<br />
1x PICkit 3<br />
1x IR diode<br />
1x n-channel MOSFET<br />
1x 10k Ohm Potentiometer<br />
1x 470 Ohm Current limiting resistor<br />
1x 0.1 uF Capacitor<br />
1x Battery Holder<br />
Switches that fit your application<br />

## Software
MPLAB X IDE 6.20<br />
KiCad 8.0<br />

# Programming this remote for your devices
1. find the IR code of your devices, if they are not the NEC format then the code will not work
2. check if the code is followed by it's logical inverse, aka the number of 1s and 0s are both 16. This is because the NEC protocol uses padding at the end to ensure that each signal is 108ms, and that padding durations is hardcoded. If the number of 1s and 0s are always 16, the padding duration is always the same so hardcoding it would not cause any issue. But if the number of 1s and 0s are not 16, some changes to function 'set_TMR0_padding()' and 'set_TMR0_repeat_padding()' in file 'transmit.c' may be needed
3. add the IR code to the file 'transmit.h' using the format '#define \[button name] \[IR code in hex]', I put some examples for my LG TV between line 17 to 31
4. add the button name from step 2 the 'switch_select' functions in file 'newmain.c', I put some examples there as well. The 'IOC' conditions in the if statements indicate which input pin that button will use and the parameter location on the 'switch_select' functions indicate which output pin that button will use
5. program your PIC

# Microcontroller
The following is the c code that goes into the microcontroller

## IR emitter
B6 (pin 11) will be connected to the IR LED to transmit IR signals. Since NEC IR Protocol uses signal modulated with a 38kHz pulse train, timer2 is used to create a PWM signal with 50% duty cycle, and timer0 will then be used to enable and disable the PWM signal at different internal to create the signal itself.

## Buttons
A2 (pin 17), A4 (pin 3), A5 (pin 2), B4 (pin 13), B5 (pin 12), B7 (pin 10), C0-C2 (pin 16-14), C3-C5 (pin 7-5), C6 (pin 8), C7 (pin 9), will be used for buttons input. A0 (pin 19), A1 (pin 18), and A3 (pin 4) are unused since they have issues when programmed as regular IO pins.
To increase the amount of buttons that can be used, the buttons are programmed to be used as a matrix. Instead of using the 14 pins as all inputs for a total of 14 buttons, by using them as a 2x12 matrix, there will be a total of 24 buttons available. To implement this, A4 and A5 will be used as output and all other pins as input. The pins of the microcontroller and pins of the buttons will be connected in a matrix where A4 and A5 are the rows and all the other pins are the columns.
![alt text](https://github.com/Jef-frey/Universal-Remote/blob/3d08582dd7476987d76cc73bbe6af61f81970d02/Microcontroller/button_matrix.jpg?raw=true)
Under this configuration, by making only one of the row pins low at one time (done by using timer1 to switch A4 and A5 high and low every 49.12ms); when one of the button is pressed, one of the input pins will be pulled low, and then the code will check which output pins is set to low. By tracing the input pin that is pulled low (column) and knowing the output pin that is set to low (row), the pressed button can be determined.
However, this configuration does have some drawbacks in that the system may not be able to detect two pressed buttons at a time and create a short circuit when two buttons are pressed, therefore do not press two buttons at the same time.

