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

#include "Instance.h"
#include "SortingDecoder.h"
#include "Timer.h"

#include "brkga_mp_ipr.hpp"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>

   using namespace std;

enum class DistanceFunc {
   HAMMING,
   KENDALL
};

boost::program_options::variables_map parseCommandline(int argc, char **argv) {
   namespace po = boost::program_options;
   po::options_description desc("Accepted command options are");
   desc.add_options()
      ("help,h", "shows this text")

      ("instance,i", po::value<string>(), "path to the instance file")

      ("seed,s", po::value<int>()->default_value(1), "seed for the PRNG")

      ("printall", "log the search progress in each generation, otherwise from "
         "50 to 50 generations, or when a improved solution is found")

      ("popsize,p", po::value<int>()->default_value(300), "number of individuals "
       "in each population")

      ("gens,g", po::value<int>()->default_value(600), 
       "max. generations for the BRKGA phase")

      ("pe", po::value<double>()->default_value(0.15), "percentage of elite population")

      ("pm", po::value<double>()->default_value(0.15), "percentagem of mutant population")

      ("mp", po::value<int>()->default_value(2), "number of parents involved in the mating")

      ("ep", po::value<int>()->default_value(1), "number of elite parents involved in the mating")

      ("bp", po::value<string>()->default_value("loginverse"), "set the bias"
       " function to be used while selecting parents for mating. Accepted "
       "values: constant, cubic, exponential, linear, loginverse, quadratic")

      ("npop", po::value<int>()->default_value(1), "number of independent populations to evolve")

      ("npairs", po::value<int>()->default_value(100), "number of pairs of chromosomes to test during PR")

      ("mindist", po::value<double>()->default_value(0.15), "minimal distance between chromosomes to allow "
       "performing PR")

      ("psel", po::value<string>()->default_value("best"), "specifies which individuals used durint the PR. "
       "Valid options: best, random")

      ("alpha", po::value<double>()->default_value(1.0), "defines the block size during PR")

      ("hdist", po::value<double>()->default_value(0.6), "defines the minimal hamming distance "
         "between two chromosomes to allow running the implicit path relinking")

      ("pperc", po::value<double>()->default_value(1.0), "defines the percentage of PR path to explore")

      ("ptype", po::value<string>()->default_value("permutation"), "type of PR. Valid values: direct, permutation")

      ("dfunc", po::value<string>()->default_value(""), "type of distance function to be used. Accepted values are hamming and kendall. If the parameter is suppressed, it is set automatically according to value of ptype: permutation => kendall, direct => hamming")

      ("pperiod", po::value<int>()->default_value(50), "number of generations between PR attempts")

      ("reset", po::value<long>()->default_value(1e6), "number of non-improving generations prior resetting the populations")

      ("xelite", po::value<int>()->default_value(100), "number of generations between exchanging elite from populations")

      ("immigrants", po::value<int>()->default_value(8), "number of immigrants while exchanging elites")
   ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);    

   if (vm.count("help") || !vm.count("instance")) {
      cout << desc << "\n";
      exit(EXIT_FAILURE);
   }

   return vm;
}

