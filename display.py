#!/usr/bin/env python3
"""
display the content of binary files
"""

import numpy as np
import matplotlib.pyplot as plt

size = np.fromfile('arraysize', dtype=np.intc)
data = np.fromfile('array', dtype=np.intc)
print(data.shape)
data.shape = (size[0], size[1])
data += data[::-1, :]
plt.imshow(data[8:-8, 8:-8])
plt.axis("image")
plt.axis("off")
plt.savefig("budac.png", bbox_inches='tight', dpi=600)
plt.show()
