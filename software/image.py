#!/usr/bin/python

from __future__ import print_function
from PIL import Image
import sys

im = Image.open(sys.argv[1])

width, height = im.size

print ("const uint16_t imagedata[%s] = {" % (width*height))

cnt = 0

for pixel in iter(im.getdata()):
	r, g, b, a = pixel;
	r = r / 8
	g = g / 4
	b = b / 8
	color = r*2048+g*32+b
	print ("0x%04x," % color, end="")
	cnt = cnt + 1
	if(cnt==width):
		cnt = 0
		print ("")
print ("};")
