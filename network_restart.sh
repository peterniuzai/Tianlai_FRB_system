#!/bin/bash
# THIS FILE IS ADDED FOR COMPATIBILITY PURPOSES
#
# It is highly advisable to create own systemd services or udev rules
# to run scripts during boot instead of using this file.
#
# In contrast to previous versions due to parallel execution during boot
# this script will NOT be run after all other services.
#
# Please note that you must run 'chmod +x /etc/rc.d/rc.local' to ensure
# that this script will be executed during boot.

#touch /var/lock/subsys/local

ifconfig p2p1 down
ifconfig p2p1 hw ether 02:02:0a:00:00:11
ifconfig p2p1 up
ifconfig p2p2 down
ifconfig p2p2 hw ether 02:02:0a:00:00:12
ifconfig p2p2 up
ifconfig p5p1 down
ifconfig p5p1 hw ether 02:02:0a:00:00:13
ifconfig p5p1 up
ifconfig p5p2 down
ifconfig p5p2 hw ether 02:02:0a:00:00:14
ifconfig p5p2 up

#ifconfig p1p1 promisc
#ifconfig p1p2 promisc
#ifconfig p4p1 promisc
#ifconfig p4p2 promisc

#mount /dev/sda1 /data0
#mount /dev/sdb1 /data1
#mount -t tmpfs -o size=80G tmpfs /FRBTMPFS

