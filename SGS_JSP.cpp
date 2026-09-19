/*
* SGS_JSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "SGS_JSP.h"
#include "ProblemJSP.h"
#include "JSPException.h"

namespace JSP {

//=============================================================================
//
//	Abstract class SGS_JSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_JSP::SGS_JSP(const FuzzyFW::ParameterDB *params)
	: schedule(NULL), isCreated(false) {
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
SGS_JSP::SGS_JSP(const SGS_JSP &source) {
	this->schedule = new ScheduleJSP(*source.schedule);
	this->isCreated = source.isCreated;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Reset  ===============================================================
ScheduleJSP * SGS_JSP::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	ProblemJSP *prob = dynamic_cast<ProblemJSP *>(svars->problem);
	if (prob == nullptr)
		throw JSPException("SGS", "This SGS can be only used on Crisp Problems.");

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleJSP(prob);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++)
		this->scheduleTask((*prob)[order[i]], order[i]);

	this->postBuild();
	return this->schedule;
}


void SGS_JSP::reset() {
	if (this->isCreated)
		this->schedule->reset();
}



}