BRKGA::BrkgaParams extractFrom(boost::program_options::variables_map &vm) {
   auto params {BRKGA::BrkgaParams()};

   params.population_size = vm["popsize"].as<int>();
   params.elite_percentage = vm["pe"].as<double>();
   params.mutants_percentage = vm["pm"].as<double>();
   params.total_parents = vm["mp"].as<int>();
   params.num_elite_parents = vm["ep"].as<int>();

   {
      auto bp = vm["bp"].as<string>();
      if (bp == "constant") {
         params.bias_type = BRKGA::BiasFunctionType::CONSTANT;
      } else if (bp == "cubic") {
         params.bias_type = BRKGA::BiasFunctionType::CUBIC;
      } else if (bp == "exponential") {
         params.bias_type = BRKGA::BiasFunctionType::EXPONENTIAL;
      } else if (bp == "linear") {
         params.bias_type = BRKGA::BiasFunctionType::LINEAR;
      } else if (bp == "loginverse") {
         params.bias_type = BRKGA::BiasFunctionType::LOGINVERSE;
      } else if (bp == "quadratic") {
         params.bias_type = BRKGA::BiasFunctionType::QUADRATIC;
      } else {
         cout << "Invalid bias for parent selection: " << bp << endl;
         exit(EXIT_FAILURE);
      }
   }

   params.num_independent_populations = vm["npop"].as<int>();
   params.pr_number_pairs = vm["npairs"].as<int>();
   params.pr_minimum_distance = vm["mindist"].as<double>();

   {
      const auto ty = vm["ptype"].as<string>();
      if (ty == "direct") {
         params.pr_type = BRKGA::PathRelinking::Type::DIRECT;
      } else if (ty == "permutation") {
         params.pr_type = BRKGA::PathRelinking::Type::PERMUTATION;
      } else {
         cout << "Unknown path relinking type: " << ty << endl;
         exit(EXIT_FAILURE);
      }
   }
   
   {
      auto sel = vm["psel"].as<string>();
      if (sel == "best") {
         params.pr_selection = BRKGA::PathRelinking::Selection::BESTSOLUTION;
      } else if (sel == "random") {
         params.pr_selection = BRKGA::PathRelinking::Selection::RANDOMELITE;
      } else {
         cout << "Invalid individual selection for PR: " << sel << endl;
         exit(EXIT_FAILURE);
      }
   }

   params.alpha_block_size = vm["alpha"].as<double>();
   params.pr_percentage = vm["pperc"].as<double>();

   return params;
}
   
void printSupplyDemandIndicators(const Instance &inst) {
   cout << "Summary of supply x demand for each service type:\n";
   for (int s = 0; s < inst.numSkills(); ++s) {
      int supply = 0, demand = 0;
      for (int i = 1; i < inst.numNodes()-1; ++i) {
         demand += inst.nodeReqSkill(i, s) ? 1 : 0;
      }
      for (int v = 0; v < inst.numVehicles(); ++v) {
         supply += inst.vehicleHasSkill(v, s) ? 1 : 0;
      }
      cout << "Skill " << s << " has supply = " << supply << " and demand "
         "= " << demand << ", ratio = " <<  double(supply)/demand<< "\n";
   }
}

