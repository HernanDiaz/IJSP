/*
 * NeighbourhoodIJSP_N2Inter.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N2Inter.h"
#include <iostream>
namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2Inter_MakespanIJSP
//
//=============================================================================

//-----  Find neighbours: boundary arcs in G-(sigma) INTERSECT G+(sigma)  -----
//
// Strategy:
//   Pass 1 (comp=1): traverse critical paths in G-, record boundary arc for
//                    each machine predecessor x  →  boundary_m[x] = y
//   Pass 2 (comp=2): same for G+  →  boundary_p[x] = y
//   Intersection: arcs where boundary_m[x] >= 0  AND  boundary_p[x] >= 0
//   Fallback:     if intersection is empty, use union (identical to N2)
//
// Because the machine order sigma is the same in both graphs, whenever
// boundary_m[x] >= 0 and boundary_p[x] >= 0 they always refer to the same
// arc (x, y) — the machine successor y of x is fixed by sigma.
//
//=============================================================================
//
//	Class NB_ParallelN2Inter_MakespanIJSP
//
//=============================================================================

unsigned int NB_ParallelN2Inter_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo mpmp, ms;
	std::queue<int> taskQueue;

	currentMakespan = currentFitness->getValue();
	nTasks = this->schedule->getScheduledTasks();

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// boundary_m[x] = machine successor y of x if (x,y) is boundary in G-, else -1
	// boundary_p[x] = same for G+
	std::vector<int> boundary_m(nTasks, -1);
	std::vector<int> boundary_p(nTasks, -1);

	// Two passes: collect boundary arcs per extreme graph
	for (short int comp = 1; comp <= 2; comp++) {
		std::vector<int>  &boundary    = (comp == 1) ? boundary_m    : boundary_p;
		std::vector<char>  criticalPath(nTasks, false);

		// Seed queue: last tasks on each machine that achieve the makespan
		while (!taskQueue.empty()) taskQueue.pop();
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
					taskQueue.push(task.mp);
					criticalPath[task.mp] = true;

					// Boundary check (same as N2)
					if (mp.mp != -1)
						mpmp = this->schedule->taskInfo[mp.mp];
					if (task.ms != -1)
						ms = this->schedule->taskInfo[task.ms];

					bool isBoundary = (mp.mp == -1 || task.ms == -1
						|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
						|| (!(task.head + task.task->p).EqualComponent(ms.head, comp)
							|| criticalPath[task.ms] == false));

					// Record first time only (BFS may visit the arc multiple times)
					if (isBoundary && boundary[task.mp] == -1)
						boundary[task.mp] = (int)taskId;
				}
			}

			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					taskQueue.push(task.task->jp);
					criticalPath[task.task->jp] = true;
				}
			}
		}
	}

	// Build intersection
	bool anyAdded = false;
	std::vector<char> added(nTasks, false);

	for (unsigned int x = 0; x < nTasks; x++) {
		if (boundary_m[x] >= 0 && boundary_p[x] >= 0 && !added[x]) {
			int y = boundary_m[x];   // == boundary_p[x] (same machine order)
			if (this->numNeighbours < this->neighbours.size()
				&& this->neighbours[this->numNeighbours] != nullptr)
				this->neighbours[this->numNeighbours]->setValues(x, y);
			else
				this->neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(x, y));
			this->numNeighbours++;
			added[x] = true;
			anyAdded = true;
		}
	}

	// Fallback: intersection empty → use N2 union
	if (!anyAdded) {
		for (unsigned int x = 0; x < nTasks; x++) {
			int y = (boundary_m[x] >= 0) ? boundary_m[x]
			      : (boundary_p[x] >= 0) ? boundary_p[x] : -1;
			if (y >= 0 && !added[x]) {
				if (this->numNeighbours < this->neighbours.size()
					&& this->neighbours[this->numNeighbours] != nullptr)
					this->neighbours[this->numNeighbours]->setValues(x, y);
				else
					this->neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(x, y));
				this->numNeighbours++;
				added[x] = true;
			}
		}
	}

	return this->numNeighbours;
}


//=============================================================================

} // namespace IJSP
