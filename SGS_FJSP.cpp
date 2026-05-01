/*
* SGS_FJSP.cpp
*
*  Created on: June 1, 2017
*/

#include "SGS_FJSP.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class SGS_FJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FJSP::SGS_FJSP(const FuzzyFW::ParameterDB *params)
	: schedule(NULL), isCreated(false) {
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
SGS_FJSP::SGS_FJSP(const SGS_FJSP &source) {
	this->schedule = new ScheduleFJSP(*source.schedule);
	this->isCreated = source.isCreated;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Reset  ===============================================================
void SGS_FJSP::reset() {
	if (this->isCreated)
		this->schedule->reset();
}

}
