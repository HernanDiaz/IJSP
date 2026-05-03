/*
 * NeighbourhoodIJSP_N2Plus.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N2Plus.h"
#include <iostream>
namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2Plus_MakespanIJSP
//
//=============================================================================

unsigned int NB_ParallelN2Plus_MakespanIJSP::findNewNeighbours(
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

	// Only G+ (comp = 2)
	short int comp = 2;

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
				if (!added[task.mp]) {
					if (mp.mp != -1)
						mpmp = this->schedule->taskInfo[mp.mp];
					if (task.ms != -1)
						ms = this->schedule->taskInfo[task.ms];

					if (mp.mp == -1 || task.ms == -1
						|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
						|| (!(task.head + task.task->p).EqualComponent(ms.head, comp)
							|| criticalPath[task.ms] == false)) {

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
		}

		if (task.task->jp != -1) {
			jp = this->schedule->taskInfo[task.task->jp];
			if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
				taskQueue.push(task.task->jp);
				criticalPath[task.task->jp] = true;
			}
		}
	}

	return this->numNeighbours;
}



} // namespace IJSP
