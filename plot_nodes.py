#!/usr/bin/env python3
#! -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from sys import argv

if __name__ == "__main__":
   if len(argv) != 2:
      print("Usage: %s <1:input instance>" % argv[0])
      exit(1)
   
   x = None
   y = None
   e = None
   l = None

   with open(argv[1], "r") as fid:
      for line in fid:
         if line.strip() == "x":
            x = [int(x.strip()) for x in fid.readline().split()]
         if line.strip() == "y":
            y = [int(x.strip()) for x in fid.readline().split()]
         if line.strip() == "e":
            e = [int(x.strip()) for x in fid.readline().split()]
         if line.strip() == "l":
            l = [int(x.strip()) for x in fid.readline().split()]

   plt.plot(x[0], y[0], color='r', marker='s')
   plt.annotate("depot", (x[0], y[0]), fontsize=16.0)
   
   for i in range(1, len(x)-1):
      plt.plot(x[i], y[i], color='grey', marker='o')
      plt.annotate(str(i), (x[i], y[i]), fontsize=16.0)

   plt.show()

