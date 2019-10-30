

/*
* SGS_IJSP_Insertion.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "SGS_IJSP_Insertion.h"
#include <iostream>
using namespace std;

namespace IJSP {

	//=============================================================================
	//
	//	Class SGS_IJSP_Insertion
	//
	//=============================================================================
	//=============================================================================
	//		METHODS
	//=============================================================================
	//=====  Build schedule  ======================================================
	ScheduleIJSP * SGS_IJSP_Insertion::buildSchedule(
		const FuzzyFW::SharedVars * svars, std::vector<int> &order) {

		if (svars->problem == NULL)
			throw new IJSPException("SGS", "Problem instance not created");

		ProblemIJSP * fjspProb =
			dynamic_cast<ProblemIJSP *>(svars->problem);
		if (fjspProb == NULL) {
			std::string errorMsg = "This SGS can be only used on Interval Problems.";
			throw new IJSPException("SGS", errorMsg);
		}

		if (this->isCreated)
			this->schedule->reset();
		else {
			this->schedule = new ScheduleIJSP(fjspProb);
			this->isCreated = true;
		}

		for (size_t i = 0; i < order.size(); i++) {
			this->scheduleTask((*fjspProb)[order[i]], order[i]);
		}
		this->schedule->verifyScheduling();

		return this->schedule;
	}


	//=====  Schedule a task  =====================================================
	FuzzyFW::Interval SGS_IJSP_Insertion::scheduleTask(const TaskIJSP *task,
		const int taskIdx) {

		int mp, ms; // Machine predecessor and successor
		FuzzyFW::Interval Stime;	// Starting time
		FuzzyFW::Interval mtHead, mtPT;
		char found;	// Big gap found in the schedule
		// Maxims are made component by component
		FuzzyFW::Interval::Maximum maxComp = FuzzyFW::Interval::M_COMPONENT;

		int mac = task->machine;
		int job = task->job;

		// Check if this task can be scheduled
		if (task->jp != this->schedule->lastTaskJob[job]) {
			std::string errorMsg;
			errorMsg = "Job precedence constraint is being violated. Scheduling ";
			errorMsg += "task " + valueToString(taskIdx) + " after task ";
			errorMsg += valueToString(this->schedule->lastTaskJob[job]);
			throw new IJSPException("SGS", errorMsg);
		}

		// If I could schedule the task just after its job predecessor, who
		// would be the machine predecessor and successors...
		Stime = this->schedule->getCTJob(job);
		ms = -1;
		mp = this->schedule->lastTaskMachine[mac];
		if (mp != -1)
			mtHead = this->schedule->taskInfo[mp].head;

		while (mp != -1 && mtHead.isGreaterEqualTo(Stime, this->cpComp)) {
			ms = mp;
			mp = this->schedule->taskInfo[ms].mp;
			if (mp != -1)
				mtHead = this->schedule->taskInfo[mp].head;
		}

		// Update the heuristic Starting time that task could take
		if (mp != -1) {
			mtHead = this->schedule->taskInfo[mp].head;
			mtPT = this->schedule->taskInfo[mp].task->p;
			Stime = maximum(Stime, mtHead + mtPT, maxComp);
		}


		// Look for the actual minimum starting time for the task
		
		found = false;
		while (!found && ms != -1) {
			mtHead = this->schedule->taskInfo[ms].head;
			mtPT = this->schedule->taskInfo[ms].task->p;

			if (mtHead.isGreaterEqualTo(Stime + task->p, this->cpComp))
				found = true;
			else {
				mp = ms;
				Stime = maximum(Stime, mtHead + mtPT, maxComp);
				ms = this->schedule->taskInfo[mp].ms;
			}
		}

		// Update the schedule
		this->schedule->addTask(taskIdx, Stime, ms);

		// Repair schedule
		this->schedule->repairScheduledTimes(taskIdx);

		return Stime;
	}

}

