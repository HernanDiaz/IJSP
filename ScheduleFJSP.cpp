/*
* ScheduleFJSP.cpp
*
*  Created on: May 16, 2017
*      Author: Juan Jose Palacios
*/

#include "ScheduleFJSP.h"


namespace FJSP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Main constructor  =====================================================
ScheduleFJSP::ScheduleFJSP(const ProblemFJSP * problem) {
	this->nScheduledTasks = 0;
	this->tailsUpdated = true;
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
ScheduleFJSP::ScheduleFJSP(const ScheduleFJSP & source) {
	this->nScheduledTasks = source.nScheduledTasks;
	this->tailsUpdated = source.tailsUpdated;
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
std::vector<int> & ScheduleFJSP::getTaskOrder() {
	if (!this->isSorted)
		this->updateTopologicalOrder();
	return this->taskOrder;
}


//====  Get Machine Completion Time  ==========================================
FuzzyFW::TFN ScheduleFJSP::getCTMachine(const unsigned int machine) const {
	if (machine < 0 || machine >= this->problem->getNumberMachines()) {
		std::string errorMsg = "Trying to access unexisting machine: ";
		errorMsg += valueToString(machine);
		throw new FJSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskMachine[machine];
	if (lastTask < 0)
		return FuzzyFW::TFN(0, 0, 0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}


//====  Get Job Completion Time  ==============================================
FuzzyFW::TFN ScheduleFJSP::getCTJob(const unsigned int job) const {
	if (job < 0 || job >= this->problem->getNumberJobs()) {
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw new FJSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskJob[job];
	if (lastTask < 0)
		return FuzzyFW::TFN(0, 0, 0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}





//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
ScheduleFJSP & ScheduleFJSP::operator=(const ScheduleFJSP & source) {
	this->nScheduledTasks = source.nScheduledTasks;
	this->tailsUpdated = source.tailsUpdated;
	this->isSorted = source.isSorted;

	this->problem = source.problem;
	this->lastTaskJob = source.lastTaskJob;
	this->lastTaskMachine = source.lastTaskMachine;
	this->taskInfo = source.taskInfo;
	this->taskOrder = source.taskOrder;
	return *this;
}


//====  Index access  =========================================================
const TaskFJSP * ScheduleFJSP::operator[](const unsigned int index) const {
	if (index < 0) {
		std::string errorMsg = "Trying to access unexisting task: ";
		errorMsg += valueToString(index);
		throw new FJSPException("Schedule", errorMsg);
	}
	if (index >= this->nScheduledTasks)
		return NULL;
	return this->taskInfo[index].task;
}





//=============================================================================
//		METHODS
//=============================================================================
//====  addTask Method  =======================================================
void ScheduleFJSP::addTask(const int taskIdx, FuzzyFW::TFN & ST,
	const int macSuc) {

	const TaskFJSP * task = (*(this->problem))[taskIdx];
	int mac = task->machine;
	int job = task->job;
	int macPred;
	FuzzyFW::TFN::Compare cev = FuzzyFW::TFN::C_EV;

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

	this->tailsUpdated = false;
	this->nScheduledTasks++;
}



//====  updateTails Method  ===================================================
void ScheduleFJSP::updateTails(const FuzzyFW::TFN::Maximum maxType) {
	if (this->tailsUpdated)
		return;

	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;

	std::vector<char> visited(this->problem->getNumberTasks(), false);

	// Look for tasks with no successors
	for (size_t i = 0; i < this->lastTaskJob.size(); i++) {
		taskIdx = this->lastTaskJob[i];
		mac = this->taskInfo[taskIdx].task->machine;
		if (taskIdx == this->lastTaskMachine[mac])
			taskQueue.push(taskIdx);
	}

	// Backwards propagation
	while (taskQueue.size() > 0) {
		taskIdx = taskQueue.front();
		taskQueue.pop();

		// Update tail
		ms = this->taskInfo[taskIdx].ms;
		js = this->taskInfo[taskIdx].task->js;

		if (ms != -1 && js != -1)
			this->taskInfo[taskIdx].tail =
			maximum(this->taskInfo[ms].tail + this->taskInfo[ms].task->p,
				this->taskInfo[js].tail + this->taskInfo[js].task->p, maxType);
		else if (ms != -1)
			this->taskInfo[taskIdx].tail =
			this->taskInfo[ms].tail + this->taskInfo[ms].task->p;
		else if (js != -1)
			this->taskInfo[js].tail + this->taskInfo[js].task->p;
		else this->taskInfo[taskIdx].tail = FuzzyFW::TFN(0, 0, 0);

		// Propagate
		mp = this->taskInfo[taskIdx].mp;
		jp = this->taskInfo[taskIdx].task->jp;

		if (mp != -1) {
			if (visited[mp] || this->taskInfo[mp].task->js == -1)
				taskQueue.push(mp);
			else visited[mp] = true;
		}

		if (jp != -1) {
			if (visited[jp] || this->taskInfo[jp].ms == -1)
				taskQueue.push(jp);
			else visited[jp] = true;
		}
	}

	this->tailsUpdated = true;
}



//====  reset Method  =========================================================
void ScheduleFJSP::reset() {
	if (this->nScheduledTasks <= 0)
		return;

	this->nScheduledTasks = 0;
	this->tailsUpdated = true;
	this->isSorted = true;

	for (size_t i = 0; i < this->taskInfo.size(); i++) {
		this->taskInfo[i].head = FuzzyFW::TFN(-1, -1, -1);
		this->taskInfo[i].tail = FuzzyFW::TFN(0, 0, 0);
		this->taskInfo[i].mp = this->taskInfo[i].mp = -1;
		this->taskOrder[i] = -1;
	}

	for (size_t i = 0; i < this->lastTaskMachine.size(); i++)
		this->lastTaskMachine[i] = -1;
	for (size_t i = 0; i < this->lastTaskJob.size(); i++)
		this->lastTaskJob[i] = -1;
}



//====  update Topological Order  =============================================
void ScheduleFJSP::updateTopologicalOrder() {
	if (this->isSorted)
		return;
	this->quicksortTasks(0, this->nScheduledTasks - 1, new FuzzyFW::Random());
	this->isSorted = true;
}


//====  apply quicksort to sort tasks  ========================================
void ScheduleFJSP::quicksortTasks(int left, int right, FuzzyFW::Random *rng) {
	int pivot;
	FuzzyFW::TFN pivotValue;
	FuzzyFW::TFN::Compare cp = FuzzyFW::TFN::C_EV;

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
