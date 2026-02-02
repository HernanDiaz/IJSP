/*
 * SimulatedCoolingClassRegister.cpp
 *
 *  Created on: May 06, 2022
 *      Author: Hernan Diaz Rodriguez
 */

#include "NonMonSimulatedCoolingClassRegister.h"

namespace FuzzyFW {

/**
 * Initialize all static variables 
 */
	std::map<std::string, Non_Monotonic_Adaptative_Cooling*(*)()>	NonMonSimulatedCoolingClassRegister::NMACMap;

}
