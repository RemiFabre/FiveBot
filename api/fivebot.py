import math
import serial
import struct
import threading

STX = 2
ETX = 3
ESC = 27

class FiveBot:
    
    encoder_increments_per_turns = 3072
    wheel_radius = .047
    half_lengths = [ .15, .15 ]
    magic = 2900
    
    def __init__(self, tty, x = 0, y = 0, w = 0):
        self.com = serial.Serial(tty, baudrate=115200, stopbits=1)
        self.x, self.y, self.w = x, y, w
        threading.Thread(target=self.run).start()
    
    def run(self):
        while True:
            self.parse_command()
    
    def parse_command(self):
        cmd, packet = self.read_packet()
        try:
            if cmd == "o":
                self.on_odometry_raw(*struct.unpack("<fff", packet))
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
    
    def on_odometry_raw(self, dx, dy, dw):
        dw *= 2 * math.pi * self.wheel_radius / 4 / sum(self.half_lengths) / self.magic
        dx *= self.wheel_radius * 2 * math.pi / self.encoder_increments_per_turns / 4
        dy *= self.wheel_radius * 2 * math.pi / self.encoder_increments_per_turns / 4
        c = math.cos(self.w + dw / 2)
        s = math.sin(self.w + dw / 2)
        dx, dy = c * dx + s * dy, -s * dx + c * dy
        self.x += dx
        self.y += dy
        self.w += dw
        self.on_odometry(self.x, self.y, self.w)
    
    def on_odometry(self, x, y, w):
        pass
    
    def on_wheel(self, i, power, position, errors, speed):
        pass
    
    def on_info(self, msg):
        pass
    
    def on_error(self, msg):
        pass
    
    def set_speed(self, vx, vy, w, bypass_pid):
        self.send_command("s", "<fff?", vx, vy, w, bypass_pid)
