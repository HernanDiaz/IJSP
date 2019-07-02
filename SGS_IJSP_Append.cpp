/*
* SGS_IJSP_Append.cpp
*
*  Created on: Jul 7, 2019
*      Author: Hernan Diaz Rodriguez
*/

#include "SGS_IJSP_Append.h"

namespace IJSP {

//=============================================================================
//
//	Class SGS_IJSP_Append
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_IJSP_Append::SGS_IJSP_Append(const FuzzyFW::ParameterDB *params)
	: maximumLabel(FJSP_SGS_APPEND_MAXIMUM), intervalMaximum(FuzzyFW::Interval::M_COMPONENT),
	SGS_IJSP(params) {
}


//=====  Copy constructor  ====================================================
SGS_IJSP_Append::SGS_IJSP_Append(const SGS_IJSP_Append &source)
	: maximumLabel(source.maximumLabel),
	intervalMaximum(source.intervalMaximum),
	SGS_IJSP(source) { }


//=====  Setup method  ========================================================
void SGS_IJSP_Append::setup(const FuzzyFW::ParameterDB *params) {
	SGS_IJSP::setup(params);

	// Load maximum type parameter
	std::string maxName = params->getStringUpper(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw new IJSPException("SGS", errorMsg);
	}
	this->intervalMaximum = FuzzyFW::Interval::getMaximum(maxName);
	if (this->intervalMaximum == FuzzyFW::Interval::M_Err) {
		std::string errorMsg = "Invalid value for parameter";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw new IJSPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleIJSP * SGS_IJSP_Append::buildSchedule(const FuzzyFW::SharedVars * svars,
	std::vector<int> &order) {

	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Interval Problems.";
		throw new IJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleIJSP(fuzzyProb);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++) {
		this->scheduleTask((*fuzzyProb)[order[i]], order[i]);
	}

	return this->schedule;
}



//=====  Schedule a task  =====================================================
FuzzyFW::Interval SGS_IJSP_Append::scheduleTask(const TaskIJSP *task,
	const int taskIdx) {

	// Machine predecessor
	int mp = this->schedule->lastTaskMachine[task->machine];

	// Job predecessor
	int jp = this->schedule->lastTaskJob[task->job];

	// Check if this task can be scheduled
	if (task->jp != jp) {
		std::string errorMsg;
		errorMsg = "Job precedence constraint is being violated. Scheduling ";
		errorMsg += "task " + valueToString(taskIdx) + " after task ";
		errorMsg += valueToString(jp);
		throw new IJSPException("SGS", errorMsg);
	}

	// Starting time
	FuzzyFW::Interval Stime = FuzzyFW::Interval(0, 0);
	if (jp != -1)
		Stime = this->schedule->taskInfo[jp].head +
		this->schedule->taskInfo[jp].task->p;

	if (mp != -1)
		Stime = maximum(Stime, this->schedule->taskInfo[mp].head +
			this->schedule->taskInfo[mp].task->p, this->intervalMaximum);

	// Update the schedule
	this->schedule->addTask(taskIdx, Stime, -1);
	return Stime;
}
}
