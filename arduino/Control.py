#!/usr/bin/env python3

import serial
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
odoBox.grid(**pad)
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

# Create speed box
speedBox = tk.LabelFrame(frame, text="Speeds (rad/s)", **pad)
speedBox.grid(**pad)
speedometer = []
for i in range(4):
    label = tk.Label(speedBox, text="M"+str(i), **pad)
    label.grid(row=0, column=i)
    meter = tk.Label(speedBox, width=5, **pad)
    meter.grid(row=1, column=i)
    speedometer.append(meter)

# Create encoder box
encoderBox = tk.LabelFrame(frame, text="Encoder buffers & errors", **pad)
encoderBox.grid(**pad)
encoderMeters = []
for i in range(4):
    label = tk.Label(encoderBox, text="M"+str(i), **pad)
    label.grid(row=0, column=i)
    meter1 = tk.Label(encoderBox, width=5, **pad)
    meter1.grid(row=1, column=i)
    meter2 = tk.Label(encoderBox, width=5, **pad)
    meter2.grid(row=2, column=i)
    encoderMeters.append([meter1, meter2])

# Create info box
infoBox = tk.LabelFrame(frame, text="Info", **pad)
infoBox.grid(**pad)
infoBoxScroll = tk.Scrollbar(infoBox)
infoBoxScroll.grid(row=0, column=1, sticky="ns")
infoBoxText = tk.Listbox(infoBox, width=70, yscrollcommand=infoBoxScroll.set)
infoBoxText.grid(row=0)
infoBoxScroll.config(command=infoBoxText.yview)

class BoardCom(threading.Thread):
    
    def run(self):
        self.connect_to_board()
        self.read_from_board()
    
    def connect_to_board(self):
        self.com = serial.Serial(sys.argv[1], baudrate=2000000, stopbits=2)
        while self.com.read() != b"s":
            pass
        assert "s" + self.com.readline().decode("ascii").strip() == "setup"
    
    def read_from_board(self):
        while True:
            args = self.com.readline().decode("ascii").strip().split("|")
            print(args)
            cmd = args.pop(0)
            try:
                cmd = getattr(self, "cmd_" + cmd)
            except AttributeError as e:
                self.cmd_(cmd, *args)
                continue
            cmd(*args)
    
    def cmd_(self, *text):
        if len(text) > 0:
            infoBoxText.insert(tk.END, text)
            infoBoxText.see(tk.END)
    
    def cmd_odometry(self, x, y, a):
        odoX["text"] = x
        odoY["text"] = y
        odoA["text"] = a
    
    def cmd_speeds(self, *speeds):
        for i in range(4):
            speedometer[i]["text"] = speeds[i]
    
    def cmd_positions(self, *positions):
        for i in range(4):
            encoderMeters[i][0]["text"] = positions[i]
    
    def cmd_errors(self, *errors):
        for i in range(4):
            encoderMeters[i][1]["text"] = errors[i]

BoardCom().start()
root.mainloop()
