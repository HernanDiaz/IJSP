/*
* SGS_JSP_Append.cpp
*
*  Created on: Jul 7, 2019
*      Author: Hernan Diaz Rodriguez
*/

#include "SGS_JSP_Append.h"

namespace JSP {

//=============================================================================
//
//	Class SGS_JSP_Append
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_JSP_Append::SGS_JSP_Append(const FuzzyFW::ParameterDB *params)
	: SGS_JSP(params) {
}


//=====  Copy constructor  ====================================================
SGS_JSP_Append::SGS_JSP_Append(const SGS_JSP_Append &source)
	: SGS_JSP(source) { }


//=====  Setup method  ========================================================
void SGS_JSP_Append::setup(const FuzzyFW::ParameterDB *params) {
	SGS_JSP::setup(params);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Schedule a task  =====================================================
FuzzyFW::Crisp SGS_JSP_Append::scheduleTask(const TaskJSP *task,
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
		throw JSPException("SGS", errorMsg);
	}

	// Starting time
	FuzzyFW::Crisp Stime = FuzzyFW::Crisp(0);
	if (jp != -1)
		Stime = this->schedule->taskInfo[jp].head +
		this->schedule->taskInfo[jp].task->p;

	if (mp != -1)
		Stime = std::max(Stime, this->schedule->taskInfo[mp].head +
			this->schedule->taskInfo[mp].task->p);

	// Update the schedule
	this->schedule->addTask(taskIdx, Stime, -1);
	return Stime;
}
}
