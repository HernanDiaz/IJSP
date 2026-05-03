/*
* SGS_IJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "SGS_IJSP.h"
#include "ProblemIJSP.h"
#include "IJSPException.h"

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
ScheduleIJSP * SGS_IJSP::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	ProblemIJSP *prob = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (prob == nullptr)
		throw IJSPException("SGS", "This SGS can be only used on Interval Problems.");

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleIJSP(prob);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++)
		this->scheduleTask((*prob)[order[i]], order[i]);

	this->postBuild();
	return this->schedule;
}


void SGS_IJSP::reset() {
	if (this->isCreated)
		this->schedule->reset();
}



}
