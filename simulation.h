/*
 * simulation.h
 *
 *  Created on: 31.08.2014
 *      Author: meicron
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <armadillo>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "environment.h"
#include "navicontrol.h"


class Simulation {
public:
	Simulation(string in_param_type, int in_num_trials);
	~Simulation();

	void run_sim(double param);
	void run_inbound();
	void run_outbound();

	NaviControl* get_controller();
	Environment* get_environment();

	double bound_angle(double phi);
	double inv_angle(double angle);
	double in_degr(double angle);

	void set_param();

	// Simulation parameters
	int total_runs;
	bool inbound_on;
	int num_params;
	string file_ext;

	// Controller parameters
	int num_neurons;
	double sens_noise;
	double pi_leakage;
	int num_motivs;
	int max_outbound_time;
	int max_inbound_time;
	double motor_command;

	// Environment parameters
	int agents;
	double max_radius;
	double goal_density;
	int num_goals;
	double lm_density;
	int num_landmarks;

	// Evaluation
	int end_run;
	double success_rate;
	int blue_hits;
	int yellow_hits;
	vector< running_stat<double> > success_rate_avg;
	vector< running_stat<double> > explor_rate_avg;
	vector< running_stat<double> > success_rate_avg2;	//YELLOW
	vector< running_stat<double> > explor_rate_avg2;
	vector< running_stat<double> > prob_B;
	vector< running_stat<double> > prob_Y;
	vector< running_stat<int> > choice;
	running_stat<double> total_reward;


	mat gv_history0;
	mat gv_history1;

private:
	NaviControl* controller;
	Environment* environment;

	ofstream distor;
	ofstream PI_results;
	ofstream PI_resultseach;
	ofstream endpoints;
	ofstream gvlearn;
	ofstream lvlearn;
	ofstream stats_gl;
	ofstream statsall_gl;
	ofstream gv_angl;
	stringstream file_name;

	// Evaluation
	running_stat<double> PI_angular_error;
	running_stat<double> PI_linear_error;

	int num_goalhits;
	int num_homing;
	int run_div;
};



#endif /* SIMULATION_H_ */
