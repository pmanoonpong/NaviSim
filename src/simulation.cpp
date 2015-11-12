/*****************************************************************************
 *  simulation.cpp                                                           *
 *                                                                           *
 *  Created on:   Aug 31, 2014                                               *
 *  Author:       Dennis Goldschmidt                                         *
 *  Email:        goldschmidtd@ini.phys.ethz.ch                              *
 *                                                                           *
 *                                                                           *
 *  Copyright (C) 2014 by Dennis Goldschmidt                                 *
 *                                                                           *
 *  This file is part of the program NaviSim                                 *
 *                                                                           *
 *  NaviSim is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 ****************************************************************************/

#include "simulation.h"

Simulation::Simulation(int in_numtrials, int in_agents, bool random_env){
	N = in_numtrials;
	agents = in_agents;
	rand_env = random_env;
	VERBOSE = false;
	SILENT = false;

	pin_on = true;
	homing_on = false;
	gvlearn_on = false;
	gvnavi_on = false;
	lvlearn_on = false;
	beta_on = false;

	(VERBOSE)?printf("Building environment.\n"):VERBOSE;
	//environment = (rand_env ? new Environment(10, 10, 25., 1) : new Environment(agents));
	environment = (rand_env ? new Environment(ngs, nlms, m_rad, agents) : new Environment(agents));
	(VERBOSE)?printf("Done.\n"):VERBOSE;

	T = 0.;
	dt = 0.;
	trial = 1;
	start_time = 0.;
	global_t = 0.;
	trial_t = 0.;
	timestep = 0;
	count_home = 0;
	count_goal = 0;

	trial_converge = 0;
	expl_rate.resize(N);
	home_rate.resize(N);
	goal_rate.resize(N);

	agent_str.width(10);
	agent_str << "#Trial\t#Trial_t\t#X\t#Y\t#Dis\t#Phi\t#Theta\t#Global_t\t#EligibLM\n";
	agent_str.open("data/agent.dat");
	endpts_str.open("data/endpoints.dat");
	//error_dist.open(str_names.at(pos).c_str());
	homevector_str.open("data/homevector.dat");
	globalvector_str.width(10);
	globalvector_str << "#Trial_t\t#Global_t\t#X\t#Y\t#Theta_GV\t#D_GV\t#Expl\t#GoalCount\t#GV_ThPVA\n";
	globalvector_str.open("data/globalvector.dat");
	localvector_str.open("data/localvector.dat");
	refvector_str.open("data/refvector.dat");
	lmr_signals.open("data/lmr_signals.dat");
	reward_str.open("data/reward.dat");
	sim_cfg.open("data/sim.cfg");
	sim_cfg << "# Na\t# Nn\t# Sno\t# Leak\t# Uncno" << endl;
	sim_cfg << agents << "\t";
	length_scaling.open("data/l_scale.dat");
	out_signals.open("data/signals.dat");
	lmr_angles.open("data/lmr_angles.dat");
	adaptive_expl.open("data/adaptive_expl.dat");
	trialtimes.open("data/trialtimes.dat");
	performance_gvl.open("data/performgvl.dat");
	performance_gvl.width(10);
	performance_gvl << "#Trial\t#ExplRate\t#HomeRate\t#GoalRate\n";
}

Simulation::~Simulation(){
	total_pi_error.reset();
	agent_str.close();
	endpts_str.close();
	error_dist.close();
	homevector_str.close();
	globalvector_str.close();
	localvector_str.close();
	refvector_str.close();
	reward_str.close();
	sim_cfg.close();
	length_scaling.close();
	out_signals.close();
	lmr_signals.close();
	lmr_angles.close();
	adaptive_expl.close();
	trialtimes.close();
	performance_gvl.close();
	delete environment;
}

Agent* Simulation::a(int i){
	return environment->a(i);
}

void Simulation::add_goal(double x, double y, int color, double size, bool decay){
	environment->add_goal(x, y, color, size, decay);
}

void Simulation::add_goal(double max_radius){
	environment->add_goal(max_radius);
}

