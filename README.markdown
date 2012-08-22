#Particle collider on DX hacked gadget

![DX icon](http://photo.buyonme.com/buyonme-pic/productimage/electronics/electronics-gadgets-78285-1.jpg)

## Overview
This is the code to make a particle collider with a JY-3208 led matrix clock from DX.com

The device is hacked replacing the atmega8 with an atmega328P and Arduino UNO bootloader inside.

The crystal is replaced with another 16MHz crystal oscillator and added 2x33pF capacitors.

Over the HT1632C i glued a 3-axis accelerometer LIS344ALH and conected X,Y,Z axis outputs to A0,A1,A2 respectively.

I added too a V-Bus connection with 2x100ohms resistors to USB D+/D- and a pull-up 1k5 resistor.

AREF in Atmega328P was connected to 3,3v from a LM1117_3v3 LDO glued over the board and removed a decoupling 100nF capacitor between AREF and GND.

Too it was necessary to remove a pull-up resistor in Atmega328P ADC2.

### Forks

HT1632 Adafruit's library for Arduino <https://github.com/adafruit/HT1632>

Thank's for the free code Adafruit! 

I made some modifications to use with DX PCB 

### Credits

Programed and hacked by **A.Prado** a.k.a. **droky** at <http://radikaldesig.com>

Follow me at [@radikaldesig](http://twitter.com/radikaldesig)
