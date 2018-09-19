/*
* DueDate.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "FuzzySGS.h"

namespace FJSP {

	//=============================================================================
	//
	//	Abstract class FuzzySGS
	//
	//=============================================================================
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//=====  Copy constructor  ====================================================
	FuzzySGS::FuzzySGS(const FuzzySGS &source) {
		this->schedule = new FuzzySchedule(*source.schedule);
		this->isCreated = source.isCreated;
	}



	//=============================================================================
	//		METHODS
	//=============================================================================
	//=====  Reset  ===============================================================
	void FuzzySGS::reset() {
		if (this->isCreated)
			this->schedule->reset();
	}
}


/*=============================================================================
||
||					CLASS Scheduler
||
=============================================================================*/
//====  Default constructor  ==================================================
Scheduler::Scheduler() {
	_nTasks = _nJobs = _nMachines = 0;
	_schedule = new Schedule();
}


//====  Main constructor  =====================================================
Scheduler::Scheduler(Problem *p) {
	_nTasks = p->numberTasks();
	_nJobs = p->numberJobs();
	_nMachines = p->numberMachines();

	_nextTask.resize(_nJobs, 0);

	for (int i = 0; i < _nJobs; i++)
		if (p->numberTasks(i) == 0)
			_nextTask[i] = -1;

	_problem = p;
	_schedule = new Schedule(p);
}


//====  Copy constructor  =====================================================
Scheduler::Scheduler(const Scheduler & source) {
	_nTasks = source._nTasks;
	_nJobs = source._nJobs;
	_nMachines = source._nMachines;

	_nextTask = source._nextTask;

	_problem = source._problem;
	_schedule = new Schedule(*(source._schedule));
}


//====  Destructor  ===========================================================
Scheduler::~Scheduler() {
	delete _schedule;
}


//====  Method clear  =========================================================
int Scheduler::clear() {
	for (int i = 0; i < _nJobs; i++)
	if (_problem->numberTasks(i) > 0)
		_nextTask[i] = 0;
	else _nextTask[i] = -1;

	_schedule->clear();
	return 0;
}


//====  Metodo scheduleTask  ==================================================
int Scheduler::scheduleTask(int task) {
	int mac, job;
	TFN Stime;
	Task * t = (*_problem)[task];

	mac = t->machine;
	job = t->job;

	// Check if this tasks can be scheduled
	if (t->jp != _schedule->_lastJob[job])
		return 1;

	Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(job));

	// Update the schedule
	_schedule->addTask(task, Stime, -1);
	_nextTask[t->job]++;

	return 0;
}


//====  Metodo buildSchedule  =================================================
int Scheduler::buildSchedule(vector<int> & permutation) {
	int job, task;
	if (_schedule->_nScheduledTasks > 0)
		clear();

	for (int i = 0; i < (int)permutation.size(); i++) {
		job = permutation[i];
		task = _problem->getTaskId(job, _nextTask[job]);
		if (scheduleTask(task) < 0)
			return -1;
	}

	return 0;
}



/*=============================================================================
||					CLASS SGS_Insertion
=============================================================================*/
//====  Metodo scheduleTask  ==================================================
int SGS_Insertion::scheduleTask(int task) {
	int mac, job;
	int mp, ms;
	TFN Stime;
	bool found;
	Task *t = (*_problem)[task];
	
	mac = t->machine;
	job = t->job;

	// Check if this tasks can be scheduled
	if (t->jp != _schedule->_lastJob[job])
		return 1;

	// Find the first time interval in the machine
	Stime = _schedule->endJob(job);
	ms = -1;
	mp = _schedule->_lastMachine[mac];

	while (mp != -1 && _schedule->_nodes[mp].head.isGreaterOrEqualTo(Stime)) {
		ms = mp;
		mp = _schedule->_nodes[ms].mp;
	}
	if (mp != -1)
		Stime = maximum(Stime, _schedule->_nodes[mp].head + _schedule->_nodes[mp].t->p);

	// Look for the minimum starting time for the task
	found = false;
	while (!found && ms != -1) {
		if ((_schedule->_nodes[ms].head).isGreaterOrEqualTo(Stime + t->p))
			found = true;
		else {
			mp = ms;
			Stime = maximum(Stime, _schedule->_nodes[mp].head + _schedule->_nodes[mp].t->p);
			ms = _schedule->_nodes[mp].ms;
		}
	}

	// Update the schedule
	_schedule->addTask(task, Stime, ms);
	_nextTask[t->job]++;

	return 0;
}



