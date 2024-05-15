import random
import sys

list = list(range(1, 101))

random.shuffle(list)

with open("range_100", "w") as f:

    for i in range(1, 100):
        f.write("RANGE " + str(i) + " " + str(i+1) + "\n")




