/*
 * main.cpp
 *
 *  Created on: 01.08.2014
 *      Author: meicron
 */

#include <armadillo>
#include "environment.h"
#include "navicontrol.h"
#include "timer.h"
using namespace std;
using namespace arma;

NaviControl* controller;
Environment* environment;

const int num_neurons = 360;			//Number of array neurons
const int num_motivs = 2;				//0=outbound,	1=inbound
const int max_outbound_time = 120;//200;
const int max_inbound_time = 10;//600;
const int total_runs = 20;
const double factor = 0.5;

const int agents = 1;
const double m_radius = 150.;
const double g_density = 0.005;//0.005;//0.0001;
const int goals = int(g_density * (M_PI * m_radius * m_radius));
const double lm_density = 0.0;//0.02;//0.001;// 0.0001;
const int landmarks = int(lm_density * (M_PI * m_radius * m_radius));
double command;
double PI_angle_error;
double PI_dist_error;
int total_hits = 0;

bool inbound_on = false;

double bound_angle(double phi){
	double rphi;
	rphi = phi;
	while(rphi > M_PI)
		rphi -= 2 * M_PI;
	while(rphi < - M_PI)
		rphi += 2 * M_PI;
	return rphi;
}

double inv_angle(double angle){
	return bound_angle(angle - M_PI);
}

double in_degr(double angle){
	return 180.*angle/M_PI;
}

int main(){
	Timer timer(true);
	controller = new NaviControl(num_neurons);											/////	+
	environment = new Environment(agents/*, goals, landmarks, m_radius*/);

	//environment->add_pipe(0.,0.,-0.0,10.,.2);			//Goal Learning Exp A (Pipe 1)
	//environment->add_pipe(0.,0.,-0.0,3.,.2);			//Goal Learning Exp A (Pipe 2)
	double a = sqrt(2.)*3.;
	//environment->add_pipe(0., a, 0., a,.2);			//Goal Learning Exp A (Pipe 3)
	//environment->add_pipe(0.,25.,25.,25.,.2);
	//environment->add_pipe(0.2,-25.,25.,25.,.4);
	//environment->add_pipe(0.,0.,25.,50.,.4);
	environment->add_goal(0,10);
	//environment->add_goal(-25.,25.);
	//environment->add_goal(0.,50.);

	for(int run = 0; run < total_runs; run++){
		//if(total_runs > 100)
			//controller->reset_matrices();
		double start_time = controller->t;
		double old_gvl = controller->gln->length;		//Old GV len

		while(controller->t < start_time + max_outbound_time && environment->sum_reward < 2.){																	//OUTBOUND RUN (SEARCHING)
			controller->set_outbound();
//			if(controller->t > start_time + max_outbound_time - 50)
//				environment->reward -= 0.0;

			command = controller->update(environment->agent_list.at(0)->phi, environment->agent_list.at(0)->v, environment->reward);

			//if(controller->t%20==0)



			environment->update(command);
			PI_angle_error = bound_angle(controller->PI_avg_angle - environment->agent_list.at(0)->theta);
			PI_dist_error = controller->pin->length - environment->agent_list.at(0)->distance;
			if(controller->t%500==0)
				printf("t = %4u\tPI_error_ang = %1.3f\tPI_error_lin = %6.3f\tPhi = %3.3f\tGV angle = %3.6f (%2.6f)\n", controller->t, PI_angle_error, PI_dist_error, in_degr(environment->agent_list.at(0)->phi), in_degr(controller->GV_angle), controller->gln->length);
		}
		double in_time = controller->t;
		while(inbound_on && environment->agent_list.at(0)->distance > 0.2 && controller->t < in_time + max_inbound_time){ 	//INBOUND RUN (PI HOMING)
			controller->set_inbound();
			command = controller->update(environment->agent_list.at(0)->phi, environment->agent_list.at(0)->v, environment->reward);;
			environment->update(command);
			if(environment->agent_list.at(0)->in_pipe)
				cout << "No. I'm in a pipe.\n";
			if(controller->t%100==0)
				printf("t = %4u\tFB_error = %1.3f\tPhi = %1.3f\n", controller->t, controller->feedback_error, environment->agent_list.at(0)->phi);
		}
		if(controller->gln->length - old_gvl < 0.000000001 && run > 20)	//convergence criterion
			run = total_runs;

		total_hits += environment->get_hits();
		printf("Run = %3u, time@nest = %7u, hits/ts = %1.6f, hits = %u, max act = %6.3f\n", run+1, controller->t, (1.0*total_hits)/(1.0*controller->t), total_hits, controller->pin->length);
		environment->reset();
		controller->reset();
	}
	controller->save_matrices();
	//printf("PI distortion rate = %4.9f, GV len = %4.9f\n", controller->pin->length/environment->agent_list.at(0)->distance, controller->gln->length);
	delete controller;
	delete environment;
	auto elapsed_secs_cl = timer.Elapsed();
	printf("%4.3f s. Done.\n", elapsed_secs_cl.count()/1000.);
}


