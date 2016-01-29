#!/usr/bin/env python3

from fivebot import Car, PositionController
from gui import DebugGui

car = Car("/dev/ttyUSB0")
gui = DebugGui().run(car)
pid = PositionController(car, 5, .5, .8)
pid.set_target(1, 0, 0)
pid.start()
