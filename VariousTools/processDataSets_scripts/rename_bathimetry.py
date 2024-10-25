import os

for f in os.listdir("."):
	if f.endswith(".Bathymetry"):
		fname=chr(ord(f[4])-32) + f[5:7] + chr(ord(f[0])-32) + f[1:4] + ".hgt"
		os.rename(f,fname)
