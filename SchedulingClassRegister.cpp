/*
 * tableOfTypes.h
 *
 *  Created on: May 17, 2017
 *      Author: Juan Jose Palacios
 */

#include "SchedulingClassRegister.h"

namespace FJSP {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, DueDate*(*)()>	SchedulingClassRegister::DueDateMap;
	std::map<std::string, FuzzySGS*(*)()>	SchedulingClassRegister::SGSMap;


}
