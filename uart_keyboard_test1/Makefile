MCU := atmega328p
F_CPU := 8000000
PROGRAMMER := usbasp

uart_keyboard_test1.o:
	avr-gcc -Os -mmcu=$(MCU) uart_keyboard_test1.c -o uart_keyboard_test1.o -DF_CPU=$(F_CPU)

uart_keyboard_test1.hex: uart_keyboard_test1.o
	avr-objcopy -j .text -j .data -O ihex uart_keyboard_test1.o uart_keyboard_test1.hex

uart_keyboard_test2_master.o:
	avr-gcc -Os -mmcu=atmega8 uart_keyboard_test2.c -o uart_keyboard_test2_master.o -DF_CPU=$(F_CPU) -D__MASTER__

uart_keyboard_test2_master.hex: uart_keyboard_test2_master.o
	avr-objcopy -j .text -j .data -O ihex uart_keyboard_test2_master.o uart_keyboard_test2_master.hex

uart_keyboard_test2_slave.o:
	avr-gcc -Os -mmcu=atmega328p uart_keyboard_test2.c -o uart_keyboard_test2_slave.o -DF_CPU=$(F_CPU) -D__SLAVE__

uart_keyboard_test2_slave.hex: uart_keyboard_test2_slave.o
	avr-objcopy -j .text -j .data -O ihex uart_keyboard_test2_slave.o uart_keyboard_test2_slave.hex

clean:
	rm -f uart_keyboard_test1.o uart_keyboard_test1.hex
	rm -f uart_keyboard_test2_master.o uart_keyboard_test2_master.hex
	rm -f uart_keyboard_test2_slave.o uart_keyboard_test2_slave.hex

test1: uart_keyboard_test1.hex

test2: uart_keyboard_test2_slave.hex uart_keyboard_test2_master.hex

flash_test1: uart_keyboard_test1.hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:uart_keyboard_test1.hex

flash_test2_master: uart_keyboard_test2_master.hex
	avrdude -c $(PROGRAMMER) -p atmega8 -U flash:w:uart_keyboard_test1_master.hex

flash_test2_slave: uart_keyboard_test2_slave.hex
	avrdude -c $(PROGRAMMER) -p atmega328p -U flash:w:uart_keyboard_test1_master.hex

all: build
