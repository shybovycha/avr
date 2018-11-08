# Simple AVR test

## Compiling program

```
avr-gcc -Os -mmcu=atmega328p -c test1.c
avr-gcc -Os -mmcu=atmega328p -o test1.elf test1.o
avr-objcopy -j .text -j .data -O ihex test1.elf test1.hex
```

## Connecting the MCU

The chinese avrdude has somewhat strange pinout on its end connector:

```
Vcc   MOSI
x     x
x     RESET [notch]
x     SCK
GND   MISO
```

The pins on ATMega328P should be connected as follows:

```
1 - RESET + 10k resistor to Vcc
7 - Vcc
8 - GND

9, 10 - XTAL (external crystal oscillator; requires fuse bits to be set; calculate them on [this website](http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p)

17 - MOSI
18 - SCK
19 - MISO
```

USBASP programmer provides you with either 3.3V or 5V (set by a jumper) power already, so you do not need any external battery and 7805 to drop it down to 5V.

## Flashing the program

```
avrdude -p m328p -cusbasp-clone -U flash:w:test1.hex 
```

For external 8+ MHz crystal oscillator, flash the fuse bits:

```
avrdude -p m328p -cusbasp-clone -U flash:w:test1.hex -U lfuse:w:0x5F:m
```

**NOTE:** if you have set the fuse bits to use external oscillator - your MCU won't start and won't be available to even read the program from by a programmer without the external oscillator connected to the MCU.

