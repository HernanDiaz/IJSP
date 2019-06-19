/*
* SGS_FJSP.cpp
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#include "SGS_IJSP.h"

namespace IJSP {

//=============================================================================
//
//	Abstract class SGS_FJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_IJSP::SGS_IJSP(const FuzzyFW::ParameterDB *params)
	: schedule(NULL), isCreated(false) {
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
SGS_IJSP::SGS_IJSP(const SGS_IJSP &source) {
	this->schedule = new ScheduleIJSP(*source.schedule);
	this->isCreated = source.isCreated;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Reset  ===============================================================
void SGS_IJSP::reset() {
	if (this->isCreated)
		this->schedule->reset();
}

}