/*=============================================================================
||					CLASS SGS_Dense
=============================================================================*/
//====  Metodo buildSchedule  =================================================
int SGS_Dense::buildSchedule(vector<int> & permutation) {
	int scheduled;
	int mac, job, task, newTask;
	TFN Stime, minS;
	vector< vector<int> > situation(_nJobs);

	// Check the position of each task in the permutation
	// NOTE: Empirical results show that this speeds up the method
	for (int i = 0; i < (int)permutation.size(); i++)
		situation[permutation[i]].push_back(i);

	if (_schedule->_nScheduledTasks > 0)
		clear();

	// Look for the task that can be scheduled the earliest
	scheduled = 0;
	while (scheduled < (int)permutation.size()) {
		minS = TFN(Inff, Inff, Inff);
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;

			newTask = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[newTask]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			if (Stime < minS) {
				minS = Stime;
				job = i;
				task = newTask;
			}
			// If there is a tie, choose the task that is more on the lost
			else if (Stime == minS && situation[i][_nextTask[i]] < situation[job][_nextTask[job]]) {
				task = newTask;
				job = i;
			}
		}

		scheduleTask(task);
		scheduled++;
	}

	return 0;
}



/*=============================================================================
||					CLASS SGS_fGYT1
=============================================================================*/
//====  Metodo buildSchedule  =================================================
int SGS_fGYT1::buildSchedule(vector<int> & permutation) {
	int position, scheduled;
	int task, newTask, mac;
	TFN Stime, Ctime;
	TFN minS, minC, threshold;

	vector< vector<int> > situation(_nJobs);

	// Keep the position of each task in the permutation to solve the ties faster
	for (int i = 0; i < (int)permutation.size(); i++)
		situation[permutation[i]].push_back(i);

	scheduled = 0;

	while (scheduled < (int)permutation.size()) {
		// Look for the minimum completion time
		minC = TFN(Inff, Inff, Inff);
		minS = TFN(Inff, Inff, Inff);
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			Ctime = Stime + ((*_problem)[task])->p;

			if (Stime < minS) minS = Stime;

			minC = minimum(minC, Ctime);
		}


		// Look for the tasks that can begin before minC and
		// choose the one that is more on the left
		threshold.a1 = (float)minS.a1 + _delta * (minC.a1 - minS.a1);
		threshold.a2 = minS.a2 + _delta * (minC.a2 - minS.a2);
		threshold.a3 = minS.a3 + _delta * (minC.a3 - minS.a3);

		task = -1; position = _nTasks + 1;
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			if ((Stime.a1 < threshold.a1 || Stime.a2 < threshold.a2 || Stime.a3 < threshold.a3)
				&& situation[i][_nextTask[i]] < position) {
				newTask = task;
				position = situation[i][_nextTask[i]];
			}
		}

		scheduleTask(newTask);
		scheduled++;
	}

	return 0;
}



/*=============================================================================
||					CLASS SGS_fGYT2
=============================================================================*/
//====  Metodo buildSchedule  =================================================
int SGS_fGYT2::buildSchedule(vector<int> & permutation) {
	int position, scheduled;
	int task, newTask, mac;
	TFN Stime, Ctime;
	TFN minS, minC, threshold;
	bool isConflict;

	vector<TFN> setA;
	vector< vector<int> > situation(_nJobs);

	// Keep the position of each task in the permutation to solve the ties faster
	for (int i = 0; i < (int)permutation.size(); i++)
		situation[permutation[i]].push_back(i);

	scheduled = 0;

	while (scheduled < (int)permutation.size()) {
		minC = TFN(Inff, Inff, Inff);
		minS = TFN(Inff, Inff, Inff);

		// Looks for the minimum completion time
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			Ctime = Stime + ((*_problem)[task])->p;

			if (Stime < minS) minS = Stime;

			if (Ctime.isLesserThan(minC)) {
				minC = Ctime;
				setA.clear();
				setA.push_back(Ctime);
			}
			else if (Ctime.a1 <= minC.a1 || Ctime.a2 <= minC.a2 || Ctime.a3 <= minC.a3) {
				minC = minimum(minC, Ctime);
				setA.push_back(Ctime);
			}
		}

		// Look for the tasks that are in conflict and
		// choose the one that is more on the left
		task = -1; position = _nTasks + 1;
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			isConflict = true;
			for (int j = 0; j < (int)setA.size(); j++) {
				threshold.a1 = minS.a1 + _delta * (setA[j].a1 - minS.a1);
				threshold.a2 = minS.a2 + _delta * (setA[j].a2 - minS.a2);
				threshold.a3 = minS.a3 + _delta * (setA[j].a3 - minS.a3);

				if (Stime.a1 >= threshold.a1 && Stime.a2 >= threshold.a2 && Stime.a3 >= threshold.a3) {
					isConflict = false;
					break;
				}
			}
			if (isConflict && situation[i][_nextTask[i]] < position) {
				newTask = task;
				position = situation[i][_nextTask[i]];
			}
		}

		scheduleTask(newTask);
		scheduled++;
	}

	return 0;
}



