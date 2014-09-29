/*
 * environment.h
 *
 *  Created on: Jul 17, 2014
 *      Author: degoldschmidt
 */

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include "agent.h"
#include "goal.h"
#include "landmark.h"
#include "pipe.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

class Environment {
public:
	Environment(double num_agents); //Empty environment
	Environment(double num_agents, double num_goals, double num_landmarks, double max_radius);
	~Environment();

	void update(double command);
	void add_goal(double x, double y, int color = 0);
	void add_landmark(double x, double y);
	void add_pipe(double x0, double x1, double y0, double y1, double width);
	int get_hits();
	int get_hits(int i);
	double get_total_reward();
	double get_distance(Goal* g1, Goal* g2);
	double get_distance(Landmark* lm1, Goal* g2);
	double get_distance(Landmark* lm1, Landmark* lm2);
	Goal* nearest(double x=0.0, double y=0.0);
	double get_real_HV();
	double getx();
	double gety();
	double d(double x0, double x1);
	int type();
	void reset();
	void swap_reward();
	vector<Agent*> agent_list;
	vector<Goal*> goal_list;
	vector<Landmark*> landmark_list;
	vector<Pipe*> pipe_list;

	double reward;
	double sum_reward;
	double total_reward;
	double lm_recogn;
	bool flag;
	int count;
	int mode; // 0 = outb, 1 = inb
	int inv_sampling_rate;

	ofstream stream_g;		//goal positions
	ofstream stream_lm;		//landmark positions
	ofstream stream_p;		//pipe positions
	ofstream stream_h;		//home positions
	ofstream stream_food;		//amount of food at goals over time
};

#endif /* ENVIRONMENT_H_ */
