#!/usr/bin/env python3
#! -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from sys import argv
from mpl_toolkits.mplot3d import Axes3D

if __name__ == "__main__":
   if len(argv) != 4:
      print("Usage: %s <1:input instance> <2:solution file> <3:plot type>" % argv[0])
      exit(1)
   
   x = None
   y = None
   e = None
   l = None

   # Reads some of instance data.
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

   nmark = ['s'] + [' ' for i in x[1:]]

   # Read the routes from the solution.
   routes = []
   with open(argv[2], "r") as fid:
      # Discards header.
      for i in range(3):
         fid.readline()

      for rlen in fid:
         rlen = int(rlen)
         r = []
         for i in range(rlen):
            tks = [int(x) for x in fid.readline().split()]
            r.append((tks[0], tks[1]))
            if tks[0] != 0:
               if nmark[tks[0]] == ' ':
                  nmark[tks[0]] = 'o'
               elif nmark[tks[0]] == 'o':
                  nmark[tks[0]] = '*'
               else:
                  nmark[tks[0]] = 't'

         routes.append(r)

   def plot2d():
      for v in range(len(routes)):
         # Prepares a new plot
         plt.figure(v)
         plt.title("[%s] Route for vehicle #%d - start from %d" % (argv[2], v, routes[v][1][0]))
         plt.grid()
         
         # Plot only the nodes.
         plt.plot(x[0], y[0], color='r', marker=nmark[0])
         plt.annotate("depot", (x[0], y[0]), fontsize=16.0)
         
         for i in range(1, len(x)-1):
            plt.plot(x[i], y[i], color='grey', marker=nmark[i])
            plt.annotate(str(i), (x[i], y[i]), fontsize=16.0)
         
         xlist = []
         ylist = []
         for (i,s) in routes[v]:
            xlist.append(x[i])
            ylist.append(y[i])
         plt.plot(xlist, ylist, alpha=0.6)

      plt.show()

   def plot3d():
      for v in range(len(routes)):
         # Prepares a new plot
         fig = plt.figure(v)
         ax = fig.gca(projection='3d')
         plt.title("[%s] Route for vehicle #%d - start from %d" % (argv[2], v, routes[v][1][0]))
         ax.set_xlabel('X')
         ax.set_ylabel('Y')
         ax.set_zlabel('TW (early)' if argv[3] == '3e' else 'TW (tardy)')

         def getz(i):
            if argv[3] == '3e':
               return e[i] if i != 0 else 0
            elif argv[3] == '3l':
               return e[i] if i != 0 else 0
            else:
               print("Unknown plot type: " + argv[3])
               exit(1)
         
         # Plot only the nodes.
         ax.plot([x[0]], [y[0]], [getz(0)], color='r', marker=nmark[0])
         ax.text(x[0], y[0], getz(0), "depot", fontsize=16.0)
         
         for i in range(1, len(x)-1):
            ax.plot([x[i]], [y[i]], [getz(i)], color='grey', marker=nmark[i])
            ax.text(x[i], y[i], getz(i), str(i), fontsize=16.0)
         
         xlist = []
         ylist = []
         zlist = []
         for (i,s) in routes[v]:
            xlist.append(x[i])
            ylist.append(y[i])
            zlist.append(getz(i))
         plt.plot(xlist, ylist, zlist, alpha=0.6)

         # for i in range(1, len(routes[v])):
         #    x0 = x[routes[v][i-1][0]]
         #    y0 = y[routes[v][i-1][0]]

         #    x1 = x[routes[v][i][0]]
         #    y1 = y[routes[v][i][0]]

         #    plt.quiver(x0, y0, x1, y1, scale_units='xy', angles='xy', scale=0.3)

      plt.show()

   if argv[3] == '2d':
      plot2d()
   else:
      plot3d()

