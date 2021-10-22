#!/usr/bin/env python3
"""
display the content of binary files
"""

import numpy as np
import matplotlib.pyplot as plt

Mb = np.fromfile('boundary.data', dtype=np.uintc)
x = Mb[1::2]
y = Mb[0::2]

size = np.fromfile('arraysize.data', dtype=np.intc)
data = np.fromfile('trajectories.data', dtype=np.uintc)
print(data)
data.shape = (size[0], size[1])
y = y + size[0]/2
data += data[::-1, :]
plt.imshow(data[8:-8, 8:-8], cmap='inferno')
plt.axis("image")
plt.axis("off")
plt.savefig("budac.png", bbox_inches='tight', dpi=600)
plt.scatter(x, y, c='r', marker='.', s=0.1)
plt.show()
