ARDUINO_PATH=/usr/share/arduino
ARDUINO_CC=$(ARDUINO_PATH)/hardware/tools/avr/bin/avr-g++
ARDUINO_OBJCOPY=$(ARDUINO_PATH)/hardware/tools/avr/bin/avr-objcopy
ARDUINO_AVRDUDE=$(ARDUINO_PATH)/hardware/tools/avr/bin/avrdude
ARDUINO_CFLAGS=-fno-exceptions -ffunction-sections -fdata-sections -fpermissive -mmcu=atmega328p -DF_CPU=16000000L -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -D__PROG_TYPES_COMPAT__
ARDUINO_LDFLAGS=-Wl,--gc-sections -lm
ARDUINO_INCLUDES=-I$(ARDUINO_PATH)/hardware/arduino/cores/arduino -I$(ARDUINO_PATH)/hardware/arduino/variants/standard
ARDUINO_SRC=`find $(ARDUINO_PATH)/hardware/arduino/cores/arduino/ -name "*.c" -or -name "*.cpp")`

CFLAGS=-g -Os -Wall
INCLUDES=-IPID -ISabertooth
TTY=/dev/ttyUSB0
OBJ=mapper
SRC=$(OBJ).cpp PID/PID.cpp Sabertooth/Sabertooth.cpp

upload: $(OBJ).hex
	$(ARDUINO_AVRDUDE) -C $(ARDUINO_PATH)/hardware/tools/avrdude.conf -p atmega328p -P $(TTY) -b 57600 -c arduino -D -U flash:w:$<:i

%.hex: %.elf
	$(ARDUINO_OBJCOPY) -O ihex $< $@ -v

$(OBJ).elf: $(SRC)
	$(ARDUINO_CC) $(ARDUINO_CFLAGS) $(ARDUINO_LDFLAGS) $(ARDUINO_INCLUDES) $(CFLAGS) $(INCLUDES) -o $@ $^ $(ARDUINO_SRC)

clean:
	rm -rf $(OBJ).hex $(OBJ).elf

.PHONY: upload clean
