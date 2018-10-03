import random
import sys

if __name__ == "__main__":

    if len(sys.argv) != 3:
        raise ValueError

    count = int(sys.argv[1])
    fname = sys.argv[2]

    buf = []
    for i in range(count):
        buf += [random.randint(0, 255), 0, 0, 0, 0, 0, 0, 0]

    with open(fname, "wb") as f:
        f.write(bytearray(buf))