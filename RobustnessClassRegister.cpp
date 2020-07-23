/*
 * RobustnessClassRegister.cpp
 *
 *  Created on: Jul 07, 2019
 *      Author: Hernan Diaz Rodriguez
 */

#include "RobustnessClassRegister.h"

namespace PostExecution {

/**
 * Initialize all static variables
 */
	std::map<std::string, RobustnessAnalyzer*(*)()>	RobustnessClassRegister::SGSMap;
}
