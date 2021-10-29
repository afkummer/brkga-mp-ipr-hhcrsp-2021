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

#include <boost/container_hash/hash.hpp>

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
   allTasks = createTaskList(inst);
   lexOrder.resize(allTasks.size());
   iota(lexOrder.begin(), lexOrder.end(), 0);
#ifdef DECODER_CACHE_STATS
   cacheHit = 0;
   cacheMiss = 0;
#endif
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

   auto taskIndices = lexOrder;
   sort(begin(taskIndices), end(taskIndices), [&](int i, int j) {
      return chromosome[i] < chromosome[j];
   });   

   const auto hval = hash(taskIndices);
   auto hcost = fetchCache(hval);
   if (hcost != numeric_limits<double>::infinity()) {
      currSol.cachedCost = hcost;
#ifdef DECODER_CACHE_STATS
      #pragma omp critical
      const_cast<int&>(cacheHit) += 1;
#endif
      return currSol;
   }      

   for (size_t i = 0; i < taskIndices.size(); ++i) {
      Task task = allTasks[taskIndices[i]];

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

   // Updates the cache.
   #pragma omp critical
   const_cast<decltype(lookupCache)&>(lookupCache)[hval] = currSol.cachedCost;

#ifdef DECODER_CACHE_STATS
   #pragma omp atomic
   const_cast<int&>(cacheMiss) += 1;
#endif

   return currSol;
}

double SortingDecoder::decode(const std::vector<double> &chromosome, bool rewrite) const {
   (void) rewrite;
   return decodeSolution(chromosome).cachedCost;
}

size_t SortingDecoder::hash(const std::vector<int> &ch) const noexcept {
   size_t seed = 0;
   boost::hash_range(seed, ch.begin(), ch.end());
   return seed;
}

double SortingDecoder::fetchCache(size_t hash) const noexcept {
   auto it = lookupCache.find(hash);
   if (it == lookupCache.end())
      return numeric_limits<double>::infinity();
   return it->second;
}
