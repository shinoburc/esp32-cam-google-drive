#!/bin/sh

platformio run -e esp32dev
platformio serialports monitor -b 115200
