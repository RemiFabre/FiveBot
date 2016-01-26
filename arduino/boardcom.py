#!/usr/bin/env python3

import serial
import struct
import threading

STX = 2
ETX = 3
ESC = 0o33

class BoardCom:
    
    def __init__(self, tty):
        self.com = serial.Serial(tty, baudrate=2000000, stopbits=2)
    
    def run(self):
        while True:
            self.parse_command()
    
    def parse_command(self):
        while (self.com.read()[0] != STX):
            pass
        cmd = self.com.read().decode("ascii")
        if cmd == "o":
            self.on_odometry(*self.read_packet("fff"))
        elif cmd == "w":
            for i in range(4):
                self.on_wheel(i, *self.read_packet("hhhf"))
        while (self.com.read()[0] != ETX):
            pass
    
    def read_packet(self, fmt):
        fmt = "<" + fmt
        return struct.unpack(fmt, self.com.read(struct.calcsize(fmt)))
    
    def build_packet(self, fmt, *args):
        fmt = "<" + fmt
        return struct.pack(fmt, *args)
    
    def send_command(self, cmd, fmt, *args):
        start = "%c%s" % (STX, cmd)
        data = ""
        for byte in self.build_packet(fmt, *args):
            if byte == STX or byte == ETX or byte == ESC:
                data += "%c" % ESC
                byte = byte ^ ESC
            data += "%c" % byte
        end = "%c" % ETX
        self.com.write((start + data + end).encode("ascii"))
    
    def on_odometry(self, x, y, w):
        pass
    
    def on_wheel(self, id, power, position, errors, speed):
        pass
    
    def send_speed(self, vx, vy, w, bypass_pid):
        self.send_command("s", "fff?", vx, vy, w, bypass_pid)
