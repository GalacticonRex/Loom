import sys

if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise ValueError

    count = int(sys.argv[1])
    fname = sys.argv[2]
    with open(fname, "rb") as f:
        ilist = []
        for _ in range(count):
            b = f.read(8)
            ilist.append(int.from_bytes(b, byteorder='little', signed=False))
        print(ilist)