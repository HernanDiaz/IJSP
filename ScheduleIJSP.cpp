/*
* ScheduleIJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "ScheduleIJSP.h"


namespace IJSP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Main constructor  =====================================================
ScheduleIJSP::ScheduleIJSP(const ProblemIJSP * problem) {
	this->nScheduledTasks = 0;
	this->isSorted = true;

	this->problem = problem;
	if (problem != NULL) {
		this->lastTaskJob.resize(problem->getNumberJobs(), -1);
		this->lastTaskMachine.resize(problem->getNumberMachines(), -1);
		this->taskInfo.resize(problem->getNumberTasks());
		this->taskOrder.resize(problem->getNumberTasks(), -1);
	}
}


//====  Copy constructor  =====================================================
ScheduleIJSP::ScheduleIJSP(const ScheduleIJSP & source) {
	this->nScheduledTasks = source.nScheduledTasks;
	this->isSorted = source.isSorted;

	this->problem = source.problem;
	this->lastTaskJob = source.lastTaskJob;
	this->lastTaskMachine = source.lastTaskMachine;
	this->taskInfo = source.taskInfo;
	this->taskOrder = source.taskOrder;
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get topological order  ================================================
std::vector<int> & ScheduleIJSP::getTaskOrder(FuzzyFW::Random *rng) {
	if (!this->isSorted)
		this->updateTopologicalOrder(rng);
	return this->taskOrder;
}


//====  Get Machine Completion Time  ==========================================
FuzzyFW::Interval ScheduleIJSP::getCTMachine(const unsigned int machine) const {
	if (machine < 0 || machine >= this->problem->getNumberMachines()) {
		std::string errorMsg = "Trying to access unexisting machine: ";
		errorMsg += valueToString(machine);
		throw new IJSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskMachine[machine];
	if (lastTask < 0)
		return FuzzyFW::Interval(0, 0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}


//====  Get Job Completion Time  ==============================================
FuzzyFW::Interval ScheduleIJSP::getCTJob(const unsigned int job) const {
	if (job < 0 || job >= this->problem->getNumberJobs()) {
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw new IJSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskJob[job];
	if (lastTask < 0)
		return FuzzyFW::Interval(0, 0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}





//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
ScheduleIJSP & ScheduleIJSP::operator=(const ScheduleIJSP & source) {
	this->nScheduledTasks = source.nScheduledTasks;
	this->isSorted = source.isSorted;

	this->problem = source.problem;
	this->lastTaskJob = source.lastTaskJob;
	this->lastTaskMachine = source.lastTaskMachine;
	this->taskInfo = source.taskInfo;
	this->taskOrder = source.taskOrder;
	return *this;
}


//====  Index access  =========================================================
const TaskIJSP * ScheduleIJSP::operator[](const unsigned int index) const {
	if (index < 0) {
		std::string errorMsg = "Trying to access unexisting task: ";
		errorMsg += valueToString(index);
		throw new IJSPException("Schedule", errorMsg);
	}
	if (index >= this->nScheduledTasks)
		return NULL;
	return this->taskInfo[index].task;
}





//=============================================================================
//		METHODS
//=============================================================================
//====  addTask Method  =======================================================
void ScheduleIJSP::addTask(const int taskIdx, FuzzyFW::Interval & ST,
	const int macSuc) {

	const TaskIJSP * task = (*(this->problem))[taskIdx];
	int mac = task->machine;
	int job = task->job;
	int macPred;
	FuzzyFW::Interval::Compare cev = FuzzyFW::Interval::C_EV;

	this->taskInfo[taskIdx].task = task;
	this->taskInfo[taskIdx].head = ST;
	this->taskInfo[taskIdx].ms = macSuc;

	// Update its predeccessor
	if (macSuc != -1) {
		macPred = this->taskInfo[macSuc].mp;
		this->taskInfo[macSuc].mp = taskIdx;
	}
	else {
		macPred = this->lastTaskMachine[mac];
		this->lastTaskMachine[mac] = taskIdx;
	}
	this->taskInfo[taskIdx].mp = macPred;

	if (macPred != -1)
		this->taskInfo[macPred].ms = taskIdx;
	this->lastTaskJob[job] = taskIdx;


	// Keep the tasks sorted. In FJSP is very likely that the new task will
	// be at the end, which makes this worth. Otherwise, it would be better
	// to leave them unsorted and use quicksort at the end
	if (this->isSorted) {
		int i;
		for (i = this->nScheduledTasks; i > 0; i--) {
			this->taskOrder[i] = this->taskOrder[i - 1];

			if (this->taskInfo[this->taskOrder[i]].head.
				isLesserThan(ST, cev)) {
				this->taskOrder[i] = taskIdx;
				break;
			}

			if (this->taskInfo[this->taskOrder[i]].head.isEqualTo(ST, cev)
				&& taskIdx < this->taskOrder[i]) {
				this->taskOrder[i] = taskIdx;
				break;
			}
		}
		if (i == 0)
			this->taskOrder[0] = taskIdx;
	}
	else
		this->taskOrder[this->nScheduledTasks] = taskIdx;

	this->nScheduledTasks++;
}



//====  reset Method  =========================================================
void ScheduleIJSP::reset() {
	if (this->nScheduledTasks <= 0)
		return;

	this->nScheduledTasks = 0;
	this->isSorted = true;

	for (size_t i = 0; i < this->taskInfo.size(); i++) {
		this->taskInfo[i].head = FuzzyFW::Interval(-1, -1);
		this->taskInfo[i].mp = this->taskInfo[i].mp = -1;
		this->taskOrder[i] = -1;
	}

	for (size_t i = 0; i < this->lastTaskMachine.size(); i++)
		this->lastTaskMachine[i] = -1;
	for (size_t i = 0; i < this->lastTaskJob.size(); i++)
		this->lastTaskJob[i] = -1;
}



//====  update Topological Order  =============================================
void ScheduleIJSP::updateTopologicalOrder(FuzzyFW::Random *rng) {
	if (this->isSorted)
		return;
	if(rng == NULL)
		this->quicksortTasks(0, this->nScheduledTasks - 1, new FuzzyFW::Random());
	else
		this->quicksortTasks(0, this->nScheduledTasks - 1, rng);
	this->isSorted = true;
}


//====  apply quicksort to sort tasks  ========================================
void ScheduleIJSP::quicksortTasks(int left, int right, FuzzyFW::Random *rng) {
	int pivot;
	FuzzyFW::Interval pivotValue;
	FuzzyFW::Interval::Compare cp = FuzzyFW::Interval::C_EV;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);
	pivotValue = this->taskInfo[this->taskOrder[pivot]].head;

	std::swap(this->taskOrder[pivot], this->taskOrder[right]);
	pivot = left;
	for (int i = left; i < right; i++) {
		if (this->taskInfo[this->taskOrder[i]].head.
			isLesserThan(pivotValue, cp)) {
			std::swap(this->taskOrder[i], this->taskOrder[pivot]);
			pivot++;
		}
		else if (this->taskInfo[this->taskOrder[i]].head.
			isEqualTo(pivotValue, cp)
			&& this->taskOrder[i] < this->taskOrder[right]) {
			std::swap(this->taskOrder[i], this->taskOrder[pivot]);
			pivot++;
		}
	}

	std::swap(this->taskOrder[pivot], this->taskOrder[right]);
	quicksortTasks(left, pivot - 1, rng);
	quicksortTasks(pivot + 1, right, rng);
}

}
