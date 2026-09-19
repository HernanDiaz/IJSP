/*
 * NeighbourhoodJSP_N2.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodJSP_N2.h"

namespace JSP {

//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN2_MakespanJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Crisp currentMakespan;
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
	// One pass over the critical path. With interval durations this ran
	// once per endpoint, because G- and G+ are different graphs; on crisp
	// times they are the same graph, so the second pass re-asked the
	// questions the first had answered and added[] discarded the answers.
	for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
		if (this->schedule->getCTMachine(i) == currentMakespan) {
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
			if ((mp.head + mp.task->p) == task.head) {
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
						|| !((mpmp.head + mpmp.task->p) == mp.head)
						|| (!((task.head + task.task->p) == ms.head) || criticalPath[task.ms] == false)) {

						if (this->numNeighbours < this->neighbours.size()
							&& this->neighbours[this->numNeighbours] != nullptr)
							this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
						else
							this->neighbours.push_back(std::make_unique<NeighbourJSP_Arc>(task.mp, taskId));
						this->numNeighbours++;
						added[task.mp] = true;
					}
				}
			}
		}

		if (task.task->jp != -1) {
			jp = this->schedule->taskInfo[task.task->jp];
			if ((jp.head + jp.task->p) == task.head) {
				if (!criticalPath[task.task->jp]) {
					taskQueue.push(task.task->jp);
					criticalPath[task.task->jp] = true;
				}
			}
		}
	}
	return this->numNeighbours;
}


//-----  Reverse an arc on the live schedule  ---------------------------------
// Reverses the disjunctive arc (x, y) on this->schedule and propagates the new
// heads forward. Every write is logged in `undo`, so revertArc() restores the
// schedule exactly. Returns false when the propagation trips the cycle guard
// or -- with `improvement` -- when a partial completion time already exceeds
// the incumbent; either way the caller has to revert.
//
// This is the code that used to run on a deep copy of the schedule. It is the
// same algorithm on the same data, so the heads it leaves behind are the ones
// the copy would have held.
bool NB_ParallelN2_MakespanJSP::applyArc(const NeighbourJSP_Arc *arc,
	const bool improvement) {

	ScheduleJSP *s = this->schedule;
	const int x = arc->x, y = arc->y;
	int job, z, mpz, jpz, msz, jsz;
	FuzzyFW::Crisp newHead;
	std::queue<int> taskQueue;
	FuzzyFW::FitnessCrisp lowerBound(*this->currentFitness);

	this->undo.x = x;
	this->undo.y = y;
	this->undo.mac = s->taskInfo[x].task->machine;
	this->undo.msy = s->taskInfo[y].ms;
	this->undo.mpx = s->taskInfo[x].mp;
	this->undo.old_y_mp = s->taskInfo[y].mp;
	this->undo.old_y_ms = s->taskInfo[y].ms;
	this->undo.old_x_mp = s->taskInfo[x].mp;
	this->undo.old_x_ms = s->taskInfo[x].ms;
	this->undo.heads.clear();

	const int mpx = this->undo.mpx, msy = this->undo.msy, mac = this->undo.mac;
	if (mpx != -1) {
		this->undo.old_mpx_ms = s->taskInfo[mpx].ms;
		s->taskInfo[mpx].ms = y;
	}
	s->taskInfo[y].mp = mpx;
	s->taskInfo[y].ms = x;
	s->taskInfo[x].mp = y;
	s->taskInfo[x].ms = msy;
	if (msy != -1) {
		this->undo.old_msy_mp = s->taskInfo[msy].mp;
		s->taskInfo[msy].mp = x;
	}
	else {
		this->undo.old_last = s->lastTaskMachine[mac];
		s->lastTaskMachine[mac] = x;
	}

	// Update heads (SPFA; cycle -> infeasible swap)
	int _nTasks = (int)s->getScheduledTasks();
	int _bfsLimit = _nTasks * 20;
	std::vector<bool> inQueue(_nTasks, false);
	taskQueue.push(y); inQueue[y] = true;
	taskQueue.push(x); inQueue[x] = true;
	int _bfsCount = 0;

	while (!taskQueue.empty()) {
		_bfsCount++;
		if (_bfsCount > _bfsLimit)
			return false;
		z = taskQueue.front();
		taskQueue.pop();
		inQueue[z] = false;
		jpz = s->taskInfo[z].task->jp;
		mpz = s->taskInfo[z].mp;
		msz = s->taskInfo[z].ms;
		job = s->taskInfo[z].task->job;
		if (s->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = s->taskInfo[z].task->js;

		if (jpz != -1 && mpz != -1)
			newHead = std::max(s->taskInfo[mpz].head + s->taskInfo[mpz].task->p, s->taskInfo[jpz].head + s->taskInfo[jpz].task->p);
		else if (mpz != -1)
			newHead = s->taskInfo[mpz].head + s->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = s->taskInfo[jpz].head + s->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::Crisp(0);

		if (!(s->taskInfo[z].head == newHead)) {
			this->undo.heads.push_back(std::make_pair(z, s->taskInfo[z].head));
			s->taskInfo[z].head = newHead;

			if (improvement && jsz == -1) {
				lowerBound.setValue(s->taskInfo[z].head
					+ s->taskInfo[z].task->p);
				if (lowerBound.isWorseThan(currentFitness))
					return false;
			}
			if (msz != -1 && !inQueue[msz]) { taskQueue.push(msz); inQueue[msz] = true; }
			if (jsz != -1 && !inQueue[jsz]) { taskQueue.push(jsz); inQueue[jsz] = true; }
		}
	}
	return true;
}


//-----  Undo the last applyArc  ----------------------------------------------
void NB_ParallelN2_MakespanJSP::revertArc() {
	ScheduleJSP *s = this->schedule;

	// Heads in reverse, so a task logged twice ends on its original value.
	for (size_t i = this->undo.heads.size(); i-- > 0; )
		s->taskInfo[this->undo.heads[i].first].head = this->undo.heads[i].second;
	this->undo.heads.clear();

	if (this->undo.mpx != -1)
		s->taskInfo[this->undo.mpx].ms = this->undo.old_mpx_ms;
	s->taskInfo[this->undo.y].mp = this->undo.old_y_mp;
	s->taskInfo[this->undo.y].ms = this->undo.old_y_ms;
	s->taskInfo[this->undo.x].mp = this->undo.old_x_mp;
	s->taskInfo[this->undo.x].ms = this->undo.old_x_ms;
	if (this->undo.msy != -1)
		s->taskInfo[this->undo.msy].mp = this->undo.old_msy_mp;
	else
		s->lastTaskMachine[this->undo.mac] = this->undo.old_last;
}


//-----  Fully evaluate the neighbour  ----------------------------------------
// Applies the move to the live schedule, reads the makespan off it and undoes
// it. The neighbour keeps the fitness and no solution; acceptNeighbour applies
// the move again for real.
FuzzyFW::Fitness *NB_ParallelN2_MakespanJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw JSPException("Neighbourhood", errorMsg);
	}

	NeighbourJSP_Arc *arc = this->neighbours[idx].get();
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	if (!this->applyArc(arc, improvement)) {
		this->revertArc();
		return NULL;
	}

	FuzzyFW::Crisp newMakespan(0);
	for (size_t i = 0; i < this->schedule->lastTaskJob.size(); i++)
		newMakespan = std::max(newMakespan, this->schedule->getCTJob(i));

	this->revertArc();

	this->neighbours[idx]->setEvaluatedFitness(
		new FuzzyFW::FitnessCrisp(newMakespan, false));
	return this->neighbours[idx]->getEvaluatedFitness();
}


//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN2_MakespanJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Crisp newTail;
	std::queue<int> taskQueue;
	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw JSPException("Neighbourhood", errorMsg);
	}

	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);
	if (!this->neighbours[idx]->isEvaluated()) {
		std::string errorMsg = "Accepting a neighbour whose move is infeasible";
		throw JSPException("Neighbourhood", errorMsg);
	}

	NeighbourJSP_Arc *arc = this->neighbours[idx].get();

	// The move was feasible when it was evaluated on this same schedule, so
	// it is feasible now; the guard is only there to make a broken invariant
	// loud rather than silent.
	if (!this->applyArc(arc, false)) {
		this->revertArc();
		std::string errorMsg = "A move that evaluated as feasible failed on accept";
		throw JSPException("Neighbourhood", errorMsg);
	}
	this->schedule->setSorted(false);

	if (this->currentFitness != nullptr)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessCrisp *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());
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
			newTail = std::max(this->schedule->taskInfo[msz].task->p + this->tails[msz], this->schedule->taskInfo[jsz].task->p + this->tails[jsz]);
		else if (msz != -1)
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		else if (jsz != -1)
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z] == newTail)) {
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

} // namespace JSP
