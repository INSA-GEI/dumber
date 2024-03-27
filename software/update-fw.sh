#!/bin/bash

# Script for updating firmware of dumber robot
# Author: S. DI MERCURIO
# Date : 25-03-2024
# Version 1.0-27.03.24

# Create temporary directory
mkdir ./tmp-robot
cd ./tmp-robot

# Get firmware from server
echo ""
wget --no-check-certificate https://download-gei.insa-toulouse.fr/Dumber/firmware-robot/dumber3-3.5-27.03.24.hex.zip

# Unzip firmware
echo ""
unzip *.zip

# Flash it
echo ""
read -n 1 -p "Check that robot is powered up, than press any key."
/usr/local/insa/STM32CubeProgrammer/bin/STM32_Programmer_CLI -c port=SWD mode=UR reset=HWrst freq=8000 -d *.hex -v -hardRst -run

# Clean up
echo ""
cd ..
rm -rf ./tmp-robot

