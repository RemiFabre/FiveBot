#!/usr/bin/env python3

import serial
import struct
import sys
import threading
import tkinter as tk

# Create window
root = tk.Tk()
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)
pad = { "padx": 5, "pady": 5 }
root.wm_title("FiveBot Control")
frame = tk.Frame(root)
frame.grid(**pad)

# Create odometry box
odoBox = tk.LabelFrame(frame, text="Odometry", **pad)
odoBox.grid(columnspan=2, **pad)
odoX = tk.Label(odoBox, text="X", **pad)
odoY = tk.Label(odoBox, text="Y", **pad)
odoA = tk.Label(odoBox, text="Angle", **pad)
odoX.grid(row=0, column=0)
odoY.grid(row=0, column=1)
odoA.grid(row=0, column=2)
odoX = tk.Label(odoBox, width=10, **pad)
odoY = tk.Label(odoBox, width=10, **pad)
odoA = tk.Label(odoBox, width=10, **pad)
odoX.grid(row=1, column=0)
odoY.grid(row=1, column=1)
odoA.grid(row=1, column=2)

# Create motor box
motorBox = tk.LabelFrame(frame, text="Motors", **pad)
motorBox.grid(row=1, column=0, **pad)
tk.Label(motorBox, text="Power", **pad).grid(row=1, column=0)
tk.Label(motorBox, text="Speed", **pad).grid(row=2, column=0)
motorMeters = []
for i in range(4):
    label = tk.Label(motorBox, text="M"+str(i), **pad)
    label.grid(row=0, column=i+1)
    meter1 = tk.Label(motorBox, width=5, **pad)
    meter1.grid(row=1, column=i+1)
    meter2 = tk.Label(motorBox, width=5, **pad)
    meter2.grid(row=2, column=i+1)
    motorMeters.append([meter1, meter2])

# Create encoder box
encoderBox = tk.LabelFrame(frame, text="Encoders", **pad)
encoderBox.grid(row=1, column=1, **pad)
tk.Label(encoderBox, text="Delta", **pad).grid(row=1, column=0)
tk.Label(encoderBox, text="Errors", **pad).grid(row=2, column=0)
encoderMeters = []
for i in range(4):
    label = tk.Label(encoderBox, text="M"+str(i), **pad)
    label.grid(row=0, column=i+1)
    meter1 = tk.Label(encoderBox, width=5, **pad)
    meter1.grid(row=1, column=i+1)
    meter2 = tk.Label(encoderBox, width=5, **pad)
    meter2.grid(row=2, column=i+1)
    encoderMeters.append([meter1, meter2])

# Create info box
infoBox = tk.LabelFrame(frame, text="Info", **pad)
infoBox.grid(columnspan=2, **pad)
infoBoxScroll = tk.Scrollbar(infoBox)
infoBoxScroll.grid(row=0, column=1, sticky="ns")
infoBoxText = tk.Listbox(infoBox, width=70, yscrollcommand=infoBoxScroll.set)
infoBoxText.grid(row=0)
infoBoxScroll.config(command=infoBoxText.yview)

class BoardCom(threading.Thread):
    
    def run(self):
        try:
            self.com = serial.Serial(sys.argv[1], baudrate=2000000, stopbits=2)
        except Exception as e:
            self.echo(e)
            raise e
        while True:
            try:
                self.com.readline()
                cmd = self.com.read().decode("ascii")
                getattr(self, "cmd_" + cmd)()
            except Exception as e:
                self.echo(e)
    
    def echo(self, msg):
        infoBoxText.insert(tk.END, msg)
        infoBoxText.see(tk.END)
    
    def read_packet(self, fmt):
        fmt = "<" + fmt
        return struct.unpack(fmt, self.com.read(struct.calcsize(fmt)))
    
    def cmd_o(self):
        x, y, z = self.read_packet("fff")
        R = 9.4 / 2 / 100
        L = 15 / 100
        x *= R / 4
        y *= R / 4
        z *= R / 4 / (L + L)
        fmt = "%.2f"
        x, y, z = fmt % x, fmt % y, fmt % z
        odoX["text"], odoY["text"], odoA["text"] = x, y, z
    
    def cmd_w(self, *power):
        for i in range(4):
            power, position, errors, speed = self.read_packet("hhhf")
            motorMeters[i][0]["text"] = "%d%%" % (power * 100 / 127)
            motorMeters[i][1]["text"] = "%.2f" % speed
            encoderMeters[i][0]["text"] = position
            encoderMeters[i][1]["text"] = errors

BoardCom().start()
root.mainloop()
