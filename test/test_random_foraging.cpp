/*
 * test_random_foraging.cpp
 *
 *  Created on: 28.07.2015
 *      Author: meicron
 */

#include "../src/simulation.h"
#include "../src/timer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
using namespace std;

Simulation* sim;
const int numagents= 1;
const int numtrials= 100;
const double T= 1000.;
const double dt= 0.1;

int main(){
	Timer timer(true);

	sim = new Simulation(numagents, true);
	sim->init_controller(16, 0.00, 0.00);
	sim->run(numtrials, T, dt);
	delete sim;
	auto elapsed_secs_cl = timer.Elapsed();

	printf("%4.3f s. Done.\n", elapsed_secs_cl.count()/1000.);
}


