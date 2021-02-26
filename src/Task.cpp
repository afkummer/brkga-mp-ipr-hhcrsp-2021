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

#include "Task.h"

#include <iostream>
#include <limits>

using namespace std;


Task::Task(): Task(-1, -1, -1) {
   // Empty.
}

Task::Task(int node_, int skill1, int skill2) {
   this->node = node_;
   skills[0] = skill1;
   skills[1] = skill2;

#ifndef NDEBUG
   vehi[0] = -1;
   vehi[1] = -1;
   leaveTime[0] = numeric_limits<double>::infinity();
   leaveTime[1] = leaveTime[0];
#endif

}

std::ostream &operator<<(std::ostream &out, const Task &t) {
   out << "Task for patient = " << t.node << " with skills = {" << t.skills[0] <<
      "," << t.skills[1] << "} and cost = " << t.cachedCost << "\n";

   out << "   Vehicle #0 = " << t.vehi[0] << ", leaving at = " << t.leaveTime[0] << "\n";
   out << "   Vehicle #1 = " << t.vehi[1] << ", leaving at = " << t.leaveTime[1] << "\n";

   out << "   Increment on distance indicator: " << t.incDist << "\n";
   out << "   Increment on tard indicator:     " << t.incTard << "\n";
   out << "   Value of tmax indicator:         " << t.incDist << endl;

   return out;
}

std::vector<Task> createTaskList(const Instance &inst) {
   vector <Task> pending(inst.numNodes()-2);

   for (size_t i = 0; i < pending.size(); ++i) {
      Task &t = pending[i];
      t.node = i+1;
      int ss = 0;

      for (int s = 0; s < inst.numSkills(); ++s) {
         if (inst.nodeReqSkill(i+1, s))
            t.skills[ss++] = s;
      }
   }

   return pending;
}
