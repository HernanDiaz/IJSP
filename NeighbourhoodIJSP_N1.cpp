/*
 * NeighbourhoodIJSP_N1.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N1.h"

namespace IJSP {

//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Crisp currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;

	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);

	if (nTasks < 2)
		return 1;

	std::vector<char> inQueue(nTasks, false);

	this->numNeighbours = 0;
	// Look for critical paths in each parallell graph:
	// One pass over the critical path. With interval durations this ran
	// once per endpoint, because G- and G+ are different graphs; on crisp
	// times they are the same graph, so the second pass re-asked the
	// questions the first had answered and added[] discarded the answers.
	std::fill(inQueue.begin(), inQueue.end(), false);
	// Look for the tasks defining the makespan value
	for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
		if (this->schedule->getCTMachine(i) == currentMakespan) {
			int t = this->schedule->lastTaskMachine[i];
			if (!inQueue[t]) { taskQueue.push(t); inQueue[t] = true; }
		}
	}

	while (taskQueue.size() > 0) {
		taskId = taskQueue.front();
		taskQueue.pop();
		task = this->schedule->taskInfo[taskId];
		if (task.mp != -1 && task.mp != task.task->jp) {
			mp = this->schedule->taskInfo[task.mp];
			if ((mp.head + mp.task->p) == task.head) {
				if (!inQueue[task.mp]) { taskQueue.push(task.mp); inQueue[task.mp] = true; }
				if (!added[task.mp]) {
					if (this->numNeighbours < this->neighbours.size()
						&& this->neighbours[this->numNeighbours] != nullptr)
						this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
					else
						this->neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(task.mp, taskId));
					this->numNeighbours++;
					added[task.mp] = true;
				}
			}
		}

		if (task.task->jp != -1) {
			jp = this->schedule->taskInfo[task.task->jp];
			if ((jp.head + jp.task->p) == task.head)
				if (!inQueue[task.task->jp]) { taskQueue.push(task.task->jp); inQueue[task.task->jp] = true; }
		}
	}
	return this->numNeighbours;
}


//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness *NB_ParallelN1_MakespanIJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	FuzzyFW::Crisp currentMakespan, newMakespan;
	ScheduleIJSP *newSolution;
	int job, mac;
	int jsx, jsy, jpx, jpy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	FuzzyFW::Crisp newHead, lower;
	std::queue<int> taskQueue;
	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Crisp(0);
	// Pruning bound, on the stack: a FitnessCrisp is an int and a flag, so
	// cloning it onto the heap cost an allocation, a free and a virtual
	// call per neighbour evaluated.
	FuzzyFW::FitnessCrisp lowerBound(*this->currentFitness);

	mac = newSolution->taskInfo[arc->x].task->machine;
	msy = newSolution->taskInfo[arc->y].ms;
	mpx = newSolution->taskInfo[arc->x].mp;
	job = newSolution->taskInfo[arc->x].task->job;
	jpx = newSolution->taskInfo[arc->x].task->jp;
	if (newSolution->lastTaskJob[job] == arc->x)
		jsx = -1;
	else
		jsx = newSolution->taskInfo[arc->x].task->js;
	job = newSolution->taskInfo[arc->y].task->job;
	jpy = newSolution->taskInfo[arc->y].task->jp;
	if (newSolution->lastTaskJob[job] == arc->y)
		jsy = -1;
	else
		jsy = newSolution->taskInfo[arc->y].task->js;

	if (mpx != -1)
		newSolution->taskInfo[mpx].ms = arc->y;
	newSolution->taskInfo[arc->y].mp = mpx;
	newSolution->taskInfo[arc->y].ms = arc->x;
	newSolution->taskInfo[arc->x].mp = arc->y;
	newSolution->taskInfo[arc->x].ms = msy;
	if (msy != -1)
		newSolution->taskInfo[msy].mp = arc->x;
	else
		newSolution->lastTaskMachine[mac] = arc->x;

	// Update heads (SPFA; cycle → infeasible swap → return NULL)
	{
	int _nTasks = (int)newSolution->getScheduledTasks();
	int _bfsLimit = _nTasks * 20;
	std::vector<bool> inQueue(_nTasks, false);
	taskQueue.push(arc->y); inQueue[arc->y] = true;
	taskQueue.push(arc->x); inQueue[arc->x] = true;
	int _bfsCount = 0;

	while (!taskQueue.empty()) {
		if (++_bfsCount > _bfsLimit) {
			delete newSolution;
			return NULL;
		}
		z = taskQueue.front();
		taskQueue.pop();
		inQueue[z] = false;
		jpz = newSolution->taskInfo[z].task->jp;
		mpz = newSolution->taskInfo[z].mp;
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		if (jpz != -1 && mpz != -1)
			newHead = std::max(newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p, newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p);
		else if (mpz != -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::Crisp(0);

		if (!(newSolution->taskInfo[z].head == newHead)) {
			newSolution->taskInfo[z].head = newHead;

			if (improvement && jsz == -1) {
				lowerBound.setValue(newSolution->taskInfo[z].head
					+ newSolution->taskInfo[z].task->p);
				if (lowerBound.isWorseThan(currentFitness)) {
					delete newSolution;
					return NULL;
				}
			}
			if (msz != -1 && !inQueue[msz]) { taskQueue.push(msz); inQueue[msz] = true; }
			if (jsz != -1 && !inQueue[jsz]) { taskQueue.push(jsz); inQueue[jsz] = true; }
		}
	}
	}

	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++) {
		newMakespan = std::max(newMakespan, newSolution->getCTJob(i));
	}

	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessCrisp(newMakespan, false));

	return this->neighbours[idx]->getEvaluatedFitness();
}


//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN1_MakespanIJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Crisp newTail;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);
	if (this->schedule != nullptr)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>
		(this->neighbours[idx]->getEvaluation()->clone());
	if (this->currentFitness != nullptr)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessCrisp *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	std::vector<char> inQueue(this->schedule->getScheduledTasks(), false);
	taskQueue.push(arc->x); inQueue[arc->x] = true;
	taskQueue.push(arc->y); inQueue[arc->y] = true;

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = this->schedule->taskInfo[z].ms;
		job = this->schedule->taskInfo[z].task->job;
		if (this->schedule->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = this->schedule->taskInfo[z].task->js;

		if (jsz != -1 && msz != -1)
			newTail = std::max(this->schedule->taskInfo[msz].task->p + this->tails[msz], this->schedule->taskInfo[jsz].task->p + this->tails[jsz]);
		else if (msz != -1)
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		else if (jsz != -1)
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z] == newTail)) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1 && !inQueue[this->schedule->taskInfo[z].mp]) {
				taskQueue.push(this->schedule->taskInfo[z].mp);
				inQueue[this->schedule->taskInfo[z].mp] = true;
			}
			if (this->schedule->taskInfo[z].task->jp != -1 && !inQueue[this->schedule->taskInfo[z].task->jp]) {
				taskQueue.push(this->schedule->taskInfo[z].task->jp);
				inQueue[this->schedule->taskInfo[z].task->jp] = true;
			}
		}
	}
}

} // namespace IJSP
