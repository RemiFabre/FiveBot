#!/usr/bin/env python3

import sys
import threading
import time
import tkinter as tk

import boardcom

class BoardGui:
    
    def run(self):
        self.create_gui()
        self.com = boardcom.BoardCom(sys.argv[1])
        self.com.on_odometry = self.on_odometry
        self.com.on_wheel = self.on_wheel
        self.com.on_info = self.on_info
        self.com.on_error = self.on_error
        self.startTime = time.time()
        threading.Thread(target=self.com.run).start()
        tk.mainloop()
    
    def create_gui(self):
        self.create_window()
        self.create_odometry_box()
        self.create_speed_box()
        self.create_motor_box()
        self.create_encoder_box()
        self.create_info_box()
        self.create_key_bindings()
    
    def create_window(self):
        self.root = tk.Tk()
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        self.pad = { "padx": 5, "pady": 5 }
        self.root.wm_title("FiveBot Control")
        self.frame = tk.Frame(self.root)
        self.frame.grid(**self.pad)
    
    def create_odometry_box(self):
        odoBox = tk.LabelFrame(self.frame, text="Odometry", **self.pad)
        odoBox.grid(row=0, column=0, **self.pad)
        self.odoX = tk.Label(odoBox, text="X", **self.pad)
        self.odoY = tk.Label(odoBox, text="Y", **self.pad)
        self.odoW = tk.Label(odoBox, text="Angle", **self.pad)
        self.odoX.grid(row=0, column=0)
        self.odoY.grid(row=0, column=1)
        self.odoW.grid(row=0, column=2)
        self.odoX = tk.Label(odoBox, width=8, **self.pad)
        self.odoY = tk.Label(odoBox, width=8, **self.pad)
        self.odoW = tk.Label(odoBox, width=8, **self.pad)
        self.odoX.grid(row=1, column=0)
        self.odoY.grid(row=1, column=1)
        self.odoW.grid(row=1, column=2)
    
    def create_speed_box(self):
        speedBox = tk.LabelFrame(self.frame, text="Speed", **self.pad)
        speedBox.grid(row=0, column=1, **self.pad)
        names = [ "Vx", "Vy", "ω" ]
        self.speedCtrl = []
        for i in range(3):
            label = tk.Label(speedBox, text="0")
            label.grid(row=0, column=i, **self.pad)
            entry = tk.Entry(speedBox, width=3)
            entry.grid(row=1, column=i, **self.pad)
            entry.insert(0, "0")
            self.speedCtrl.append(entry)
        self.pid = tk.IntVar()
        self.pid.set(1)
        pid = tk.Checkbutton(speedBox, text="PID", variable=self.pid)
        pid.grid(row=0, column=3, **self.pad)
        btn = tk.Button(speedBox, command=self.set_speed, text="Set", **self.pad)
        btn.grid(row=1, column=3, **self.pad)
    
    def create_motor_box(self):
        motorBox = tk.LabelFrame(self.frame, text="Motors", **self.pad)
        motorBox.grid(row=1, column=0, **self.pad)
        tk.Label(motorBox, text="Power", **self.pad).grid(row=1, column=0)
        tk.Label(motorBox, text="Speed", **self.pad).grid(row=2, column=0)
        self.motorMeters = []
        for i in range(4):
            label = tk.Label(motorBox, text="M"+str(i), **self.pad)
            label.grid(row=0, column=i+1)
            meter1 = tk.Label(motorBox, width=5, **self.pad)
            meter1.grid(row=1, column=i+1)
            meter2 = tk.Label(motorBox, width=5, **self.pad)
            meter2.grid(row=2, column=i+1)
            self.motorMeters.append([meter1, meter2])
    
    def create_encoder_box(self):
        encoderBox = tk.LabelFrame(self.frame, text="Encoders", **self.pad)
        encoderBox.grid(row=1, column=1, **self.pad)
        tk.Label(encoderBox, text="Delta", **self.pad).grid(row=1, column=0)
        tk.Label(encoderBox, text="Errors", **self.pad).grid(row=2, column=0)
        self.encoderMeters = []
        for i in range(4):
            label = tk.Label(encoderBox, text="M"+str(i), **self.pad)
            label.grid(row=0, column=i+1)
            meter1 = tk.Label(encoderBox, width=5, **self.pad)
            meter1.grid(row=1, column=i+1)
            meter2 = tk.Label(encoderBox, width=5, **self.pad)
            meter2.grid(row=2, column=i+1)
            self.encoderMeters.append([meter1, meter2])
    
    def create_info_box(self):
        infoBox = tk.LabelFrame(self.frame, text="Info", **self.pad)
        infoBox.grid(columnspan=2, **self.pad)
        infoBoxScroll = tk.Scrollbar(infoBox)
        infoBoxScroll.grid(row=0, column=1, sticky="ns")
        self.infoBoxText = tk.Listbox(infoBox, width=70, yscrollcommand=infoBoxScroll.set)
        self.infoBoxText.grid(row=0)
        infoBoxScroll.config(command=self.infoBoxText.yview)
    
    def create_key_bindings(self):
        def forward(event):
            self.com.send_speed(10, 0, 0, False)
        def backward(event):
            self.com.send_speed(-10, 0, 0, False)
        def left(event):
            self.com.send_speed(0, 0, 10, False)
        def right(event):
            self.com.send_speed(0, 0, -10, False)
        def stop(event):
            self.com.send_speed(0, 0, 0, False)
        self.root.bind("<Z>", forward)
        self.root.bind("<S>", backward)
        self.root.bind("<Q>", left)
        self.root.bind("<D>", right)
        self.root.bind("<KeyRelease-Shift_L>", stop)
    
    def echo(self, msg):
        self.infoBoxText.insert(tk.END, "%d %s" % ((time.time() - self.startTime) * 1000, msg))
        self.infoBoxText.see(tk.END)
    
    def on_odometry(self, x, y, w):
        R = 9.4 / 2 / 100
        L = 15 / 100
        x *= R / 4
        y *= R / 4
        w *= R / 4 / (L + L) / 2900 # 2900 units in a cycle
        fmt = "%.2f"
        x, y, w = fmt % x, fmt % y, fmt % w
        self.odoX["text"], self.odoY["text"], self.odoW["text"] = x, y, w
    
    def on_wheel(self, i, power, position, errors, speed):
        self.motorMeters[i][0]["text"] = "%d%%" % (power * 100 / 127)
        self.motorMeters[i][1]["text"] = "%.2f" % speed
        self.encoderMeters[i][0]["text"] = position
        self.encoderMeters[i][1]["text"] = errors
    
    def on_info(self, msg):
        self.echo(msg)
    
    def on_error(self, msg):
        self.echo(msg)
    
    def set_speed(self):
        vx, vy, vz = [ float(entry.get()) for entry in self.speedCtrl ]
        self.com.send_speed(vx, vy, vz, not self.pid.get())

if __name__ == "__main__":
    BoardGui().run()