int main(int argc, char* argv[]) {

   auto args = parseCommandline(argc, argv);
   auto brkga_params = extractFrom(args);

   cout << "--- BRKGA-MP-IPR for HHCRSP ---\n";
   cout << "Instance: " << args["instance"].as<string>() << endl;
   cout << "Seed: " << args["seed"].as<int>() << endl;

   cout << endl;

   double overallBest = 1e75;
   try {
      
      const unsigned seed = args["seed"].as<int>();
      const string config_file = "config.conf";
      const unsigned num_generations = args["gens"].as<int>();
      const string instance_file = args["instance"].as<string>();
      const int immigrants = args["immigrants"].as<int>();
      const double hdist = args["hdist"].as<double>();

      cout << "Reading data... " << flush;
      auto instance = Instance(instance_file.c_str());
      cout << "Problem contains " << (instance.numNodes()-2) << " patients "
         " and " << instance.numVehicles() << " caregivers.\n\n";
      printSupplyDemandIndicators(instance);
      cout << endl;

      cout << "Reading parameters..." << endl;
      DistanceFunc dfunc =
         brkga_params.pr_type == BRKGA::PathRelinking::Type::DIRECT
            ? DistanceFunc::HAMMING
            : DistanceFunc::KENDALL;

      {
         const auto &str = args["dfunc"].as<string>();
         if (str.empty()) {
            cout << "Inferred distance function by PR type: ";
            if (dfunc == DistanceFunc::HAMMING)
               cout << "hamming";
            else
               cout << "kendall-tau";
            cout << endl;
         } else {
            if (str == "hamming") {
               dfunc = DistanceFunc::HAMMING;
               cout << "Using hamming distance forcibly.\n";
            } else if (str == "kendall") {
               dfunc = DistanceFunc::KENDALL;
               cout << "Using kendall-tau distance forcibly.\n";
            } else {
               cout << "Unknown distance function: " << str << "\n";
               abort();
            }
         }
      }

      cout << "Building BRKGA data and initializing..." << endl;
      SortingDecoder decoder(instance);
      BRKGA::BRKGA_MP_IPR<SortingDecoder> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            decoder.chromosomeLength(), brkga_params, omp_get_max_threads());

      algorithm.initialize();

      cout << "Evolving " << num_generations << " generations..." << endl;
      const auto prPeriod = args["pperiod"].as<int>();
      const auto resetPeriod = args["reset"].as<long>();
      const auto exchangePeriod = args["xelite"].as<int>();
      const auto printPeriod = args.count("printall") ? 1 : 50;

      unsigned generation = 0;
      Timer tm;
      int noImprove = 0;

      double localBest = numeric_limits<double>::infinity();
      double dist = localBest, tard = localBest, tmax = localBest;

      int linesPrinted = 0;
      int prOp = 0, xeOp = 0, rstOp = 0;

      double eliteMean, eliteStdev;
      auto updateEliteDiversity = [&] () {
         int cnt = 0;
         eliteMean = 0.0;
         const auto npop = brkga_params.num_independent_populations;
         const auto pops = int(brkga_params.population_size * brkga_params.elite_percentage);
         for (unsigned k = 0; k < npop; ++k) {
            for (int i = 0; i < pops; ++i) {
               eliteMean += algorithm.getFitness(k, i);
               ++cnt;
            }
         }
         eliteMean /= cnt;
         eliteStdev = 0.0;
         for (unsigned k = 0; k < npop; ++k) {
            for (int i = 0; i < pops; ++i) {
               eliteStdev += pow(algorithm.getFitness(k, i) - eliteMean, 2);
            }
         }
         eliteStdev /= cnt;
         eliteStdev = sqrt(eliteStdev);
      };

      // Prints the header of algorithm output.
      auto printHeader = [] () {
         cout << "\n";
         cout << 
            setw(1) << "" << " " <<
            setw(4) << "Gens" << " " <<
            setw(4) << "Rem" << " " <<
            
            setw(8) << "Local" << " " << 
            setw(7) << "ElDiv" << " " <<
            setw(3) << "NoImpr" << " " <<

            setw(33) << "Best solution        " << " " << 
            
            setw(7) << "Time" << " " <<
            setw(3) << "Op" << 
         endl;

         cout << 
            setw(34) << "" << " " <<
            setw(8) << "Cost" << " " <<
            setw(8) << "Dist" << " " <<
            setw(7) << "Tard" << " " <<
            setw(7) << "TMax" << " " <<
         endl;

      };

      // Prints the algorithm progress.
      char hdr = ' ';
      string evt = "";
      auto printProgress = [&] () {
         if (linesPrinted >= 15) {
            printHeader();
            linesPrinted = 0;
         } else {
            ++linesPrinted;
         }

         updateEliteDiversity();
         tm.finish();
         cout << 
            fixed << 
            setw(1) << hdr << " " <<
            setw(4) << generation << " " <<
            setw(4) << num_generations - generation << " " <<
            
            setw(8) << setprecision(2) << localBest << " " << 
            setw(7) << setprecision(2) << eliteStdev << " " <<
            setw(6) << noImprove << " " <<
            
            setw(8) << setprecision(2) << overallBest << " " <<
            setw(8) << setprecision(2) << dist << " " <<
            setw(7) << setprecision(1) << tard << " " <<
            setw(7) << setprecision(1) << tmax << " " <<

            setw(7) << setprecision(1) << tm.elapsed() << " " <<
            setw(3) << evt << 
         endl;
         hdr = ' ';
         evt = "";
      };
      
      printHeader();
      tm.start();
      do {
         bool hasImproved = false;
         algorithm.evolve();         
         
         if (localBest >= numeric_limits<double>::infinity()) {
            localBest = algorithm.getBestFitness(); 
            hasImproved = true;
         }
         
         if (localBest < overallBest) {
            overallBest = localBest;
            const auto sol = decoder.decodeSolution(algorithm.getBestChromosome());
            dist = sol.dist;
            tard = sol.tard;
            tmax = sol.tmax;
            hdr = '*';
         }

         if (generation % printPeriod == 0 || hasImproved || hdr == '*') {
            printProgress();
         }

         if (prPeriod > 0 && generation > 0 && generation % prPeriod == 0) {
            using BRKGA::PathRelinking::PathRelinkingResult;
            evt += 'P';
            prOp++;
            PathRelinkingResult result;
            if (dfunc == DistanceFunc::HAMMING) {
               result = algorithm.pathRelink(make_shared<BRKGA::HammingDistance>(hdist));
            } else {
               result = algorithm.pathRelink(make_shared<BRKGA::KendallTauDistance>());
            }
            cout << "Path relinking result: ";
            switch(result) {
               case PathRelinkingResult::TOO_HOMOGENEOUS:
                  cout << "too homogeneous.";
                  break;
               case PathRelinkingResult::NO_IMPROVEMENT:
                  cout << "no improvement.";
                  break;
               case PathRelinkingResult::ELITE_IMPROVEMENT:
                  cout << "elite improvement.";
                  break;
               case PathRelinkingResult::BEST_IMPROVEMENT:
                  cout << "best improvement.";
                  break;
            }
            cout << endl;
         }

         if (exchangePeriod > 0 && generation > 0 && brkga_params.num_independent_populations > 1 && generation % exchangePeriod == 0) {
            evt += 'X';
            xeOp++;
            algorithm.exchangeElite(immigrants);
         }

         
         if (localBest - algorithm.getBestFitness() < 0.05) {
            ++noImprove;
         } else {
            localBest = algorithm.getBestFitness();
            noImprove = 0;
         }

         if (noImprove >= resetPeriod) {
            evt += 'R';
            rstOp++;
            localBest = numeric_limits<double>::infinity();
            algorithm.reset();
            noImprove = 0;
         }

         if (!evt.empty()) {
            printProgress();
         }

         ++generation;
      } while (generation < num_generations);

      printProgress(); 
      cout << "Evolutionary process finished.\n";

      {
         stringstream ss;
         ss << "solution-" << getpid() << ".txt";
         auto sol = decoder.decodeSolution(algorithm.getBestChromosome());
         sol.writeTxt2(ss.str().c_str(), "# Seed is " + to_string(seed) + ".\n");
         cout << "Solution written to '" << ss.str() << "'.\n";

         sort(rbegin(sol.insertOrder), rend(sol.insertOrder), [] (const auto &i, const auto &j) {
            return i.incTard < j.incTard;
         });

         cout << "\nSorted sequence of worst to best tardiness value:\n";
         for (const auto &t: sol.insertOrder)
            cout << t << endl;

         cout << "\nUnused vehicle list: ";
         int idle = 0;
         for (int v = 0; v < instance.numVehicles(); ++v) {
            if (sol.routes[v].size() <= 2) {
               ++idle;
               cout << v << ' ';
            }
         }
         cout << "(total = " << idle << " out of " << instance.numVehicles() << ")" << endl;
      }


      cout << "\n---\nSearch finished.\n";
      cout << "Total of " << generation << " generations in " << tm.elapsed() << " seconds.\n";
      cout << "Exchange elite runs: " << xeOp << "\n";
      cout << "Implicit path relinking runs: " << prOp << "\n";
      cout << "Reset attempts: " << rstOp << "\n";
      cout << "\n";

      cout << "Best solution found:\n";
      cout << "   Cost: " << overallBest << "\n";
      cout << "   Total travel time: " << dist << "\n";
      cout << "   Total tardiness time: " << tard << "\n";
      cout << "   Largest tardiness: " << tmax << "\n";
   }
   catch(exception& e) {
      cerr << "\n***********************************************************"
         << "\n****  Exception Occured: " << e.what()
         << "\n***********************************************************"
         << endl;
      return 70; // BSD software internal error code
   }

   cout << "\n\n" << overallBest << endl;

   return 0;
}
