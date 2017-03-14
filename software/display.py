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
	input = ser.readline();
	if(len(input)<3):
		continue
	value = ord(input[1])*256+ord(input[2])
	if(ord(input[0])==0):
		sx = value
	if(ord(input[0])==1):
		sy = value
	if(ord(input[0])==2):
		ex = value
	if(ord(input[0])==3):
		ey = value
	if(ord(input[0])==4):
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
		if(x==ex):
			x = sx
			y = y + 1
			if(y==ey):
				y = sy
		count = count + 1
		if(count==320):
			pygame.display.update()
			count = 0
	else:
		x = sx
		y = sy

