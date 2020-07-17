#! /usr/bin/python
#import corr,socket,array
#import socket,pylab,matplotlib,math,corr,array
import socket
import struct
import time
import numpy as np


if __name__ == '__main__':
    IP = "10.0.0.17" #bind on IP addresses
    PORT = 10000
    counter = 0
    file_name = "TL-test.dat"
    #sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, )
    #sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.IPPROTO_RAW)
    sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x800))
    #sock.setopt()
    sock.bind(('p1p1', 0))
    #sock.bind((IP, PORT))
    if PORT != -1:
        print "10GbE port connect done!"
    #data, addr = sock.recvfrom(4096+8)
    addr = sock.recv(4096+8)
    print 'done?'
    print addr
    exit()
    #header = struct.unpack('<Q', data[0:8])[0]
    for i in range(100):
        #while(1):
            data, addr = sock.recvfrom(4104)
            print "ok Receiv"
            np.save(f,data)
            if len(data) != 4104:
            	counter += 1
            	print "See !"
            print 'counter:', counter
            print 'received',len(data),'bytes'
            print 'from', addr
		
