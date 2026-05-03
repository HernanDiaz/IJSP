/*
 * NeighbourhoodIJSP_N3.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N3.h"

namespace IJSP {

//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN3_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo  mpmpmp, mpmp, ms;
	std::queue<int> taskQueue;
	std::vector<char> added0;
	std::vector<char> added1;
	std::vector<char> added2;
	std::vector<char> added3;
	std::vector<char> criticalPath;

	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added0.resize(nTasks, false);
	added1.resize(nTasks, false);
	added2.resize(nTasks, false);
	added3.resize(nTasks, false);
	criticalPath.resize(nTasks, false);

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	for (short int comp = 1; comp <= 2; comp++) {
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp)) {
				taskQueue.push(this->schedule->lastTaskMachine[i]);
			}
		}

		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();

			task = this->schedule->taskInfo[taskId];
			if (task.mp != -1) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					if (!criticalPath[task.mp]) { taskQueue.push(task.mp); criticalPath[task.mp] = true; }
					if (mp.mp != -1) {
						mpmp = this->schedule->taskInfo[mp.mp];
					}
					if (task.ms != -1) {
						ms = this->schedule->taskInfo[task.ms];
					}
					if (!added0[task.mp] && task.mp != task.task->jp) {
						if (task.mp != task.task->jp) {
							if (mp.mp == -1 || task.ms == -1
								|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
								|| !(task.head + task.task->p).EqualComponent(ms.head, comp)) {

								this->addNeighbour(task.mp, taskId);
								added0[task.mp] = true;
							}
						}
					}

					if (mp.mp != -1 && mpmp.mp!=-1) {
						mpmpmp = this->schedule->taskInfo[mpmp.mp];
					}
					if (mp.mp !=-1 && (mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
						&& ((mpmp.mp == -1 || !(mpmpmp.head + mpmpmp.task->p).EqualComponent(mpmp.head, comp))
						|| (task.ms == -1 || !(task.head + task.task->p).EqualComponent(ms.head, comp))))  {
						if (!added3[task.mp]) {
							this->addNeighbour(mp.mp, task.mp, taskId, 3);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == nullptr) {
								this->discardNeighbour(this->numNeighbours - 1);
							}
							else added3[task.mp] = true;
						}

						if (!added1[task.mp] && (mpmp.mp == -1 || !(mpmpmp.head + mpmpmp.task->p).EqualComponent(mpmp.head, comp))) {
							this->addNeighbour(mp.mp, task.mp, taskId, 1);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == nullptr) {
								this->discardNeighbour(this->numNeighbours - 1);
							}
							else added1[task.mp] = true;
						}
						if (!added2[task.mp] && (task.ms == -1 || !(task.head + task.task->p).EqualComponent(ms.head, comp))) {
							this->addNeighbour(mp.mp, task.mp, taskId, 2);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == nullptr) {
								this->discardNeighbour(this->numNeighbours - 1);
							}
							else added2[task.mp] = true;
						}
					}
				}
			}

			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					if (!criticalPath[task.task->jp]) { taskQueue.push(task.task->jp); criticalPath[task.task->jp] = true; }
				}
			}
		}
	}

	return this->numNeighbours;
}


//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness * NB_ParallelN3_MakespanIJSP::evaluateNeighbour(
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

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	if (arc->x < 0 || arc->y < 0 || arc->z < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

	if (arc->tipo == 0) {
		mac = newSolution->taskInfo[arc->x].task->machine;
		msy = newSolution->taskInfo[arc->y].ms;
		mpx = newSolution->taskInfo[arc->x].mp;

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

		taskQueue.push(arc->y);
	}
	else if (arc->tipo == 1) {
		mac = newSolution->taskInfo[arc->x].task->machine;
		msz = newSolution->taskInfo[arc->z].ms;
		mpx = newSolution->taskInfo[arc->x].mp;

		if (mpx != -1)
			newSolution->taskInfo[mpx].ms = arc->y;
		newSolution->taskInfo[arc->y].mp = mpx;
		newSolution->taskInfo[arc->z].ms = arc->x;
		newSolution->taskInfo[arc->x].mp = arc->z;
		newSolution->taskInfo[arc->x].ms = msz;
		if (msz != -1)
			newSolution->taskInfo[msz].mp = arc->x;
		else
			newSolution->lastTaskMachine[mac] = arc->x;

		taskQueue.push(arc->y);
	}
	else if (arc->tipo == 2) {
		mac = newSolution->taskInfo[arc->x].task->machine;
		mpx = newSolution->taskInfo[arc->x].mp;
		msz = newSolution->taskInfo[arc->z].ms;

		if (mpx != -1)
			newSolution->taskInfo[mpx].ms = arc->z;
		newSolution->taskInfo[arc->z].mp = mpx;
		newSolution->taskInfo[arc->z].ms = arc->x;
		newSolution->taskInfo[arc->x].mp = arc->z;
		newSolution->taskInfo[arc->y].ms = msz;
		if (msz != -1)
			newSolution->taskInfo[msz].mp = arc->y;
		else
			newSolution->lastTaskMachine[mac] = arc->y;

		taskQueue.push(arc->z);
	}
	else if (arc->tipo == 3) {
		mac = newSolution->taskInfo[arc->x].task->machine;
		mpx = newSolution->taskInfo[arc->x].mp;
		msz = newSolution->taskInfo[arc->z].ms;

		if (mpx != -1)
			newSolution->taskInfo[mpx].ms = arc->z;
		newSolution->taskInfo[arc->z].mp = mpx;
		newSolution->taskInfo[arc->y].mp = arc->z;
		newSolution->taskInfo[arc->z].ms = arc->y;
		newSolution->taskInfo[arc->x].mp = arc->y;
		newSolution->taskInfo[arc->y].ms = arc->x;
		newSolution->taskInfo[arc->x].ms = msz;
		if (msz != -1)
			newSolution->taskInfo[msz].mp = arc->x;
		else
			newSolution->lastTaskMachine[mac] = arc->x;

		taskQueue.push(arc->z);
		taskQueue.push(arc->y);
		taskQueue.push(arc->x);
	}

	// Update heads (SPFA; cycle → infeasible swap → return NULL)
	{
	int _nTasks = (int)newSolution->getScheduledTasks();
	int _bfsLimit = _nTasks * 20;
	int _bfsCount = 0;
	std::vector<bool> inQueue(_nTasks, false);
	if (!taskQueue.empty()) {
		std::queue<int> _tmp = taskQueue;
		while (!_tmp.empty()) { int _t = _tmp.front(); _tmp.pop(); if (_t >= 0 && _t < _nTasks) inQueue[_t] = true; }
	}
	while (!taskQueue.empty()) {
		if (++_bfsCount > _bfsLimit) {
			delete lowerBound;
			delete newSolution;
			return NULL;
		}
		z = taskQueue.front();
		taskQueue.pop();
		if (z >= 0 && z < _nTasks) inQueue[z] = false;
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
			if (msz != -1 && msz < _nTasks && !inQueue[msz]) { taskQueue.push(msz); inQueue[msz] = true; }
			if (jsz != -1 && jsz < _nTasks && !inQueue[jsz]) { taskQueue.push(jsz); inQueue[jsz] = true; }
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
void NB_ParallelN3_MakespanIJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
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
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	std::vector<char> inQueue(this->schedule->getScheduledTasks(), false);
	switch (arc->tipo) {
		case 0: {
			taskQueue.push(arc->x); inQueue[arc->x] = true;
			taskQueue.push(arc->y); inQueue[arc->y] = true;
		}
		break;
		case 1: {
			taskQueue.push(arc->x); inQueue[arc->x] = true;
			taskQueue.push(arc->z); inQueue[arc->z] = true;
			taskQueue.push(arc->y); inQueue[arc->y] = true;
     	}
		break;
		case 2: {
			taskQueue.push(arc->y); inQueue[arc->y] = true;
			taskQueue.push(arc->z); inQueue[arc->z] = true;
		}
		break;
		case 3: {
			taskQueue.push(arc->x); inQueue[arc->x] = true;
			taskQueue.push(arc->y); inQueue[arc->y] = true;
			taskQueue.push(arc->z); inQueue[arc->z] = true;
		}
	};
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


//-----  Add a neighbour  -----------------------------------------------------
void NB_ParallelN3_MakespanIJSP::addNeighbour(const unsigned int x,
	const unsigned int y, const unsigned int z, const unsigned int tipo) {
	if (this->numNeighbours < this->neighbours.size()
		&& this->neighbours[this->numNeighbours] != nullptr) {
		this->neighbours[this->numNeighbours]->setValues(x, y, z, tipo);
	}
	else if (this->numNeighbours < this->neighbours.size()
		&& this->neighbours[this->numNeighbours] == nullptr) {
		this->neighbours[this->numNeighbours] = std::make_unique<NeighbourIJSP_Arc>(x, y, z, tipo);
	}
	else {
		this->neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(x, y, z, tipo));
	}
	this->numNeighbours++;
}


//-----  Discard a neighbour  -------------------------------------------------
void NB_ParallelN3_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}
	this->neighbours[idx].reset();
	if (idx != this->numNeighbours - 1) {
		std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	}
	this->numNeighbours--;
}


//-----  Estimate via heads and tails  ----------------------------------------
void NB_ParallelN3_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, tailZ, headX, headY, headZ;
    FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	if (arc->tipo == 0) {
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		int jsx, jsy;

		int jpy = schedule->taskInfo[y].task->jp;
		int msy = schedule->taskInfo[y].ms;
		int mpx = schedule->taskInfo[x].mp;
		int jpx = schedule->taskInfo[x].task->jp;

		if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
			jsx = -1;
		else
			jsx = schedule->taskInfo[x].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
			jsy = -1;
		else
			jsy = schedule->taskInfo[y].task->js;

		if (jsx != -1 && msy != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msy] + schedule->taskInfo[msy].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msy != -1)
			tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailX + schedule->taskInfo[x].task->p;

		if (mpx != -1 && jpy != -1)
			headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpy != -1)
			headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
		else headY = FuzzyFW::Interval(0, 0);

		if (jpx != -1)
			headX = maximum(headY + schedule->taskInfo[y].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headY + schedule->taskInfo[y].task->p;

		makespan = maximum(headX + schedule->taskInfo[x].task->p + tailX,
			headY + schedule->taskInfo[y].task->p + tailY,
			FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}

	if (arc->tipo == 1) {
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int jpy = schedule->taskInfo[y].task->jp;
		int mpx = schedule->taskInfo[x].mp;
		int jpx = schedule->taskInfo[x].task->jp;
	    int msz = schedule->taskInfo[z].ms;
		int jpz = schedule->taskInfo[z].task->jp;

		if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
			jsx = -1;
		else
			jsx = schedule->taskInfo[x].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
			jsy = -1;
		else
			jsy = schedule->taskInfo[y].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[z].task->job] == z)
			jsz = -1;
		else
			jsz = schedule->taskInfo[z].task->js;

		if (jsx != -1 && msz != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msz != -1)
			tailX = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailX + schedule->taskInfo[x].task->p;

		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailZ + schedule->taskInfo[z].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailZ + schedule->taskInfo[z].task->p;

		if (mpx != -1 && jpy != -1)
			headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpy != -1)
			headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
		else headY = FuzzyFW::Interval(0, 0);

		if (jpz != -1)
			headZ = maximum(headY + schedule->taskInfo[y].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headZ = headY + schedule->taskInfo[y].task->p;

		if (jpx != -1)
			headX = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headZ + schedule->taskInfo[z].task->p;

		makespan = maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX,
				headY + schedule->taskInfo[y].task->p + tailY,
				FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ,
			FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}

	if (arc->tipo == 2) {
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int jpy = schedule->taskInfo[y].task->jp;
		int mpx = schedule->taskInfo[x].mp;
		int jpx = schedule->taskInfo[x].task->jp;
		int msz = schedule->taskInfo[z].ms;
		int jpz = schedule->taskInfo[z].task->jp;

		if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
			jsx = -1;
		else
			jsx = schedule->taskInfo[x].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
			jsy = -1;
		else
			jsy = schedule->taskInfo[y].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[z].task->job] == z)
			jsz = -1;
		else
			jsz = schedule->taskInfo[z].task->js;

		if (jsy != -1 && msz != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsy != -1)
			tailY = this->tails[jsy] + schedule->taskInfo[jsy].task->p;
		else if (msz != -1)
			tailY = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailY = FuzzyFW::Interval(0, 0);

		if (jsx != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				tailY + schedule->taskInfo[y].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailX = tailY + schedule->taskInfo[y].task->p;

		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailX + schedule->taskInfo[x].task->p;

		if (mpx != -1 && jpz != -1)
			headZ = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headZ = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpz != -1)
			headZ = schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p;
		else headZ = FuzzyFW::Interval(0, 0);

		if (jpx != -1)
			headX = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headZ + schedule->taskInfo[z].task->p;

		if (jpy != -1)
			headY = maximum(headX + schedule->taskInfo[x].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headY = headX + schedule->taskInfo[x].task->p;

		makespan = maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX,
				headY + schedule->taskInfo[y].task->p + tailY,
				FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ,
			FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}

	if (arc->tipo == 3) {
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int jpy = schedule->taskInfo[y].task->jp;
		int mpx = schedule->taskInfo[x].mp;
		int jpx = schedule->taskInfo[x].task->jp;
		int msz = schedule->taskInfo[z].ms;
		int jpz = schedule->taskInfo[z].task->jp;

		if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
			jsx = -1;
		else
			jsx = schedule->taskInfo[x].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
			jsy = -1;
		else
			jsy = schedule->taskInfo[y].task->js;
		if (schedule->lastTaskJob[schedule->taskInfo[z].task->job] == z)
			jsz = -1;
		else
			jsz = schedule->taskInfo[z].task->js;

		if (jsx != -1 && msz != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msz != -1)
			tailX = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailX + schedule->taskInfo[x].task->p;

		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailY + schedule->taskInfo[y].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailY + schedule->taskInfo[y].task->p;

		if (mpx != -1 && jpz != -1)
			headZ = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headZ = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpz != -1)
			headZ = schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p;
		else headZ = FuzzyFW::Interval(0, 0);

		if (jpy != -1)
			headY = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headY = headZ + schedule->taskInfo[z].task->p;

		if (jpx != -1)
			headX = maximum(headY + schedule->taskInfo[y].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headY + schedule->taskInfo[y].task->p;

		makespan = maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX,
				headY + schedule->taskInfo[y].task->p + tailY,
				FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ,
			FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}
}

} // namespace IJSP
