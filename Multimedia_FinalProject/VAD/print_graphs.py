import numpy as np
import matplotlib.pyplot as plt

number = input("Enter a number ")
file1 = "denoised_input/inputaudio"+number+".data"
file2 = "../inputdatabase/inputaudio"+number+".data"

a = np.fromfile(file1, dtype=np.int8)
b = np.fromfile(file2, dtype=np.int8)

plt.plot(a)
plt.plot(b)
plt.xlabel('Time')
plt.show()
