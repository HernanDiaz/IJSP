/*
 * NeighbourhoodIJSP_N2.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N2.h"

namespace IJSP {

//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN2_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo mpmp, ms;
	std::queue<int> taskQueue;
	std::vector<char> added;
	std::vector<char> criticalPath;

	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	criticalPath.resize(nTasks, false);

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;
	for (short int comp = 1; comp <= 2; comp++) {
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp)) {
				criticalPath[this->schedule->lastTaskMachine[i]] = true;
				taskQueue.push(this->schedule->lastTaskMachine[i]);
			}
		}

		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();
			task = this->schedule->taskInfo[taskId];
			if (task.mp != -1 && task.mp != task.task->jp) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					if (!criticalPath[task.mp]) {
						taskQueue.push(task.mp);
						criticalPath[task.mp] = true;
					}
					if (!added[task.mp]) {
						if (mp.mp != -1)
							mpmp = this->schedule->taskInfo[mp.mp];
						if (task.ms != -1)
							ms = this->schedule->taskInfo[task.ms];

						if (mp.mp == -1 || task.ms == -1
							|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
							|| (!(task.head + task.task->p).EqualComponent(ms.head, comp) || criticalPath[task.ms] == false)) {

							if (this->numNeighbours < this->neighbours.size()
								&& this->neighbours[this->numNeighbours] != NULL)
								this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
							else
								this->neighbours.push_back(new NeighbourIJSP_Arc(task.mp, taskId));
							this->numNeighbours++;
							added[task.mp] = true;
						}
					}
				}
			}

			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					if (!criticalPath[task.task->jp]) {
						taskQueue.push(task.task->jp);
						criticalPath[task.task->jp] = true;
					}
				}
			}
		}
	}
	return this->numNeighbours;
}


//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness *NB_ParallelN2_MakespanIJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	FuzzyFW::FitnessInterval *lowerBound;
	FuzzyFW::Interval currentMakespan, newMakespan;
	ScheduleIJSP *newSolution;
	int job, mac;
	int jsx, jsy, jpx, jpy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	FuzzyFW::Interval newHead, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

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
		_bfsCount++;
		if (_bfsCount > _bfsLimit) {
			delete lowerBound;
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
			newHead = maximum(newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p,
				newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpz != -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::Interval(0, 0);

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::Interval::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;

			if (improvement && jsz == -1) {
				lowerBound->setValue(newSolution->taskInfo[z].head
					+ newSolution->taskInfo[z].task->p);
				if (lowerBound->isWorseThan(currentFitness)) {
					delete lowerBound;
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
		newMakespan = maximum(newMakespan, newSolution->getCTJob(i),
			FuzzyFW::Interval::M_COMPONENT);
	}

	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessInterval(newMakespan, false));

	delete lowerBound;
	return this->neighbours[idx]->getEvaluatedFitness();
}


//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN2_MakespanIJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
	std::queue<int> taskQueue;
	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>
		(this->neighbours[idx]->getEvaluation()->clone());
	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	std::vector<int> tailsUpdated;
	tailsUpdated.resize(this->schedule->getScheduledTasks(), 0);

	taskQueue.push(arc->x);
	tailsUpdated[arc->x] = 1;
	taskQueue.push(arc->y);
	tailsUpdated[arc->y] = 1;

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = this->schedule->taskInfo[z].ms;
		job = this->schedule->taskInfo[z].task->job;
		if (this->schedule->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = this->schedule->taskInfo[z].task->js;

		if (jsz != -1 && msz != -1)
			newTail = maximum(this->schedule->taskInfo[msz].task->p + this->tails[msz],
				this->schedule->taskInfo[jsz].task->p + this->tails[jsz],
				FuzzyFW::Interval::M_COMPONENT);
		else if (msz != -1)
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		else if (jsz != -1)
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail, FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1 && !tailsUpdated[this->schedule->taskInfo[z].mp]) {
				taskQueue.push(this->schedule->taskInfo[z].mp);
				tailsUpdated[this->schedule->taskInfo[z].mp]++;
			}
			if (this->schedule->taskInfo[z].task->jp != -1 && !tailsUpdated[this->schedule->taskInfo[z].task->jp]) {
				taskQueue.push(this->schedule->taskInfo[z].task->jp);
				tailsUpdated[this->schedule->taskInfo[z].task->jp]++;
			}
		}
	}
}

} // namespace IJSP
