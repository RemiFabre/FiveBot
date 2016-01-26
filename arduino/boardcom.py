#!/usr/bin/env python3

import serial
import struct
import threading

STX = 2
ETX = 3
ESC = 27

class BoardCom:
    
    def __init__(self, tty):
        self.com = serial.Serial(tty, baudrate=115200, stopbits=1)
    
    def run(self):
        while True:
            self.parse_command()
    
    def parse_command(self):
        cmd, packet = self.read_packet()
        try:
            if cmd == "o":
                self.on_odometry(*struct.unpack("<fff", packet))
            elif cmd == "w":
                self.on_wheel(*struct.unpack("<bbhHf", packet))
            elif cmd == "i":
                self.on_info(packet.decode())
            else:
                raise Exception("unknown command: " + cmd)
        except Exception as e:
            self.on_error("cmd = %c, packet = [ %s ], %s" % (cmd, " ".join([ "%.2x" % byte for byte in packet ]), e))
    
    def read_packet(self):
        while (self.com.read()[0] != STX):
            pass
        cmd = chr(self.com.read()[0])
        packet = []
        while True:
            byte = self.com.read()[0]
            if byte == ETX:
                break
            elif byte == ESC:
                byte = self.com.read()[0] ^ ESC
            packet += [ byte ]
        return cmd, bytes(packet)
    
    def send_command(self, cmd, fmt, *args):
        data = [ STX, ord(cmd) ]
        for byte in struct.pack(fmt, *args):
            if byte == STX or byte == ETX or byte == ESC:
                data += [ ESC ]
                byte = byte ^ ESC
            data += [ byte ]
        data += [ ETX ]
        self.com.write(bytes(data))
    
    def on_odometry(self, x, y, w):
        pass
    
    def on_wheel(self, i, power, position, errors, speed):
        pass
    
    def on_info(self, msg):
        pass
    
    def on_error(self, msg):
        pass
    
    def send_speed(self, vx, vy, w, bypass_pid):
        self.send_command("s", "<fff?", vx, vy, w, bypass_pid)
