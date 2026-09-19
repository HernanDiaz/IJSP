/*
* ScheduleJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/
#include <cstdio>
using namespace std;
#include "ScheduleJSP.h"

namespace JSP {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Main constructor  =====================================================
ScheduleJSP::ScheduleJSP(const ProblemJSP * problem) {
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
ScheduleJSP::ScheduleJSP(const ScheduleJSP & source) {
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
std::vector<int> & ScheduleJSP::getTaskOrder(FuzzyFW::Random *rng) {
	if (!this->isSorted)
		this->updateTopologicalOrder(rng);
	return this->taskOrder;
}


//====  Get Machine Completion Time  ==========================================
FuzzyFW::Crisp ScheduleJSP::getCTMachine(const unsigned int machine) const {
	if (machine < 0 || machine >= this->problem->getNumberMachines()) {
		std::string errorMsg = "Trying to access unexisting machine: ";
		errorMsg += valueToString(machine);
		throw JSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskMachine[machine];
	if (lastTask < 0)
		return FuzzyFW::Crisp(0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}


//====  Get Job Completion Time  ==============================================
FuzzyFW::Crisp ScheduleJSP::getCTJob(const unsigned int job) const {
	if (job < 0 || job >= this->problem->getNumberJobs()) {
		std::string errorMsg = "Trying to access unexisting job: ";
		errorMsg += valueToString(job);
		throw JSPException("Schedule", errorMsg);
	}

	int lastTask = this->lastTaskJob[job];
	if (lastTask < 0)
		return FuzzyFW::Crisp(0);
	return this->taskInfo[lastTask].head + this->taskInfo[lastTask].task->p;
}





//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
ScheduleJSP & ScheduleJSP::operator=(const ScheduleJSP & source) {
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
const TaskJSP * ScheduleJSP::operator[](const unsigned int index) const {
	if (index < 0) {
		std::string errorMsg = "Trying to access unexisting task: ";
		errorMsg += valueToString(index);
		throw JSPException("Schedule", errorMsg);
	}
	if (index >= this->nScheduledTasks)
		return NULL;
	return this->taskInfo[index].task;
}





//=============================================================================
//		METHODS
//=============================================================================
//====  addTask Method  =======================================================
void ScheduleJSP::addTask(const int taskIdx, FuzzyFW::Crisp & ST,
	const int macSuc) {

	const TaskJSP * task = (*(this->problem))[taskIdx];
	int mac = task->machine;
	int job = task->job;
	int macPred;

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


	// Keep the tasks sorted. In JSP is very likely that the new task will
	// be at the end, which makes this worth. Otherwise, it would be better
	// to leave them unsorted and use quicksort at the end
	if (this->isSorted) {
		int i;
		for (i = this->nScheduledTasks; i > 0; i--) {
			this->taskOrder[i] = this->taskOrder[i - 1];

			if (this->taskInfo[this->taskOrder[i]].head < ST) {
				this->taskOrder[i] = taskIdx;
				break;
			}

			if (this->taskInfo[this->taskOrder[i]].head == ST
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
void ScheduleJSP::verifyHeads(const FuzzyFW::Crisp& expectedMakespan,
	const std::string& context) const {

	int n = (int)this->taskInfo.size();
	std::vector<FuzzyFW::Crisp> computedHead(n, FuzzyFW::Crisp(0));
	FuzzyFW::Crisp exactMakespan(0);

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

		FuzzyFW::Crisp expected(0);
		if (jp != -1 && mp != -1) {
			FuzzyFW::Crisp fromJp = computedHead[jp] + this->taskInfo[jp].task->p;
			FuzzyFW::Crisp fromMp = computedHead[mp] + this->taskInfo[mp].task->p;
			expected = std::max(fromJp, fromMp);
		} else if (jp != -1) {
			expected = computedHead[jp] + this->taskInfo[jp].task->p;
		} else if (mp != -1) {
			expected = computedHead[mp] + this->taskInfo[mp].task->p;
		}

		computedHead[t] = expected;

		// Compare stored head with recomputed head. Crisp times are exact
		// integers, so this is an equality check and not a tolerance one.
		if (this->taskInfo[t].head.v != expected.v) {
			std::cerr << "[" << context << "] HEAD MISMATCH task " << t
				<< " stored=" << this->taskInfo[t].head.v
				<< " expected=" << expected.v
				<< std::endl;
		}

		// Accumulate exact makespan using recomputed heads
		int ct = expected.v + this->taskInfo[t].task->p.v;
		if (ct > exactMakespan.v) exactMakespan.v = ct;

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

	// Compare exact makespan with the neighbourhood-computed one. Crisp times
	// are exact integers, so this is an equality check and not a tolerance
	// one, matching the head check above.
	if (exactMakespan.v != expectedMakespan.v) {
		std::cerr << "[" << context << "] MAKESPAN MISMATCH"
			<< " computed=" << expectedMakespan.v
			<< " exact=" << exactMakespan.v
			<< std::endl;
	}
}


//====  reset Method  =========================================================
void ScheduleJSP::reset() {
	if (this->nScheduledTasks <= 0)
		return;

	this->nScheduledTasks = 0;
	this->isSorted = true;

	for (size_t i = 0; i < this->taskInfo.size(); i++) {
		this->taskInfo[i].head = FuzzyFW::Crisp(-1);
		this->taskInfo[i].mp = this->taskInfo[i].mp = -1;
		this->taskOrder[i] = -1;
	}

	for (size_t i = 0; i < this->lastTaskMachine.size(); i++)
		this->lastTaskMachine[i] = -1;
	for (size_t i = 0; i < this->lastTaskJob.size(); i++)
		this->lastTaskJob[i] = -1;
}



//====  update Topological Order  =============================================
void ScheduleJSP::updateTopologicalOrder(FuzzyFW::Random *rng) {
	if (this->isSorted)
		return;
	if(rng == NULL)
		this->quicksortTasks(0, this->nScheduledTasks - 1, new FuzzyFW::Random());
	else
		this->quicksortTasks(0, this->nScheduledTasks - 1, rng);
	this->isSorted = true;
}

//====  repair times of the sheduling so there is no overlapping ===================================
void ScheduleJSP::repairScheduledTimes(const int taskIdx, int _depth) {
	if (_depth > 10000) {
		fprintf(stderr, "[DBG] repairScheduledTimes DEPTH LIMIT at task=%d depth=%d\n", taskIdx, _depth);
		fflush(stderr);
		return;
	}
	if (taskIdx < 0 || taskIdx >=  (int)this->taskInfo.size()) return;

	JSP::ScheduledTaskInfo currentTask = this->taskInfo[taskIdx];
	JSP::ScheduledTaskInfo* repairedTask;

	//repair job successor
	if (currentTask.task->js >= 0 && currentTask.task->js < (int)this->taskInfo.size()) {
		repairedTask = &(this->taskInfo[currentTask.task->js]);
		if (repairedTask->task && adjustHead(currentTask, repairedTask)) {
			this->repairScheduledTimes(repairedTask->task->id, _depth + 1);
		}
	}

	//repair machine successor
	if (currentTask.ms >=0 && currentTask.ms < (int)this->taskInfo.size()) {
		repairedTask = &(this->taskInfo[currentTask.ms]);
		if (repairedTask -> task && adjustHead(currentTask, repairedTask)) {
			this->repairScheduledTimes(repairedTask->task->id, _depth + 1);
		}
	}

}

//====  adjusts head of a succcessor after completion of the current task===========================
bool ScheduleJSP::adjustHead(const ScheduledTaskInfo currentTask, ScheduledTaskInfo* successor) {
	FuzzyFW::Crisp completionTime = currentTask.head + currentTask.task->p;
	bool repairNeed = false;
	
	if (successor->head.v < 0) return false;

	if (completionTime.v > successor->head.v) {
		successor->head.v = completionTime.v;
		repairNeed = true;
	}

	return repairNeed && successor->task;
}


void ScheduleJSP::verifyScheduling() {
	//We verify task by task that the job restrictions are correct
	for (int i = 0; i < this->lastTaskJob.size(); i++) {
		ScheduledTaskInfo currentTask = this->taskInfo[this->lastTaskJob[i]];
		while (currentTask.task->jp >= 0 && currentTask.task->jp < this->taskInfo.size()) {
			ScheduledTaskInfo jobPredeccessor = this->taskInfo[currentTask.task->jp];
			if (currentTask.head < (jobPredeccessor.head + jobPredeccessor.task->p)) {
				throw JSPException("Schedule", "Error scheduling tasks in job number" + valueToString(jobPredeccessor.task->job) +
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
			if (currentTask.head < (machinePredeccessor.head + machinePredeccessor.task->p)) {
				throw JSPException("Schedule", "Error scheduling tasks in machine" + valueToString(machinePredeccessor.task->machine) +
					": id = " + valueToString(machinePredeccessor.task->id) + "; head = " + valueToString(machinePredeccessor.head) +
					"; p = " + valueToString(machinePredeccessor.task->p) + "; total = " + valueToString(machinePredeccessor.head + machinePredeccessor.task->p) +
					" AND id = " + valueToString(currentTask.task->id) + "; head = " + valueToString(currentTask.head));
			}
			currentTask = machinePredeccessor;
		}

	}
}


//====  apply quicksort to sort tasks  ========================================
void ScheduleJSP::quicksortTasks(int left, int right, FuzzyFW::Random *rng) {
	int pivot;
	FuzzyFW::Crisp pivotValue;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);
	pivotValue = this->taskInfo[this->taskOrder[pivot]].head;

	std::swap(this->taskOrder[pivot], this->taskOrder[right]);
	pivot = left;
	for (int i = left; i < right; i++) {
		if (this->taskInfo[this->taskOrder[i]].head < pivotValue) {
			std::swap(this->taskOrder[i], this->taskOrder[pivot]);
			pivot++;
		}
		else if (this->taskInfo[this->taskOrder[i]].head == pivotValue
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
