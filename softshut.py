#!/usr/bin/env python

# Import the modules to send commands to the system and access GPIO pins
from subprocess import call
import RPi.GPIO as GPIO
from time import sleep
import board
import neopixel

#Switch off the neopixels
pixel_pin = board.D18
num_pixels = 30
ORDER = neopixel.GRBW
pixels = neopixel.NeoPixel(pixel_pin, num_pixels, brightness=0.6, auto_write=False,
                           pixel_order=ORDER)


# Map pin seven and eight on the Pi Switch PCB to chosen pins on the Raspberry Pi header
# The PCB numbering is a legacy with the original design of the board
PinSeven = 7
PinEight = 11
GPIO.setmode(GPIO.BOARD) # Set pin numbering to board numbering
GPIO.setup(PinSeven, GPIO.IN) # Set up PinSeven as an input
GPIO.setup(PinEight, GPIO.OUT, initial=1) # Setup PinEight as output

while (GPIO.input(PinSeven) == False): # While button not pressed
 GPIO.wait_for_edge(PinSeven, GPIO.RISING) # Wait for a rising edge on PinSeven
 sleep(0.1); # Sleep 100ms to avoid triggering a shutdown when a spike occured

pixels.fill((0, 0, 0, 0))
pixels.show()
sleep(2); # Sleep 2s to distinguish a long press from a short press

if (GPIO.input(PinSeven) == False):
 GPIO.output(PinEight,0) # Bring down PinEight so that the capacitor can discharge and remove power to the Pi
 call('poweroff', shell=False) # Initiate OS Poweroff
else:
 call('reboot', shell=False) # Initiate OS Reboot