/*
* DueDateClassRegister.h
*
*  Created on: May 17, 2017
*      Author: Juan Jose Palacios
*/

#include "DueDateClassRegister.h"

namespace FJSP {

	/**
	* Initialize all static variables in SchedulingClassRegister
	*/
	std::map<std::string, DueDate*(*)()>	DueDateClassRegister::DueDateMap;


}
