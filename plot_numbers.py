import matplotlib.pyplot as plt
import numpy as np
import os

with open('./output/array_timing.txt') as f:
    lines = f.readlines()
    numbers = [float(line.split()[1]) for line in lines]
    values = [int(line.split()[0]) for line in lines]
    
    plt.plot(numbers, values)
    #for x, y in zip(numbers, values):
    #    if y < 150:
    #        plt.annotate(f"{int(x)}", (x,y))
    plt.show()