void Simulation::add_landmark(double x, double y){
	environment->add_landmark(x, y);
}

void Simulation::add_pipe(double x0, double x1, double y0, double y1){
	environment->add_pipe(x0,x1,y0,y1);
}

void Simulation::beta(bool _opt){
	beta_on = _opt;
}

Controller* Simulation::c(int i){
	return controllers.at(i);
}

Environment* Simulation::e(){
	return environment;
}

void Simulation::gvlearn(bool _opt){
	gvlearn_on = _opt;
}

void Simulation::homing(bool _opt){
	homing_on = _opt;
}

void Simulation::init_controller(int num_neurons, double sensory_noise, double uncor_noise, double leakage, double syn_noise){
	sim_cfg << num_neurons << "\t" << sensory_noise << "\t" << uncor_noise << "\t" << leakage << endl;
	vector<bool> opt_switches = {homing_on, gvlearn_on, lvlearn_on, SILENT};
	for(unsigned int i= 0; i< agents; i++){
		Controller* control = new Controller(num_neurons, sensory_noise, leakage, uncor_noise, syn_noise, opt_switches);
		int size = N*pow( 10, int(log10( double( num_neurons ) ) ) );
		control->set_sample_int(size/10);      // sample activity data every 10 time steps

		control->beta_on = beta_on;
		a(i)->init(control);
		controllers.push_back(control);
	}
}

void Simulation::lvlearn(bool _opt){
	lvlearn_on = _opt;
}

void Simulation::reset(){
	timestep = 0;
	trial_t = 0.;
	pi_error.reset();
	pi_error_max.reset();
	environment->reset();
}

void Simulation::run(int in_numtrials, double in_duration, double in_interval){
	N = in_numtrials;
	T = in_duration;
	dt = in_interval;
	global_t = 0.0;
	int total_steps = int(N*T/dt);
	sample_time = int(total_steps/1000000.);
	if(sample_time < 1)
		sample_time = 1;
	if(c()->get_inward() == 0)
		c()->set_inward(T/dt);
	if(!SILENT){
		printf("Total timesteps is %u\nSet sampling interval to %u\n", total_steps, sample_time);
		printf("Inward time is %u\n", c()->get_inward());
	}

	for(; trial < N+1; trial++){
		start_time = global_t;
		prev_expl = c()->expl(0);

		reset();
		while(trial_t < T){
			if(int(trial_t/dt)%sample_time == 0)
				writeTrialData();
			update();
			pi_error( (a(0)->HV()-a(0)->v()).len() );
		}
		if(trial_t <= T + 0.5)
			is_home(0);

		if(prev_expl >= 0.5 && c()->expl(0) < 0.5)
			trial_converge = trial;
		expl_rate.at(trial-1) = c()->expl(0);
		home_rate.at(trial-1) = is_home.mean();
		goal_rate.at(trial-1) = is_goal.mean();

		if(N > 1 && !SILENT){
			total_pi_error( pi_error.mean() );
			avg_length(a(0)->d());
			writeSimData();
			if(N > 19 && trial%(in_numtrials/20)==0){
				printf("#%u\tL=%g\t", trial, avg_length.mean());
				if(pin_on)
					printf("e=%2.3f\t<e>=%2.3f\t", pi_error.mean(), total_pi_error.mean());
				if(homing_on)
					printf("<R>=%1.2f\t", avg_reward.mean());
					//printf("<Home>=%3.1f%%\t<Goal>=%2.0f\t", 100.*is_home.mean(), 1.0*count_goal);
				if(gvlearn_on)
					printf("e= %1.2f\tGV= (%3.1f, %1.2f)\t", c()->expl(0), c()->GV_vecavg().deg(), a(0)->GV().len()/*, e()->nearest()->d()*/);
				if(lvlearn_on){
					for(int i = 0; i < c()->K(); i++){
						printf("LV%u=(%3.1f,%1.2f), V%u= %1.1f\t", i, c()->LV_vecavg(i).deg(), c()->LV(i).len(), i, c()->LV_value_raw(i));
					}
				}
				if(beta_on)
					printf("Beta=%g\t", c()->e_beta());
				//printf("Amount=%g", e()->g(0)->a());
				printf("\n");
			}

			if(N <= 19 && !SILENT){
				printf("#%u\t", trial);
				if(pin_on)
					printf("e=%2.3f\t<e>=%2.3f\t", pi_error.mean(), total_pi_error.mean());
				if(homing_on)
					printf("<G>=%5.0f\t", 1.0*count_goal);
					//printf("<Home>=%3.1f%%\t<Goal>=%2.0f\t", 100.*is_home.mean(), 1.0*count_goal);
				if(gvlearn_on)
					printf("e= %1.2f\tGV= (%3.1f, %1.2f)\t", c()->expl(0), a(0)->GV().ang().deg(), a(0)->GV().len()/*, e()->nearest()->d()*/);
				if(lvlearn_on){
					for(int i = 0; i < c()->K(); i++){
						printf("LV%u=(%3.1f,%1.2f), V%u= %1.1f\t", i, c()->LV(i).ang().deg(), c()->LV(i).len(), i, c()->LV_value_raw(i));
					}
				}
				if(beta_on)
					printf("Beta=%g\t", c()->e_beta());
				printf("\n");
			}
		}
	}
}

