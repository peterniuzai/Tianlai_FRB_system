import socket
import numpy as np
import struct

sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x3))
#sock = socket.socket(socket.PF_PACKET, socket.SOCK_RAW, socket.htons(0x800))
sock.bind(('p1p1', 0))
#sock.bind(('eth0', 0))
#sock.bind(('wlan0', 0))
#des_mac = '\x20\xcf\x30\x6f\x5d\x17' # My PC
#des_mac = '\x88\xae\x1d\xe3\x73\xa6' # My Laptop
#src_mac = '\x88\xae\x1d\xe3\x73\xa6' # My Laptop
#src_mac = '\x18\x03\x73\xB4\x2C\x05' # cosm2

def compDmac(pkt, destMAC):
    return pkt[:6] == destMAC

def compSmac(pkt, srcMAC):
    return pkt[6:12] == srcMAC

def showpkt(pkt):
    '''show packet in hex'''
    #print struct.unpack(len(pkt) * 'c', pkt)
    a = struct.unpack(len(pkt) * 'c', pkt)
    print fmt_mac(a[:6])
    print fmt_mac(a[6:12])
    print fmt_mac(a[12:14])
    print 'ipv4 (0x45)', map(hex, map(ord, a[14:15]))
    print 'service type', map(hex, map(ord, a[15:16]))
    print 'total length', map(hex, map(ord, a[16:18]))
#    print 'pass',         map(hex, map(ord, a[18:26]))
    print 'Protocol UDP', map(hex, map(ord, a[23:24]))
    print 'IP check sum', map(hex, map(ord, a[24:26]))
    print 'IP source:\t', map(ord, a[26:30])
    print 'IP destination:\t', map(ord, a[30:34])
    print 'UDP source Port', map(ord, a[34:36])
    print 'UDP destination Port', map(hex, map(ord, a[36:38]))
    print 'UDP pkt length', map(hex, map(ord, a[38:40]))
    print 'UDP check sum', map(hex, map(ord, a[40:42]))
    print '\n~~FRB data format~~\n'
    print 'FRB beam ID', map(hex, map(ord, a[42:43]))
    print 'FRB counter', map(hex, map(ord, a[43:50]))
    #print a[50:]

    return


 
def showpkt10(pkt):
    '''show packet in decimal'''
    #print struct.unpack(len(pkt) * 'c', pkt)
    #a = struct.unpack(len(pkt) * 'c', pkt)
    a = pkt
    #print 'MAC, Len:', struct.unpack('14c', a[:14])
    i = map(hex, map(ord, a[:6]))
    j = map(hex, map(ord, a[6:12]))
    mac_dst = '%s:%s:%s:%s:%s:%s' % (i[0][2:], i[1][2:], i[2][2:], i[3][2:], i[4][2:], i[5][2:])
    mac_src = '%s:%s:%s:%s:%s:%s' % (j[0][2:], j[1][2:], j[2][2:], j[3][2:], j[4][2:], j[5][2:])
    #if mac_dst.startswith('88:ae'): return
    #if mac_src.startswith('88:ae'): return
    print 'MAC Destination:', mac_dst
    print 'MAC Source:     ', mac_src
    #print struct.unpack('4B', a[14:18])
    #print struct.unpack('4B', a[18:22])
    #print struct.unpack('4B', a[22:26])
    print  'IP destination:\t', map(ord, a[26:30])
    print  'IP source:\t',      map(ord, a[30:34])
    #print struct.unpack(len(a[34:])*'c', a[34:])
    return
   
def fmt_mac(pkt):
    '''Format bin str to AA:BB:CC:DD:EE'''
    return (len(pkt) * '%s:' % tuple(map(hex, map(ord, pkt))))[:-1].replace('0x', '')

def showibpkt(pkt):
    '''Shwo InfiniBand Packet.'''
    #print 'MAC Dest:', map(hex, map(ord, pkt[:20]))
    #print 'MAC Src :', map(hex, map(ord, pkt[20:40]))
    print 'MAC Dest:', fmt_mac(pkt[:20])
    print 'MAC Src :', fmt_mac(pkt[20:40])
    return

#while 1:
for i in xrange(3):
    pkt = sock.recv(4200)
    print '== length = %d ==' % (len(pkt) - 14)
    #if pkt[:6] == '\xF0\x1F\xAF\x01\x29\x15':
    #if pkt[:6] == des_mac:
    #    showpkt(pkt[:70]
    #if pkt[:6] != 6*'\xFF':
    #    continue
    #showpkt10(pkt)
    print showpkt(pkt[:100])
    print '==Original packet length = %d ==' % (len(pkt))
    print '\n\n'
#    showibpkt(pkt)
    print



#    if (pkt[:6] == '\xff'*6) or (pkt[:6] == '\x00'*6):
#        #print '+++=++++'
#        #showpkt(pkt[:14])
#        continue
#    elif compDmac(pkt, des_mac) != True:
#        #print 'Received, but Destination MAC wrong. Dmac = ', showpkt(pkt[:6])
#        continue
#    elif compSmac(pkt, src_mac) != True:
#        #print 'Received, but Source MAC wrong. Smac = ', showpkt(pkt[6:12])
#        continue
#    else:
#        print '##############################'
#        showpkt(pkt[:100])
#        print '######## Len = %d ############' % len(pkt)





#sock = socket.socket(socket.PF_PACKET, socket.SOCK_RAW)



