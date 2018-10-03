import sys
import math
import time

def isPrime(v):
    if v < 2:
        return False
    sq = int(math.sqrt(v))+1
    for i in range(2,sq):
        if v % i == 0:
            return False
    return True

if __name__ == "__main__":
    start = time.time()
    count = int(sys.argv[1])
    total = 0
    for i in range(2, count+1):
        if isPrime(i):
            total += 1
    print("There are", total, "primes less than", count)
    end = time.time() - start
    print("Execution took", str(end), "seconds")