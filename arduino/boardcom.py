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
    
    def build_packet(self, fmt, *args):
        fmt = "<" + fmt
        return struct.pack(fmt, *args)
    
    def send_command(self, cmd, fmt, *args):
        self.com.write("s".encode("ascii") + self.build_packet(fmt, *args) + "\r\n".encode("ascii"))
    
    def on_odometry(self, x, y, w):
        pass
    
    def on_wheel(self, id, power, position, errors, speed):
        pass
    
    def send_speed(self, vx, vy, w):
        self.send_command("s", "fff", vx, vy, w)
