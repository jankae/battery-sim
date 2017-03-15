#!/usr/bin/python

import pygame
import sys
import serial

ser = serial.Serial('/dev/ttyACM0')

pygame.init()
screen = pygame.display.set_mode((320,240))
surface = pygame.display.get_surface()

sx = 0
sy = 0
ex = 320
ey = 240

x = 0
y = 0
count = 0
while True:
	event = pygame.event.poll()
        if event.type == pygame.QUIT:
        	running = 0
     	elif event.type == pygame.MOUSEBUTTONDOWN:
         	print "You pressed the mouse button at (%d, %d)" % event.pos
		# send event 
		x, y = event.pos
		ser.write(bytearray([0x01, x/256, x%256, y/256, y%256]))
     	elif event.type == pygame.MOUSEBUTTONUP:
         	print "You released the mouse button at (%d, %d)" % event.pos
		# send event 
		x, y = event.pos
		ser.write(bytearray([0x02, x/256, x%256, y/256, y%256]))
	if(ser.inWaiting()>0):
		input = ser.readline();
		while(len(input)>=3):
			#print len(input)
			value = ord(input[1])*256+ord(input[2])
			if(ord(input[0])==0):
				sx = value
				#print "Start x: %d" % value
			elif(ord(input[0])==1):
				sy = value
				#print "Start y %d" % value
			elif(ord(input[0])==2):
				ex = value
				#print "Stop y %d" % value
			elif(ord(input[0])==3):
				ey = value
				#print "Stop y %d" % value
			if(ord(input[0])==4):
				#print "Color %d" % value
				# transform value into color
				r = int(value / 2048);
				value -= r * 2048;
				g = int(value / 32);
				value -= g * 32;
				b = value
				r = r * 8
				g = g * 4
				b = b * 8
				surface.set_at((x, y), (r, g, b))
				x = x + 1
				if(x>ex):
					x = sx
					y = y + 1
					if(y>ey):
						y = sy
			else:
				x = sx
				y = sy
			input = input[3:]
		pygame.display.update()

		
