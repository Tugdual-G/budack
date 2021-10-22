#!/usr/bin/env python3
"""
display the content of binary files
"""

import numpy as np
import matplotlib.pyplot as plt

size = np.fromfile('arraysize.o', dtype=np.intc)
data = np.fromfile('trajectories.o', dtype=np.uintc)
print(data)
data.shape = (size[0], size[1])
data += data[::-1, :]
plt.imshow(data[8:-8, 8:-8], cmap='inferno')
plt.axis("image")
plt.axis("off")
plt.savefig("budac.png", bbox_inches='tight', dpi=600)
plt.show()
