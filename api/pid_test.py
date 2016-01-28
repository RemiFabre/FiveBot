#!/usr/bin/env python3

import fivebot

car = fivebot.Car("/dev/ttyUSB0")
pid = fivebot.PositionController(car, 5, .5, .8)
pid.set_target(1, 0, 0)
pid.start()
