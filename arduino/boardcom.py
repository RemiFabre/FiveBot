#!/usr/bin/env python3

import serial
import struct
import threading

class BoardCom:
    
    def __init__(self, tty):
        self.com = serial.Serial(tty, baudrate=2000000, stopbits=2)
    
    def run(self):
        while True:
            self.parse_command()
    
    def parse_command(self):
        self.com.readline() # skip incomplete packets
        cmd = self.com.read().decode("ascii")
        if cmd == "o":
            self.on_odometry(*self.read_packet("fff"))
        elif cmd == "w":
            for i in range(4):
                self.on_wheel(i, *self.read_packet("hhhf"))
    
    def read_packet(self, fmt):
        fmt = "<" + fmt
        return struct.unpack(fmt, self.com.read(struct.calcsize(fmt)))
    
    def on_odometry(self, x, y, w):
        pass
    
    def on_wheel(self, id, power, position, errors, speed):
        pass
