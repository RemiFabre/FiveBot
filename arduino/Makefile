ARDUINO_PATH=/usr/share/arduino
ARDUINO_CC=$(ARDUINO_PATH)/hardware/tools/avr/bin/avr-g++
ARDUINO_OBJCOPY=$(ARDUINO_PATH)/hardware/tools/avr/bin/avr-objcopy
ARDUINO_AVRDUDE=$(ARDUINO_PATH)/hardware/tools/avr/bin/avrdude
ARDUINO_AVRDUDE_CONF=$(ARDUINO_PATH)/hardware/tools/avrdude.conf
ARDUINO_CFLAGS=-fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega328p -DF_CPU=16000000L -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -D__PROG_TYPES_COMPAT__ -Os -ffast-math -fsingle-precision-constant
ARDUINO_LDFLAGS=-Wl,--gc-sections -lm
ARDUINO_INCLUDES=-I$(ARDUINO_PATH)/hardware/arduino/cores/arduino -I$(ARDUINO_PATH)/hardware/arduino/variants/standard
ARDUINO_SRC=`find $(ARDUINO_PATH)/hardware/arduino/cores/arduino/ -name "*.c" -or -name "*.cpp")`

TARGET=arduino
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
CFLAGS=-std=c++11 -Wall

TTY=/dev/ttyUSB0

A_CC=$(ARDUINO_CC) $(ARDUINO_CFLAGS) $(ARDUINO_LDFLAGS) $(ARDUINO_INCLUDES) $(CFLAGS)
A_AVRDUDE=$(ARDUINO_AVRDUDE) -C $(ARDUINO_AVRDUDE_CONF) -p atmega328p -P $(TTY) -b 57600 -c arduino

all: clean build upload control

build: $(TARGET).hex

upload: build
	$(A_AVRDUDE) -D -U flash:w:$(TARGET).hex:i

serial:
	cu -l $(TTY) -s 2000000 --nostop

control:
	./boardgui.py $(TTY)

%.hex: %.elf
	$(ARDUINO_OBJCOPY) -O ihex $^ $@ -v

$(TARGET).elf: $(OBJ)
	$(A_CC) -w -fpermissive -o $@ $^ $(ARDUINO_SRC) $(LIBS)

%.o: %.cpp
	$(A_CC) $(ARDUINO_INCLUDES) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(TARGET).hex $(TARGET).elf *.o
	rm -rf __pycache__

.PHONY: build upload serial clean
