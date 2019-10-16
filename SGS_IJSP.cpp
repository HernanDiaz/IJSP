/*
* SGS_IJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "SGS_IJSP.h"

namespace IJSP {

//=============================================================================
//
//	Abstract class SGS_IJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_IJSP::SGS_IJSP(const FuzzyFW::ParameterDB *params)
	: schedule(NULL), isCreated(false), compareLabel(IJSP_SGS_COMPARE){
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
SGS_IJSP::SGS_IJSP(const SGS_IJSP &source)
	: compareLabel(source.compareLabel) {
	this->schedule = new ScheduleIJSP(*source.schedule);
	this->isCreated = source.isCreated;
	this->cpComp = source.cpComp;
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
