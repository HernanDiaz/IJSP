

/*
* SGS_IJSP_Insertion.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "SGS_IJSP_Insertion.h"

namespace IJSP {

	//=============================================================================
	//
	//	Class SGS_IJSP_Insertion
	//
	//=============================================================================
	//=============================================================================
	//		METHODS
	//=============================================================================
	//=====  Post-build hook  =====================================================
	void SGS_IJSP_Insertion::postBuild() {
		this->schedule->verifyScheduling();
	}


	//=====  Schedule a task  =====================================================
	FuzzyFW::Crisp SGS_IJSP_Insertion::scheduleTask(const TaskIJSP *task,
		const int taskIdx) {

		int mp, ms; // Machine predecessor and successor
		FuzzyFW::Crisp Stime;	// Starting time
		FuzzyFW::Crisp mtHead, mtPT;
		char found;	// Big gap found in the schedule
		// Maxims are made component by component

		int mac = task->machine;
		int job = task->job;

		// Check if this task can be scheduled
		if (task->jp != this->schedule->lastTaskJob[job]) {
			std::string errorMsg;
			errorMsg = "Job precedence constraint is being violated. Scheduling ";
			errorMsg += "task " + valueToString(taskIdx) + " after task ";
			errorMsg += valueToString(this->schedule->lastTaskJob[job]);
			throw IJSPException("SGS", errorMsg);
		}

		// If I could schedule the task just after its job predecessor, who
		// would be the machine predecessor and successors...
		Stime = this->schedule->getCTJob(job);
		ms = -1;
		mp = this->schedule->lastTaskMachine[mac];
		if (mp != -1)
			mtHead = this->schedule->taskInfo[mp].head;

		while (mp != -1 && mtHead >= Stime) {
			ms = mp;
			mp = this->schedule->taskInfo[ms].mp;
			if (mp != -1)
				mtHead = this->schedule->taskInfo[mp].head;
		}

		// Update the heuristic Starting time that task could take
		if (mp != -1) {
			mtHead = this->schedule->taskInfo[mp].head;
			mtPT = this->schedule->taskInfo[mp].task->p;
			Stime = std::max(Stime, mtHead + mtPT);
		}


		// Look for the actual minimum starting time for the task
		
		found = false;
		while (!found && ms != -1) {
			mtHead = this->schedule->taskInfo[ms].head;
			mtPT = this->schedule->taskInfo[ms].task->p;

			if (mtHead >= (Stime + task->p))
				found = true;
			else {
				mp = ms;
				Stime = std::max(Stime, mtHead + mtPT);
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