void Simulation::set_inward(int _time){
	c()->set_inward(_time);
}

void Simulation::update(){
//	if(accu(c()->GV_module()->dW()) < 0.0 && (a(0)->pos - c()->HV()).len() > 0.3)
//		printf("GV learn at (%g,%g) -> (%g, %g), R = %g\n", a(0)->pos.x, a(0)->pos.y, c()->HV().x, c()->HV().y, c()->GV_module()->R());

	if(timestep%1000==0 && N == 1 && pin_on && !SILENT)
		printf("Time = %g\te = %g\te_max = %g\n", trial_t, pi_error.mean(), pi_error_max.mean());
	timestep++;
	trial_t += dt;
	global_t += dt;
	avg_reward(c()->R(0));
	environment->update();
	is_goal(environment->get_hits(0) > 0);
	count_goal += environment->get_hits(0);
	if(environment->stop_trial){
		if(N < 10 && !SILENT)
			printf("Homing success at %g s\n", trial_t);
		trial_t = T+1.;
		count_home++;
		is_home(1);
	}
}

void Simulation::writeSimData(){
	trialtimes << trial << "\t" << start_time << "\t" << global_t << "\t" << global_t-start_time << endl;
	performance_gvl << fixed;
	if(gvlearn_on){
		performance_gvl << setprecision(0) << trial << "\t" ;
		performance_gvl	<< setprecision(6) << c()->expl(0) << "\t";
		performance_gvl	<< setprecision(6) << is_home.mean() << "\t";
		performance_gvl	<< setprecision(6) << is_goal.mean() << endl;
	}

	endpts_str << trial;
	for(unsigned int i= 0; i< agents; i++){
		endpts_str << "\t" << a(i)->x()<< "\t" << a(i)->y() << "\t" << a(i)->d();
	}
	endpts_str << endl;
}

