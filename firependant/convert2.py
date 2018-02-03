# Inspired by https://github.com/adafruit/FirePendant

from PIL import Image
import sys

HEIGHT = 9
row = 0;

filename = sys.argv[1]

image = Image.open(filename)
if image.mode != 'L': # Not grayscale? Convert it
	image = image.convert("L")

image.pixels = image.load()

print (image.size)


sys.stdout.write("const uint8_t PROGMEM frames[%d][%d] = {\n" % (image.size[1]/HEIGHT, image.size[0]*HEIGHT));

row = 0
while(row < image.size[1]):
    sys.stdout.write("  {")
    for y in range(HEIGHT):
        for x in range(image.size[0]):
            n = image.pixels[x, row]
            sys.stdout.write("{0:#0{1}X},".format(n, 4))
        row = row+1
    sys.stdout.write("},\n")


sys.stdout.write("};\n");
