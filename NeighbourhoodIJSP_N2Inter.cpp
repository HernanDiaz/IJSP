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
	FuzzyFW::Crisp currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo mpmp, ms;
	std::queue<int> taskQueue;

	currentMakespan = currentFitness->getValue();
	nTasks = this->schedule->getScheduledTasks();

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// boundary[x] = machine successor y of x if (x, y) is a boundary arc, else -1.
	//
	// This neighbourhood existed to intersect the boundary arcs of the two
	// extreme graphs G- and G+, one per interval endpoint, and to fall back to
	// their union when the intersection came out empty. On crisp times G- and
	// G+ are the same graph: the second pass ran identical code over identical
	// data and produced a second copy of the same vector, so the intersection,
	// the union and either operand all coincide. One pass, one vector, and the
	// empty-intersection fallback has nothing left to fall back to.
	//
	// What this means for a setup file is worth stating plainly: on crisp data
	// ijsp.makespan.n2inter is the same neighbourhood as ijsp.makespan.n2.
	std::vector<int> boundary(nTasks, -1);
	std::vector<char> criticalPath(nTasks, false);

	// Seed queue: last tasks on each machine that achieve the makespan
	while (!taskQueue.empty()) taskQueue.pop();
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

				// Boundary check (same as N2)
				if (mp.mp != -1)
					mpmp = this->schedule->taskInfo[mp.mp];
				if (task.ms != -1)
					ms = this->schedule->taskInfo[task.ms];

				bool isBoundary = (mp.mp == -1 || task.ms == -1
					|| !((mpmp.head + mpmp.task->p) == mp.head)
					|| (!((task.head + task.task->p) == ms.head)
						|| criticalPath[task.ms] == false));

				// Record first time only (BFS may visit the arc multiple times)
				if (isBoundary && boundary[task.mp] == -1)
					boundary[task.mp] = (int)taskId;
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

	// Build the neighbourhood from the boundary arcs
	for (unsigned int x = 0; x < nTasks; x++) {
		if (boundary[x] >= 0) {
			int y = boundary[x];
			if (this->numNeighbours < this->neighbours.size()
				&& this->neighbours[this->numNeighbours] != nullptr)
				this->neighbours[this->numNeighbours]->setValues(x, y);
			else
				this->neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(x, y));
			this->numNeighbours++;
		}
	}

	return this->numNeighbours;
}


//=============================================================================

} // namespace IJSP