/*=============================================================================
||					CLASS SGS_eGYT
=============================================================================*/
//====  Metodo buildSchedule  =================================================
int SGS_eGYT::buildSchedule(vector<int> & permutation) {
	int position, scheduled;
	int task, mac, job, machine;
	float minS, minC;
	double threshold;
	TFN Stime, Ctime;

	vector< vector<int> > situation(_nJobs);
	vector<int> conflict;
	vector<TFN> starts(_nJobs);

	// Keep the position of each task in the permutation to solve the ties faster
	for (int i = 0; i < (int)permutation.size(); i++)
		situation[permutation[i]].push_back(i);

	scheduled = 0;
	while (scheduled < (int)permutation.size()) {
		minS = minC = Inff;

		// Look for the minimum expected completion time
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			starts[i] = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			Ctime = starts[i] + (*_problem)[task]->p;
			if (Ctime.expectedValue() < minC) {
				minC = Ctime.expectedValue();
				machine = mac;
			}
		}

		// Look for the minimum expected starting time
		conflict.clear();
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			if (mac == machine) {
				if (starts[i].expectedValue() < minC) {
					conflict.push_back(i);
					if (starts[i].expectedValue() < minS)
						minS = starts[i].expectedValue();
				}
			}
		}

		// Compute the threshold based on the delta value
		threshold = minS + _delta * (minC - minS);

		// Choose a task from the conflict set using the permutation order
		position = _nTasks + 1;
		for (int i = 0; i < (int)conflict.size(); i++) {
			job = conflict[i];
			if (starts[job].expectedValue() < threshold && situation[job][_nextTask[job]] < position) {
				position = situation[job][_nextTask[job]];
				task = _problem->getTaskId(job, _nextTask[job]);
			}
		}

		scheduleTask(task);
		scheduled++;
	}

	return 0;
}



/*=============================================================================
||					CLASS SGS_Sakawa
=============================================================================*/
//====  Metodo buildSchedule  =================================================
int SGS_Sakawa::buildSchedule(vector<int> & permutation) {
	int position, scheduled;
	int task, mac, machine, chosen;
	float minS;
	TFN Stime, Ctime, minC;

	vector< vector<int> > situation(_nJobs);

	// Keep the position of each task in the permutation to solve the ties faster
	for (int i = 0; i < (int)permutation.size(); i++)
		situation[permutation[i]].push_back(i);

	scheduled = 0;
	while (scheduled < (int)permutation.size()) {
		minS = Inff;
		minC = TFN(Inff, Inff, Inff);

		// Look for the minimum completion time following Sakawa's criteria
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
			Ctime = Stime + (*_problem)[task]->p;
			if (Ctime.a1 < minC.a1) {
				minC = Ctime;
				machine = mac;
			}
		}

		position = _nTasks + 1;
		for (int i = 0; i < _nJobs; i++) {
			if (_nextTask[i] >= _problem->numberTasks(i))
				continue;
			task = _problem->getTaskId(i, _nextTask[i]);
			mac = (*_problem)[task]->machine;

			if (mac == machine) {
				Stime = maximum(_schedule->endMachine(mac), _schedule->endJob(i));
				if (Stime.a1 < minC.a1 || Stime.a2 < minC.a2 || Stime.a3 < minC.a3) {
					if (situation[i][_nextTask[i]] < position) {
						position = situation[i][_nextTask[i]];
						chosen = _problem->getTaskId(i, _nextTask[i]);
					}
				}
			}
		}

		scheduleTask(chosen);
		scheduled++;
	}

	return 0;
}

