import time

start = time.time()

def getInt(file):
    return int.from_bytes(file.read(8), byteorder='little', signed=False)

count = 1048576

fileA = open("bufferA.bin", "rb")
fileB = open("bufferB.bin", "rb")

listA = [getInt(fileA) for _ in range(count)]
listB = [getInt(fileB) for _ in range(count)]

fileA.close()
fileB.close()

listC = [0] * count

for i in range(count):
    listC[i] = listA[i] + listB[i]

fileC = open("bufferC.bin", "wb")
for i in listC:
    fileC.write(int.to_bytes(i, 8, byteorder='little', signed=False))
fileC.close()

end = time.time()
print(end-start)