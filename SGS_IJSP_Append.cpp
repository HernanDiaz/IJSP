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
	: SGS_IJSP(params) {
}


//=====  Copy constructor  ====================================================
SGS_IJSP_Append::SGS_IJSP_Append(const SGS_IJSP_Append &source)
	: SGS_IJSP(source) { }


//=====  Setup method  ========================================================
void SGS_IJSP_Append::setup(const FuzzyFW::ParameterDB *params) {
	SGS_IJSP::setup(params);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Schedule a task  =====================================================
FuzzyFW::Crisp SGS_IJSP_Append::scheduleTask(const TaskIJSP *task,
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
		throw IJSPException("SGS", errorMsg);
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
