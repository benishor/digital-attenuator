# digital-attenuator

## Features

- easily switchable three selectable attenuation levels
- pass-through function as easy as re-selecting the selected attenuator
- settings saving upon change and restoring on start-up

## Description

The aim of the project is to create a flexible digital step attenuator allowing the user to quickly switch between configured values or pass-through. It uses two PE4306 attenuator modules, allowing user selectable attenuations within a range of 1-62 dB with a step of 1dB.

## Modules needed

- 1 x 2x16 liquid crystal display (http://www.ebay.com/itm/LCD-Display-Module-Anzeigen-1602-Character-HD44780-2X16-Zeichen-blau-JTOO-/301990937671?hash=item4650100447:g:SzgAAOSwjXRXZ7ai)
- 1 x 4 channel level shifter  (http://www.ebay.com/itm/New-IIC-I2C-Logic-Level-Converter-Bi-Directional-Module-5V-to-3-3V-For-Arduino-/310965629892?hash=item4866fef3c4:g:VX8AAOSwbYZXdzWN)
- 5 x push buttons for the functions
- 2 x attenuator module (http://www.ebay.com/itm/RF-ATTENUATOR-0-31-dB-w-1-dB-STEP-SPI-MODULE-for-ARDUINO-or-MCU-PE4306-/201527616718?hash=item2eebfb64ce:g:HygAAOxyiRlSan6z)
- 1 x Arduino Uno/Nano or any compatible clone
- 1 x 10k trimpot
- 1 x 220Ohm resistor

## Connections

### LCD

 * RS pin to digital pin 12
 * Enable pin to digital pin 11
 * D4 pin to digital pin 5
 * D5 pin to digital pin 4
 * D6 pin to digital pin 3
 * D7 pin to digital pin 2
 * R/W pin to ground
 * 10K resistor:
 *    ends to +5V and ground
 *    wiper to LCD VO pin (pin 3)


### Attenuators

 * D6 -> attenuator clock (both units)
 * D7 -> attenuator data (both units)
 * D8 -> attenuator latch enable (unit 1)
 * D9 -> attenuator latch enable (unit 2)
 
### Push buttons
 * A0 -> select attenuation 1
 * A1 -> select attenuation 2
 * A2 -> select attenuation 3
 * A3 -> increase attenuation for the selected attenuator
 * A4 -> decrease atenuation for the selected attenuator
 

