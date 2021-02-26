/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020
 * Alberto Francisco Kummer Neto (afkneto@inf.ufrgs.br),
 * Luciana Salete Buriol (buriol@inf.ufrgs.br) and
 * Olinto César Bassi de Araújo (olinto@ctism.ufsm.br)
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

using namespace std;


SortingDecoder::SortingDecoder(const Instance& inst_): inst(inst_) {
   vehiSkills.resize(inst.numSkills());
   for (int s = 0; s < inst.numSkills(); ++s) {
      for (int v = 0; v < inst.numVehicles(); ++v) {
         if (inst.vehicleHasSkill(v, s)) {
            vehiSkills[s].push_back(v);
         }
      }
   }

   // Pair the task lists with the chromosome keys.
   initialTasks.resize(chromosomeLength());
   {
      vector <Task> tl = createTaskList(inst);
      for (size_t i = 0; i < tl.size(); ++i) {
         get<0>(initialTasks[i]) = i/1000.0;
         get<1>(initialTasks[i]) = tl[i];
      }
   }
}

int SortingDecoder::chromosomeLength() const {
   return inst.numNodes()-2;
}

Solution SortingDecoder::decodeSolution(const std::vector<double> &chromosome) const {
   // Solution being build.
   Solution currSol(inst);

   // Create the entire assignment order using random keys from chromossome.
   assert(chromosomeLength() == static_cast<int>(chromosome.size()) &&
      "Chromossome not long enough to support the sorting procedure.");

   // Pair the task lists with the chromosome keys.
   vector <TElem> tasks(initialTasks);
   {
      for (size_t i = 0; i < tasks.size(); ++i) {
         get<0>(tasks[i]) = chromosome[i];
      }
   }

   // Sorts the task list according the paired keys.
   sort(begin(tasks), end(tasks), [] (const TElem &i, const TElem &j) {
      return get<0>(i) < get<0>(j);
   });

   for (size_t i = 0; i < tasks.size(); ++i) {
      Task &task = get<1>(tasks[i]);

      Task best = task;
      best.cachedCost = 1e6;

      assert(task.skills[0] != -1 && "First service type is unset.");

      for (int v0: vehiSkills[task.skills[0]]) {

         task.vehi[0] = v0;

         if (inst.nodeSvcType(task.node) == Instance::SvcType::SINGLE) {
            if (currSol.findInsertionCost(task) < best.cachedCost)
               best = task;
         } else {
            assert(task.skills[1] != -1 && "Second service type is unset.");

            for (int v1: vehiSkills[task.skills[1]]) {
               if (v0 == v1)
                  continue;
               if (!inst.vehicleHasSkill(v1, task.skills[1]))
                  continue;

               task.vehi[1] = v1;

               if (currSol.findInsertionCost(task) < best.cachedCost)
                  best = task;
            }
         }
      }

      // Update the current solution.
      currSol.updateRoutes(best);
   }

   // Return nodes to depot.
   currSol.finishRoutes();

   return currSol;
}

double SortingDecoder::decode(const std::vector<double> &chromosome, bool rewrite) const {
   (void) rewrite;
   return decodeSolution(chromosome).cachedCost;
}

