/*****************************************************************************
 *  pin.h                                                                    *
 *                                                                           *
 *  Created on:   Jul 23, 2014                                               *
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

#ifndef PIN_H_
#define PIN_H_

#include "circulararray.h"
#include "geom.h"
#include <armadillo>
#include <vector>
using namespace std;
using namespace arma;



enum{HD, G, M, PI};    	//HD = Head Direction; G = Gater; M = Memory; PI = Home vector output


/**
 * Path Integration Network Class
 *
 * 	This class creates a network of circular array
 * 	for path integration
 *
 */

class PIN:
		public CircArray
{
public:

	/**
	 * Constructor
	 *
	 *  @param (int) num_neurons: number of neurons in this array (default: 360)
	 *  @param (int) input_dim: number of incoming signals (default: 0)
	 */
	PIN(int num_neurons = 36, double leak = 0.0, double sens_noise = 0.0, double neur_noise = 0.0, bool in_silent=false);

	/**
	 * Destructor
	 *
	 */
	~PIN();

	CircArray* array(int i);

	vec get_output();

	/**
	 * Return the home vector using average
	 *
	 * @return (Vec)
	 */
	Vec HV();

	/**
	 * Return the home vector using maximum
	 *
	 * @return (Vec)
	 */
	Vec HVm();

	/**
	 * Resets the activities of the path integration network
	 *
	 *  @return (void)
	 */
	void reset();

	/**
	 * Updates the path integration network
	 *
	 *  @param (Angle) angle: Input angle from compass
	 *  @param (double) speed: Input walking speed from odometry (legged: "differential step counter")
	 *  @return (void)
	 */
	void update(Angle angle, double speed);

	/**
	 * Returns the PI x coordinate
	 *
	 * @return (double)
	 */
	double x();

	/**
	 * Returns the PI y coordinate
	 *
	 * @return (double)
	 */
	double y();

	bool SILENT;
	bool VERBOSE;

private:
	vector<CircArray*> ar;

	Vec home_vector;
	Vec home_vector_max;

	mat w_cos;
	double leak_rate;
	double snoise;
	double nnoise;
	int t_step;
};



#endif /* PIN_H_ */
