/*
* ScheduleIJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/
#include <cstdio>
using namespace std;
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



//====  verifyHeads Method  ===================================================
void ScheduleIJSP::verifyHeads(const FuzzyFW::Interval& expectedMakespan,
	const std::string& context) const {

	int n = (int)this->taskInfo.size();
	std::vector<FuzzyFW::Interval> computedHead(n, FuzzyFW::Interval(0, 0));
	FuzzyFW::Interval exactMakespan(0, 0);

	// Count how many predecessors each task is waiting for
	std::vector<int> remaining(n, 0);
	for (int t = 0; t < n; t++) {
		if (this->taskInfo[t].task == NULL) continue;
		if (this->taskInfo[t].task->jp != -1) remaining[t]++;
		if (this->taskInfo[t].mp != -1) remaining[t]++;
	}

	// Seed queue with root tasks (no predecessors)
	std::queue<int> q;
	for (int t = 0; t < n; t++) {
		if (this->taskInfo[t].task != NULL && remaining[t] == 0)
			q.push(t);
	}

	int processed = 0;
	while (!q.empty()) {
		int t = q.front();
		q.pop();
		processed++;

		int jp = this->taskInfo[t].task->jp;
		int mp = this->taskInfo[t].mp;

		FuzzyFW::Interval expected(0, 0);
		if (jp != -1 && mp != -1) {
			FuzzyFW::Interval fromJp = computedHead[jp] + this->taskInfo[jp].task->p;
			FuzzyFW::Interval fromMp = computedHead[mp] + this->taskInfo[mp].task->p;
			expected = FuzzyFW::Interval(
				std::max(fromJp.a, fromMp.a),
				std::max(fromJp.b, fromMp.b));
		} else if (jp != -1) {
			expected = computedHead[jp] + this->taskInfo[jp].task->p;
		} else if (mp != -1) {
			expected = computedHead[mp] + this->taskInfo[mp].task->p;
		}

		computedHead[t] = expected;

		// Compare stored head with recomputed head
		if (std::fabs(this->taskInfo[t].head.a - expected.a) > AccuracyError ||
			std::fabs(this->taskInfo[t].head.b - expected.b) > AccuracyError) {
			std::cerr << "[" << context << "] HEAD MISMATCH task " << t
				<< " stored=[" << this->taskInfo[t].head.a << ","
				<< this->taskInfo[t].head.b << "]"
				<< " expected=[" << expected.a << "," << expected.b << "]"
				<< std::endl;
		}

		// Accumulate exact makespan using recomputed heads
		double ct_a = expected.a + this->taskInfo[t].task->p.a;
		double ct_b = expected.b + this->taskInfo[t].task->p.b;
		if (ct_a > exactMakespan.a) exactMakespan.a = ct_a;
		if (ct_b > exactMakespan.b) exactMakespan.b = ct_b;

		// Decrement remaining count for successors
		int js = this->taskInfo[t].task->js;
		int ms = this->taskInfo[t].ms;
		if (js != -1) {
			remaining[js]--;
			if (remaining[js] == 0) q.push(js);
		}
		if (ms != -1) {
			remaining[ms]--;
			if (remaining[ms] == 0) q.push(ms);
		}
	}

	if (processed != (int)this->nScheduledTasks) {
		std::cerr << "[" << context << "] verifyHeads: possible cycle detected!"
			<< " processed=" << processed
			<< " nScheduledTasks=" << this->nScheduledTasks << std::endl;
	}

	// Compare exact makespan with neighbourhood-computed makespan
	if (std::fabs(exactMakespan.a - expectedMakespan.a) > AccuracyError ||
		std::fabs(exactMakespan.b - expectedMakespan.b) > AccuracyError) {
		std::cerr << "[" << context << "] MAKESPAN MISMATCH"
			<< " computed=[" << expectedMakespan.a << "," << expectedMakespan.b << "]"
			<< " exact=[" << exactMakespan.a << "," << exactMakespan.b << "]"
			<< std::endl;
	}
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

//====  repair times of the sheduling so there is no overlapping ===================================
void ScheduleIJSP::repairScheduledTimes(const int taskIdx) {
	if (taskIdx < 0 || taskIdx >=  this->taskInfo.size()) return;
	
	IJSP::ScheduledTaskInfo currentTask = this->taskInfo[taskIdx];
	IJSP::ScheduledTaskInfo* repairedTask;

	//repair job successor
	if (currentTask.task->js >= 0 && currentTask.task->js < this->taskInfo.size()) {
		repairedTask = &(this->taskInfo[currentTask.task->js]);
		if (repairedTask->task && adjustHead(currentTask, repairedTask)) {
			this->repairScheduledTimes(repairedTask->task->id);
		}
	}

	//repair machine successor
	if (currentTask.ms >=0 && currentTask.ms < this->taskInfo.size()) {
		repairedTask = &(this->taskInfo[currentTask.ms]);
		if (repairedTask -> task && adjustHead(currentTask, repairedTask)) {
			this->repairScheduledTimes(repairedTask->task->id);
		}
	}

}

//====  adjusts head of a succcessor after completion of the current task===========================
bool ScheduleIJSP::adjustHead(const ScheduledTaskInfo currentTask, ScheduledTaskInfo* successor) {
	FuzzyFW::Interval completionTime = currentTask.head + currentTask.task->p;
	bool repairNeed = false;
	
	if (successor->head.a < 0 || successor->head.b < 0) return false;

	if (completionTime.a > successor->head.a) {
		successor->head.a = completionTime.a;
	    repairNeed = true;
	}
	if (completionTime.b > successor->head.b) {
	   successor->head.b = completionTime.b;
    	repairNeed = true;
	}

	return repairNeed && successor->task;
}


void ScheduleIJSP::verifyScheduling() {
	//We verify task by task that the job restrictions are correct
	FuzzyFW::Interval::Compare comp = FuzzyFW::Interval::C_COMPONENT;
	for (int i = 0; i < this->lastTaskJob.size(); i++) {
		ScheduledTaskInfo currentTask = this->taskInfo[this->lastTaskJob[i]];
		while (currentTask.task->jp >= 0 && currentTask.task->jp < this->taskInfo.size()) {
			ScheduledTaskInfo jobPredeccessor = this->taskInfo[currentTask.task->jp];
			if (currentTask.head.isLesserThan(jobPredeccessor.head + jobPredeccessor.task->p, comp)) {
				throw new IJSPException("Schedule", "Error scheduling tasks in job number" + valueToString(jobPredeccessor.task->job) +
					": id = " + valueToString(jobPredeccessor.task->id) + "; head = " + valueToString(jobPredeccessor.head) +
					"; p = " + valueToString(jobPredeccessor.task->p) + "; total = " + valueToString(jobPredeccessor.head + jobPredeccessor.task->p) +
					" AND id = " + valueToString(currentTask.task->id) + "; head = " + valueToString(currentTask.head));
					
			}
			currentTask = jobPredeccessor;
		}
		

	}
	//We verify task by task that the machine restrictions are correct
	for (int i = 0; i < this->lastTaskMachine.size(); i++) {
		ScheduledTaskInfo currentTask = this->taskInfo[this->lastTaskMachine[i]];
		while (currentTask.mp >= 0 && currentTask.mp < this->taskInfo.size()) {
			ScheduledTaskInfo machinePredeccessor = this->taskInfo[currentTask.mp];
			if (currentTask.head.isLesserThan(machinePredeccessor.head + machinePredeccessor.task->p, comp)) {
				throw new IJSPException("Schedule", "Error scheduling tasks in machine" + valueToString(machinePredeccessor.task->machine) +
					": id = " + valueToString(machinePredeccessor.task->id) + "; head = " + valueToString(machinePredeccessor.head) +
					"; p = " + valueToString(machinePredeccessor.task->p) + "; total = " + valueToString(machinePredeccessor.head + machinePredeccessor.task->p) +
					" AND id = " + valueToString(currentTask.task->id) + "; head = " + valueToString(currentTask.head));
			}
			currentTask = machinePredeccessor;
		}

	}
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
