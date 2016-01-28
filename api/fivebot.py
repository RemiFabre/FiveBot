import math
import serial
import struct
import threading

STX = 2
ETX = 3
ESC = 27

class Event(list):
    def __call__(self, *args, **kwargs):
        for f in self:
            f(*args, **kwargs)

class FiveBot:
    
    encoder_increments_per_turns = 3072
    wheel_radius = .047
    half_lengths = [ .15, .15 ]
    magic = 2900
    
    on_odometry = Event() # (x, y, a)
    on_wheel = Event() # (i, power, position, errors, speed)
    on_info = Event() # (msg)
    on_error = Event() # (self, msg)
    
    def __init__(self, tty, x = 0, y = 0, a = 0):
        self.com = serial.Serial(tty, baudrate=115200, stopbits=1)
        self.x, self.y, self.a = x, y, a
        self.thread = threading.Thread(target=self.__parse_commands_forever)
        self.thread.start()
    
    def __parse_commands_forever(self):
        while True:
            self.__parse_command()
    
    def __parse_command(self):
        cmd, packet = self.__read_packet()
        try:
            if cmd == "o":
                self.__publish_odometry(*struct.unpack("<fff", packet))
            elif cmd == "w":
                self.on_wheel(*struct.unpack("<bbhHf", packet))
            elif cmd == "i":
                self.on_info(packet.decode())
            else:
                raise Exception("unknown command: " + cmd)
        except Exception as e:
            self.on_error("cmd = %c, packet = [ %s ], %s" % (cmd, " ".join([ "%.2x" % byte for byte in packet ]), e))
    
    def __read_packet(self):
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
    
    def __send_command(self, cmd, fmt, *args):
        data = [ STX, ord(cmd) ]
        for byte in struct.pack(fmt, *args):
            if byte == STX or byte == ETX or byte == ESC:
                data += [ ESC ]
                byte = byte ^ ESC
            data += [ byte ]
        data += [ ETX ]
        self.com.write(bytes(data))
    
    def __publish_odometry(self, dx, dy, da):
        da *= 2 * math.pi * self.wheel_radius / 4 / sum(self.half_lengths) / self.magic
        dx *= self.wheel_radius * 2 * math.pi / self.encoder_increments_per_turns / 4
        dy *= self.wheel_radius * 2 * math.pi / self.encoder_increments_per_turns / 4
        c = math.cos(self.a + da / 2)
        s = math.sin(self.a + da / 2)
        dx, dy = c * dx + s * dy, -s * dx + c * dy
        self.x += dx
        self.y += dy
        self.a += da
        self.on_odometry(self.x, self.y, self.a)
    
    def set_speed(self, vx, vy, w, bypass_pid):
        self.vx = vx
        self.vy = vy
        self.w = w
        self.__send_command("s", "<fff?", vx, vy, w, bypass_pid)
