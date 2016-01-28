import math
import serial
import struct
import threading
import time

STX = 2
ETX = 3
ESC = 27

class Event(list):
    def __call__(self, *args, **kwargs):
        for f in self:
            f(*args, **kwargs)

class Car:
    
    encoder_increments_per_turns = 3072
    wheel_radius = .047
    half_lengths = [ .15, .15 ]
    magic = 2900
    max_speed = 2 * (2 * math.pi)
    
    on_odometry = Event() # (x, y, a)
    on_wheel = Event() # (i, power, position, errors, speed)
    on_info = Event() # (msg)
    on_error = Event() # (self, msg)
    
    def __init__(self, tty, x = 0, y = 0, a = 0):
        self.com = serial.Serial(tty, baudrate=115200, stopbits=1)
        self.x, self.y, self.a = x, y, a
        self.on_info.append(print)
        self.on_error.append(print)
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
        self.vx = min(max(vx, -self.max_speed), self.max_speed)
        self.vy = min(max(vy, -self.max_speed), self.max_speed)
        self.w = min(max(w, -self.max_speed), self.max_speed)
        self.__send_command("s", "<fff?", vx, vy, w, bypass_pid)

class PositionController:
    
    error = [ 0, 0, 0 ]
    max_error = 10
    last_odometry = None
    update_interval = 0.02
    last_update_time = 0
    
    def __init__(self, car, p, i, d):
        self.car = car
        self.set_pid(p, i, d)
    
    def set_pid(self, p, i, d):
        self.p, self.i, self.d = p, i, d
    
    def set_target(self, x, y, a):
        self.target = [ x, y, a ]
        self.error = [ 0, 0, 0 ]
    
    def start(self):
        self.car.on_odometry.append(self.__on_odometry)
    
    def stop(self):
        self.car.on_odometry.remove(self.__on_odometry)
    
    def __on_odometry(self, x, y, a):
        try:
            now = time.time()
            if now - self.last_update_time < self.update_interval:
                return
            self.last_update_time = now
            odometry = [ x, y, a ]
            print(odometry) # XXX
            if self.last_odometry is None:
                self.last_odometry = odometry
            error = [ self.target[k] - odometry[k] for k in range(3) ]
            cmd = [ 0, 0, 0 ]
            for k in range(3):
                    p = self.p * error[k]
                    self.error[k] = min(max(self.error[k] + error[k], -self.max_error), self.max_error)
                    i = self.i * self.error[k]
                    d = self.d * self.error[k]
                    cmd[k] = p + i + d
            self.last_odometry = odometry
            self.car.set_speed(*cmd, bypass_pid=False)
            print(cmd) # XXX
            print() # XXX
        except Exception as e:
            print(e)
