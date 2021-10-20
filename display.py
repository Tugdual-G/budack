#!/usr/bin/env python3
"""
display the content of binary files
"""

import numpy as np
import matplotlib.pyplot as plt

data = np.fromfile('array', dtype=np.ubyte)
print(data.shape)
data.shape = (1713, 2000)
data += data[::-1, :]
plt.imshow(data[4:-4, 4:-4])
plt.axis("image")
plt.axis("off")
plt.savefig("budac.png", bbox_inches='tight', dpi=600)
plt.show()
