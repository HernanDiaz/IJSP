/*
 * NeighbourhoodJSP_Next.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodJSP_Next.h"
#include <iostream>
namespace JSP {

//
//	Class NB_ParallelNext_MakespanJSP
//
//=============================================================================

//-----  Check viability of an interior-block swap using heads & tails  -------
bool NB_ParallelNext_MakespanJSP::isViableSwap(unsigned int x, unsigned int y) {
	FuzzyFW::Crisp tailX, tailY, headX, headY;
	int mpx, jpx, jsx;
	int jpy, msy, jsy;

	mpx = schedule->taskInfo[x].mp;
	jpx = schedule->taskInfo[x].task->jp;
	jpy = schedule->taskInfo[y].task->jp;
	msy = schedule->taskInfo[y].ms;

	if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == (int)x)
		jsx = -1;
	else
		jsx = schedule->taskInfo[x].task->js;

	if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == (int)y)
		jsy = -1;
	else
		jsy = schedule->taskInfo[y].task->js;

	// New tail for x (x moves to y's machine slot, ms(y) becomes its machine successor)
	if (jsx != -1 && msy != -1)
		tailX = std::max(this->tails[jsx] + schedule->taskInfo[jsx].task->p, this->tails[msy] + schedule->taskInfo[msy].task->p);
	else if (jsx != -1)
		tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::Crisp(0);

	// New tail for y (y moves behind x)
	if (jsy != -1)
		tailY = std::max(this->tails[jsy] + schedule->taskInfo[jsy].task->p, tailX + schedule->taskInfo[x].task->p);
	else
		tailY = tailX + schedule->taskInfo[x].task->p;

	// New head for y (y takes x's predecessors)
	if (mpx != -1 && jpy != -1)
		headY = std::max(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p, schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p);
	else if (mpx != -1)
		headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
	else
		headY = FuzzyFW::Crisp(0);

	// New head for x (x goes after y)
	if (jpx != -1)
		headX = std::max(headY + schedule->taskInfo[y].task->p, schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p);
	else
		headX = headY + schedule->taskInfo[y].task->p;

	FuzzyFW::Crisp estimate = std::max(headX + schedule->taskInfo[x].task->p + tailX, headY + schedule->taskInfo[y].task->p + tailY);

	FuzzyFW::FitnessCrisp estimatedFit(estimate, false);
	return estimatedFit.isBetterThan(this->currentFitness);
}


//-----  Find neighbours: all critical-block arcs (boundary + interior)  ------
//
// N_ext extends N2 by including ALL consecutive arc pairs within each
// critical block, not only the boundary (first/last) pairs.
// Boundary arcs (same as N2) are added unconditionally.
// Interior arcs are added if the heads&tails viability check passes.
//
unsigned int NB_ParallelNext_MakespanJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Crisp currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo  mpmp, ms;
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

	// Look for critical paths in each parallel graph:
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
				taskQueue.push(task.mp);
				criticalPath[task.mp] = true;
				if (!added[task.mp]) {
					if (mp.mp != -1) {
						mpmp = this->schedule->taskInfo[mp.mp];
					}
					if (task.ms != -1) {
						ms = this->schedule->taskInfo[task.ms];
					}

					bool isBoundary = (mp.mp == -1 || task.ms == -1
						|| !((mpmp.head + mpmp.task->p) == mp.head)
						|| (!((task.head + task.task->p) == ms.head)
							|| criticalPath[task.ms] == false));

					bool viable = !isBoundary && isViableSwap(task.mp, taskId);

					if (isBoundary || viable) {
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
				taskQueue.push(task.task->jp);
				criticalPath[task.task->jp] = true;
			}
		}
	}

	return this->numNeighbours;
}

} // namespace JSP
