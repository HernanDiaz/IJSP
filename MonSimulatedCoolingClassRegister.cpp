/*
 * SimulatedCoolingClassRegister.cpp
 *
 *  Created on: May 06, 2022
 *      Author: Hernan Diaz Rodriguez
 */

#include "MonSimulatedCoolingClassRegister.h"

namespace FuzzyFW {

/**
 * Initialize all static variables 
 */
	std::map<std::string, Monotonic_Adaptative_Cooling*(*)()>	MonSimulatedCoolingClassRegister::MACMap;
}
