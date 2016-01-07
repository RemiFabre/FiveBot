#!/usr/bin/env python

import sys
import time
from Queue import Queue as Q
import RPi.GPIO as GPIO

assert len(sys.argv) is 3
PIN_A = int(sys.argv[1])
PIN_B = int(sys.argv[2])

q = Q()
def edge(pin):
    q.put_nowait(pin)

GPIO.setmode(GPIO.BOARD)

GPIO.setup(PIN_A, GPIO.IN)
GPIO.add_event_detect(PIN_A, GPIO.BOTH, callback=edge)

GPIO.setup(PIN_B, GPIO.IN)
GPIO.add_event_detect(PIN_B, GPIO.BOTH, callback=edge)

try:
    values = 0
    errors = 0
    old_val = None
    while True:
        val = q.get()
        values = values + 1
        if val is old_val:
            errors = errors + 1
            print "%d / %d (%d%%)" % (errors, values, 100 * errors / values)
        old_val = val
finally:
    GPIO.cleanup()
