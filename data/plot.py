import sys
import matplotlib.pyplot as plt
import numpy as np

file = sys.argv[1]
fileinfo = file.split("_")
fanout = fileinfo[1]

heap = {}
bptree = {}

with open(file) as f:
    for i in f:
        blockaccesscount = i.split()

        if blockaccesscount[0] not in bptree.keys():
            bptree[blockaccesscount[0]] = 0

        if blockaccesscount[1] not in heap.keys():
            heap[blockaccesscount[1]] = 0

        bptree[blockaccesscount[0]] += 1
        heap[blockaccesscount[1]] += 1

    bptree_counts = dict(sorted(bptree.items()))
    heap_counts = dict(sorted(heap.items()))

    plt.title('Block Access frequency when FANOUT = ' + fanout)
    plt.xlabel('Number of Block Accesses')
    plt.ylabel('Count')
    plt.bar(list(bptree_counts.keys()), bptree_counts.values(), label='With B+ Tree')
    plt.bar(list(heap_counts.keys()), heap_counts.values(), label='Without B+ Tree')
    plt.legend()
    plt.show()





