/*
 * FVRPClassRegister.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: Juan Jose Palacios
 */

#include "FVRPClassRegister.h"

namespace FVRP {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, SGS_FVRP*(*)()>	FVRPClassRegister::SGSMap;


}
