/*
 * StatisticsClassRegister.h
 *
 *  Created on: Oct 4, 2017
 */

#include "StatisticsClassRegister.h"

namespace FuzzyFW {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, Statistics*(*)()>	StatisticsClassRegister::StatsMap;

}