void Simulation::writeTrialData(){
	agent_str << trial << "\t" << trial_t;						//1,2
	agent_str << "\t" << a(0)->x()<< "\t" << a(0)->y();			//3,4
	agent_str << "\t" << a(0)->d() << "\t" << a(0)->phi();		//5,6
	agent_str << "\t" << a(0)->v().ang() << "\t" << global_t;	//7,8
	if(lvlearn_on){
		for(int lm_i=0; lm_i < c()->K(); lm_i++)
			agent_str  << "\t" << c()->el_lm(lm_i);	// TODO: different streams for different agents
	}
	agent_str << endl;
	error_dist  << (a(0)->x() - a(0)->HV().x) << "\t" << (a(0)->y() - a(0)->HV().y) << endl;
	if(pin_on){
		homevector_str << trial_t << "\t" << global_t; 													//1,2
		homevector_str << "\t" << a(0)->HV().x << "\t" << a(0)->HV().y;									//3,4
		homevector_str << "\t" << a(0)->HVm().x << "\t" << a(0)->HVm().y; 								//5,6
		homevector_str << "\t" << a(0)->HV().ang() << "\t" << a(0)->HVm().ang(); 						//7,8
		homevector_str << "\t" <<  (a(0)->HV()-a(0)->v()).len() << "\t" <<  a(0)->HV().len();			//9,10
		homevector_str << "\t" << a(0)->d() << endl;	//11
	}
	if(gvlearn_on){
		globalvector_str << trial_t << "\t" << global_t;
		globalvector_str << "\t" << a(0)->GV().x << "\t" << a(0)->GV().y;						//3,4
		globalvector_str << "\t" << a(0)->GV().ang() << "\t" << a(0)->GV().len();				//5,6
		globalvector_str << "\t" << c()->expl(0) << "\t" << 1.0*count_goal;						//7,8
		globalvector_str << "\t" << c()->GV_vecavg() << endl;									//9
	}
	if(lvlearn_on){
		refvector_str << trial_t << "\t" << global_t;
		refvector_str << "\t" << c()->RV().x << "\t" << c()->RV().y << "\t" << c()->RV().ang() << "\t" << c()->RV().len() << endl;
		localvector_str << trial_t << "\t" << global_t;
		for(int lm_i=0; lm_i < c()->K(); lm_i++)
			localvector_str << "\t" << c()->LV(lm_i).x << "\t" << c()->LV(lm_i).y << "\t" << c()->LV(lm_i).ang() << "\t" << c()->LV(lm_i).len() << "\t" << c()->LV_vecavg(lm_i);
		localvector_str  << endl;
	}
	reward_str << trial_t << "\t" << global_t;
	reward_str << "\t" << c()->R(0) << "\t" << c()->v(0) << endl;
	length_scaling << a(0)->v().len() << "\t" << sum(a(0)->pi()->get_output()) << "\t" << a(0)->c()->N() << endl;
	out_signals << trial_t << "\t" << global_t << "\t";
	out_signals << c()->rand_w << "\t" << c()->rand_m << "\t";
	out_signals << c()->pi_w << "\t" << c()->pi_m << "\t";
	out_signals << c()->gl_w << "\t" << c()->gl_m << "\t";
	out_signals << c()->rl_w << "\t" << c()->rl_m << endl;

	if(lvlearn_on){
		lmr_signals << trial_t << "\t" << global_t << "\t";
		for(int lm_unit = 0; lm_unit < c()->K(); lm_unit++){
			lmr_signals << c()->LV_module()->state_lm(lm_unit) << "\t"		// 3
					<< c()->LV_module()->dstate_lm(lm_unit) << "\t"		// 4
					<< c()->LV_module()->cl_state_lm(lm_unit) << "\t"	// 5
					<< c()->el_lm(lm_unit) << "\t"						// 6
					<< c()->LV_value(lm_unit) << "\t"					// 7
					<< c()->LV_value_raw(lm_unit) << "\t";				// 8
		}
		lmr_signals << endl;
		lmr_angles << trial_t << "\t" << global_t << "\t";
		lmr_angles << a(0)->x()<< "\t" << a(0)->y() << "\t" ;			//3,4
		lmr_angles << e()->get_visible_LM_th(0) << "\t"<< a(0)->phi()<< "\t"  << sin(e()->get_visible_LM_th(0) - a(0)->phi().rad()) << "\t"; //5,6,7
		lmr_angles << 0.1*cos(e()->get_visible_LM_th(0)) << "\t" << 0.1*sin(e()->get_visible_LM_th(0)) << endl; // 8,9
	}

	if(gvlearn_on){
		adaptive_expl << trial_t << "\t" << global_t << "\t";								// 1,2
		adaptive_expl << trial << "\t" << c()->e_beta() << "\t";							// 3,4
		adaptive_expl << c()->v(0) << "\t" << avg_reward.mean()  << "\t";					// 5,6
		adaptive_expl << c()->expl(0)  << "\t" << 2. * c()->v(0) * c()->expl(0)  << endl;	// 7,8							//7
	}
}

