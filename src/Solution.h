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

#include <vector>

#include "Instance.h"
#include "Task.h"

struct Solution {
   constexpr static double ONE_THIRD = 1.0/3.0;
   constexpr static double COEFS[] = {ONE_THIRD, ONE_THIRD, ONE_THIRD}; // 0:dist, 1:tard, 2:tmax

   const Instance *inst;

   // [vehicle] -> route
   // get<0> -> node
   // get<1> -> skill
   std::vector <std::vector<std::tuple<int,int>>> routes;

   // Task insertion order.
   std::vector <Task> insertOrder;

   std::vector <int> vehiPos;
   std::vector <double> vehiLeaveTime;

   double dist;
   double tard;
   double tmax;
   double cachedCost;

   Solution(const Instance &inst_);

   double findInsertionCost(Task &task) const;

   void updateRoutes(const Task &task);

   void finishRoutes();

   void writeTxt(const char *fname) const;
   void writeTxt2(const char *fname, const std::string &headers = "") const;

};

