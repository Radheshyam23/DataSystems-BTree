import random
import sys

list = list(range(1, 101))

random.shuffle(list)

with open("insert_rand_100", "w") as f:
    for item in list:
        f.write("INSERT " + str(item) + "\n")




