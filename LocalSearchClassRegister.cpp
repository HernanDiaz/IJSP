/*
 * LocalSearchClassRegister.h
 *
 *  Created on: Oct 13, 2017
 *      Author: Juan Jose Palacios
 */

#include "LocalSearchClassRegister.h"

namespace FuzzyFW {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, LocalSearch*(*)()>	LocalSearchClassRegister::LocalSearchMap;
	std::map<std::string, Neighbourhood*(*)()>	LocalSearchClassRegister::NeighbourhoodMap;

}
