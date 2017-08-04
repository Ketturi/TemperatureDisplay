# TemperatureDisplay
1Wire 7-segment temperature dispaly with attiny 2313 and ds1820
Uses Danfoss EKA162 Remote Display as hardware base.
Converts original remote display to standalone thermometer.

Modified firmware is intended for laser cutter coolant system to measure water temperature
coming from CO2 laser tube.

# Hardware
![alt text](https://raw.githubusercontent.com/Ketturi/TemperatureDisplay/master/EKA162.png)
Display board contains 5V linear regulator supply (not drawn in schematic), onewire interface,
reset circuitry, two rubber dome buttons and multiplexed 7-segment display with 3 digits and
6 other symbols. Atmel ATTiny 2313 acts as controller.

1-wire circuit uses UART TX and RX pins with open collector inverter to drive bus. RX pin receives data both ways, and TX pin drives line down. 1-Wire bus is pulled up with resistors by default.

7-Segment LED display is multiplexed trough 9 cathodes and 3 common anodes. Anodes are connected with pnp transistors to ease load of AVR GPIO pins. Cathodes are connected to gpio pins trough resistors.

Anode lines also drive button multiplexing. Up and Down buttons are connected to first and second anode, and feed back external interupt INT0.

# Software

Software consist displa driver, simple DS18B20 temperature sensor and one wire library and glue code. Current version contains logic to measure and show temperature with 1 decimal resolution, maximum temperature display that stores value to EEPROM and button logic that enables max temperature display, reset and high tempereature warning reset.

Software contains also basic error handling. Onewire bus is constantly checked for errors, and can return following error codes to display:
Er.1: 1-wire communication error
Er.2: Received data contains errors
Er.3: Onewire bus stuck on low level
Er.4: Other error

Watchdog timer resets MCU in 4 seconds after error, and tries to initialize onewire bus again.

Software drives 4 indicator leds, lowest led acts as busy indicator, second led indicates maximum temperature displayed, third led acts as EEPROM access indicator and uppermost leds warns from excessive temperature.
