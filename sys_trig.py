import casperfpga
import time
import sys
#import matplotlib.pyplot as plt

def rawtm2tm(v):
    '''Convert Raw time format to correct time.
       Raw time is expressed by 2 bytes hex string.'''
    s = hex(v).replace('0x', '000')[-4:]
    dv = map(int, [s[:2], s[2:]])
    return 10*dv[0] + dv[1]

board1=casperfpga.CasperFpga('100.0.0.3')
board2=casperfpga.CasperFpga('100.0.0.4')

if(board1.read_uint('adc_valid')==1 and board2.read_uint('adc_valid')==1):
       Year	= rawtm2tm(board1.read_uint('gps_year'))
       Month	= rawtm2tm(board1.read_uint('gps_month'))
       Day	= rawtm2tm(board1.read_uint('gps_day'))
       Hour	= rawtm2tm(board1.read_uint('gps_hour'))
       Min	= rawtm2tm(board1.read_uint('gps_min'))
       Sec	= rawtm2tm(board1.read_uint('gps_sec'))
       Msec	= board1.read_uint('gps_cnt_in_sec')*4/1000000
       Usec	= int((board1.read_uint('gps_cnt_in_sec')*4%1000000)/1000)
       f = open('/FRBTMPFS/GPS_File.txt','wb')
       f.write('20' + bytes(Year)+'\n')
       f.write(bytes(Month)+'\n')
       f.write(bytes(Day)+'\n')
       f.write(bytes(Hour)+'\n')
       f.write(bytes(Min)+'\n')
       f.write(bytes(Sec)+'\n')
       f.write(bytes(Msec)+'\n')
       f.write(bytes(Usec)+'\n')
       f.close()
	


else:
       print('adc is not ready!')
