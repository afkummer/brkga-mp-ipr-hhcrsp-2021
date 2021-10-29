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

#pragma once

#include "Instance.h"
#include "Solution.h"

#include <unordered_map>

// #define DECODER_CACHE_STATS

struct SortingDecoder {
   const Instance &inst;

   // Cache of vehicles by skills.
   // [skill id] -> vehicles list
   std::vector <std::vector <int>> vehiSkills;

   // Caches the task vector used into the decoding process.
   std::vector <Task> allTasks;
   std::vector <int> lexOrder;

   SortingDecoder(const Instance &inst_);

   int chromosomeLength() const;

   Solution decodeSolution(const std::vector <double> &chromosome) const;

   double decode(const std::vector <double> &chromosome, bool rewrite) const;

   // Data structure to cache solution values.
   // Useful because distinct individuals lead to the same solution.
   std::unordered_map <size_t, double> lookupCache;
#ifdef DECODER_CACHE_STATS
   int cacheHit, cacheMiss;
#endif

   size_t hash(const std::vector<int> &ch) const noexcept;
   double fetchCache(size_t hash) const noexcept;
   void clearCache() noexcept;
};
