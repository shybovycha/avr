# V-USB mouse sample

## Compiling

```
# copy v-usb/usbdrv to the project directory
# copy usbconfig-prototype.h to usbconfig.h
# modify the usbconfig.h file and set these constants:
# # USB_CFG_IOPORTNAME
# # USB_CFG_DMINUS_BIT
# # USB_CFG_DPLUS_BIT
# # USB_CFG_CLOCK_KHZ
# # USB_CFG_IS_SELF_POWERED
# # USB_CFG_MAX_BUS_POWER
# # USB_CFG_VENDOR_NAME
# # USB_CFG_VENDOR_NAME_LEN
# # USB_CFG_DEVICE_NAME
# # USB_CFG_DEVICE_NAME_LEN

# compile usbdrv for the target MCU
avr-gcc -Wall -Os -DF_CPU=12000000 -Iusbdrv -DDEBUG_LEVEL=0 -mmcu=attiny44 -c usbdrv/usbdrv.c -o usbdrv/usbdrv.o
avr-gcc -Wall -Os -DF_CPU=12000000 -Iusbdrv -DDEBUG_LEVEL=0 -mmcu=attiny44 -x assembler-with-cpp -c usbdrv/usbdrvasm.S -o usbdrv/usbdrvasm.o
avr-gcc -Wall -Os -DF_CPU=12000000 -Iusbdrv -DDEBUG_LEVEL=0 -mmcu=attiny44 -c usbdrv/oddebug.c -o usbdrv/oddebug.o

# compile the program
avr-gcc -Wall -Os -Iusbdrv -DDEBUG_LEVEL=0 -mmcu=attiny44 -c vusb_mouse.c -o vusb_mouse.o

# link program
avr-gcc -Wall -Os -DF_CPU=12000000 -Iusbdrv -DDEBUG_LEVEL=0 -mmcu=attiny44 -o vusb_mouse.elf usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o vusb_mouse.o

# generate HEX file
avr-objcopy -j .text -j .data -O ihex vusb_mouse.elf vusb_mouse.hex

# flash the MCU
avrdude -p t44 -cusbasp -U flash:w:vusb_mouse.hex
```

