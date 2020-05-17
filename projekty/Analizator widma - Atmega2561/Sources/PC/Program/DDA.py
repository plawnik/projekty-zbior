# coding=utf-8
# plot includes
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
#serial includes
import serial
import sys
import time
from sys import stdin
import atexit


# searching for all avilable com ports
def search_serial_ports():
    ports = ['COM%s' % (i + 1) for i in range(256)]
    found = []
    for port in ports:
        try:
            ser = serial.Serial(port)
            ser.close()
            found.append(port)
        except (OSError, serial.SerialException):
            pass
    return found


# receive data 
def receive_until(port,txt):
    d=""
    while d.find(txt) == -1:
        try:
            dnow=port.read(1)
            if len(dnow)==0:
                return
        except (OSError, serial.SerialException):
            return "Serial error"
        d= d+dnow.decode("utf-8")
    return d

 
def printtext(text):
    for c in text:
        sys.stdout.write(c)
        sys.stdout.flush()
       
 
# main program
# search com ports in system
com_ports = search_serial_ports()
print("Found "+str(len(com_ports))+" com ports!")
print("List:")
for c in com_ports:
    print(c)
# choose port    
print("Choose one of them(COM1,COM2,etc)")
com_number = stdin.readline()
com_number = com_number.rstrip()
com = serial.Serial(com_number,9600)
print("Listening on "+com_number)
# init graph
fig = plt.figure()
ax1=fig.add_subplot(1,1,1)


    
def animate(i):
    # receive data form analyzer
    receive_until(com,"START_SEND\n\r")
    tmp_str = receive_until(com,'AT+STOP')
    tmp_str = tmp_str[:-8]
    #print(tmp_str)
    y_axis_val=tmp_str.split(" ")
    try:
        y_axis_val=list(map(float,y_axis_val))
    except: 
        print("Frame receive error!")
    y_axis_val.pop(0)
    y_axis_val=y_axis_val[:-1]
    
    y_float=[]
    
    for c in y_axis_val:
        y_float.append(float(c*5/1024))
    y_axis_val=y_float
	
    # draw plot
    x_axis_val= list(range(100, 25600,100))
    
    try:
        ax1.clear()
        line, = ax1.plot(x_axis_val,y_axis_val)
    except:
        print("Draw error")
    #print(x_axis_val)



    

ani = animation.FuncAnimation(fig, animate, interval=500)
plt.show()
