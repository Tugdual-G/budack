#!/usr/bin/env python3
"""
whatisthisthing
"""

import numpy as np
import matplotlib.pyplot as plt

M = np.fromfile('M.o', dtype=np.intc)
M.shape = (int(len(M)/1000), 1000)
Mb = np.fromfile('M_b.o', dtype=np.intc)
x = Mb[1::2]
y = Mb[0::2]
print(len(y), len(x))
plt.pcolormesh(M)
plt.scatter(x, y, c='r', marker='.')
plt.show()
