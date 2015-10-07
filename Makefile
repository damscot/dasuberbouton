PORT?=/dev/ttyUSB0
PROJECT=dasuberbouton.cpp
ARDUINO_MODEL=nano
MCU=atmega328p
UPLOAD_RATE=57600
ARDUINO_DIR=/work1/dcottier/arduino/arduino-1.6.5-r5
AVR_TOOLS_PATH=/work1/dcottier/arduino/arduino-1.6.5-r5/hardware/tools/avr/bin/
USER_LIBDIR=${HOME}/Arduino/libraries
USER_LIBS=RadioHead Adafruit_NeoPixel ubberFrame
include Makefile.arduino

