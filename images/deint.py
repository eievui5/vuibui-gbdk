import sys

input = open(sys.argv[1], "rb").read()
even = True
output = []
for byte in input:
	if not even:
		output.append(byte)
	even = not even
print(output)
open("out.bin", "wb").write(bytearray(output))
