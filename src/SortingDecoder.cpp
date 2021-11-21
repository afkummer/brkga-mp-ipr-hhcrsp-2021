/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021
 * Alberto Francisco Kummer Neto (afkneto@inf.ufrgs.br),
 * Luciana Salete Buriol (buriol@inf.ufrgs.br),
 * Olinto César Bassi de Araújo (olinto@ctism.ufsm.br) and
 * Mauricio G.C. Resende (resendem@amazon.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "SortingDecoder.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

using namespace std;

SortingDecoder::SortingDecoder(const Instance& inst_): inst(inst_) {
   // Pair the task lists with the chromosome keys.
   allTasks = createTaskList(inst);
   lexOrder.resize(allTasks.size());
   iota(lexOrder.begin(), lexOrder.end(), 0);
}

int SortingDecoder::chromosomeLength() const {
   // This new version of the algorithm uses a different layout for the chromosome.
   // Keys within [0, n-2) keys map to patient nodes of the problem.
   // Key n-2 encodes a boolean value to use or not the convex hull
   // algorithm within the Solution class.
   // Key n-1 indicates if the vehicle load heuristic should be applied or not.
   return (inst.numNodes()-2) + 1 + 1;
}

Solution SortingDecoder::decodeSolution(const std::vector<double> &chromosome) const {
   // Solution being build.
   Solution currSol(inst);
   currSol.convHull = chromosome[chromosome.size()-2] >= 0.5;
   bool enableHeur = chromosome[chromosome.size()-1] >= 0.5;

   // Create the entire assignment order using random keys from chromossome.
   assert(chromosomeLength() == static_cast<int>(chromosome.size()) &&
      "Chromossome not long enough to support the sorting procedure.");

   // Takes the lexOrder as template, and the sorts a copy of
   // the indirect index vector.
   auto taskIndices = lexOrder;
   sort(begin(taskIndices), end(taskIndices), [&](int i, int j) {
      return chromosome[i] < chromosome[j];
   });

   // Implements an heuristic that balances the workload among vehicles.
   // This very simple implementation does not seems to make a great difference
   // in the vehicles workload, but is has a secondary function of tie-breaking
   // routes of vehicles that are much similar (regarding their qualifications).
   vector <double> wtime(inst.numVehicles(), 0.0);
   auto heur = [&] (Task &t) {
      currSol.findInsertionCost(t);
      if (enableHeur) {
         double w = wtime[t.vehi[0]] + t.skills[1] >= 0 ? wtime[t.vehi[1]] : 0.0;
         t.cachedCost += enableHeur*w;
      }
      return t.cachedCost;
   };

   auto accept = [&] (Task &t) {
      wtime[t.vehi[0]] += inst.nodeProcTime(t.node, t.skills[0]);
      if (t.vehi[1] >= 0)
         wtime[t.vehi[1]] += inst.nodeProcTime(t.node, t.skills[1]);
      currSol.updateRoutes(t);
   };

   for (size_t i = 0; i < taskIndices.size(); ++i) {
      Task task = allTasks[taskIndices[i]];
      if (verbose)
       cout << "Finding best assignment to node " << task.node << "...\n";

      Task best = task;
      best.cachedCost = 1e6;

      assert(task.skills[0] != -1 && "First service type is unset.");

      for (int v0: inst.qualifiedVehicles(task.skills[0])) {

         task.vehi[0] = v0;

         if (inst.nodeSvcType(task.node) == Instance::SvcType::SINGLE) {
            if (heur(task) <= best.cachedCost) {
               if (task.cachedCost < best.cachedCost) {
                  best = task;
               } else {
                  if (chromosome[i] >= 0.5)
                     best = task;
               }
            }

         } else {
            assert(task.skills[1] != -1 && "Second service type is unset.");
            for (int v1: inst.qualifiedVehicles(task.skills[1])) {
               if (v0 == v1)
                  continue;

               task.vehi[1] = v1;

               if (heur(task) <= best.cachedCost) {
                  if (task.cachedCost < best.cachedCost) {
                     best = task;
                  } else {
                     if (chromosome[i] >= 0.5)
                        best = task;
                  }
               }
            }
         }
      }

      // Update the current solution.
      accept(best);
   }

   // Return nodes to depot.
   currSol.finishRoutes();

   return currSol;
}

double SortingDecoder::decode(const std::vector<double> &chromosome, bool rewrite) const {
   (void) rewrite;
   return decodeSolution(chromosome).cachedCost;
}
