/*
* Neighbourhood.cpp
*
*  Created on: Oct 12, 2019
*      Author: hdiaz
*/

#include "NeighbourhoodIJSP_Cmax.h"

#include <iostream>
namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN1_MakespanIJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN1_MakespanIJSP::NB_ParallelN1_MakespanIJSP(
	const NB_ParallelN1_MakespanIJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourIJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN1_MakespanIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_ESTIMATOR_HEADSTAILS) == 0)
		this->estimator = Estimator::ESTIM_HEADTAILS;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN1_MakespanIJSP::~NB_ParallelN1_MakespanIJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelN1_MakespanIJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Interval::Maximum maxType = FuzzyFW::Interval::M_COMPONENT;
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy IJSP Schedules are allowed.";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	ProblemIJSP * problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to IJSP problems";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;

	std::vector<char> visited(problem->getNumberTasks(), false);
	//this->tails.clear();
	//this->tails.resize(0); //TODO
	if (tails.size() != problem->getNumberTasks()) {
		this->tails.resize(problem->getNumberTasks());
	}
	/*
	for (int i = 0; i < tails.size(); i++) {
		tails[i].a = 0;
		tails[i].b = 0;
	}*/

	// Look for tasks with no successors
	for (size_t i = 0; i < schedule->lastTaskJob.size(); i++) {
		taskIdx = schedule->lastTaskJob[i];
		mac = schedule->taskInfo[taskIdx].task->machine;
		if (taskIdx == schedule->lastTaskMachine[mac])
			taskQueue.push(taskIdx);
	}

	// Backwards propagation
	while (taskQueue.size() > 0) {
		taskIdx = taskQueue.front();
		taskQueue.pop();

		// Update tail
		ms = schedule->taskInfo[taskIdx].ms;
		js = schedule->taskInfo[taskIdx].task->js;

		if (ms != -1 && js != -1)
			this->tails[taskIdx] = 
			maximum(this->tails[ms] + schedule->taskInfo[ms].task->p,
				this->tails[js] + schedule->taskInfo[js].task->p, maxType);
		else if (ms != -1)
			this->tails[taskIdx] = 
			this->tails[ms] + schedule->taskInfo[ms].task->p;
		else if (js != -1)
			this->tails[js] + schedule->taskInfo[js].task->p;
		else this->tails[taskIdx] = FuzzyFW::Interval(0, 0);

		// Propagate
		mp = schedule->taskInfo[taskIdx].mp;
		jp = schedule->taskInfo[taskIdx].task->jp;

		if (mp != -1) {
			if (visited[mp] || schedule->taskInfo[mp].task->js == -1)
				taskQueue.push(mp);
			else visited[mp] = true;
		}

		if (jp != -1) {
			if (visited[jp] || schedule->taskInfo[jp].ms == -1)
				taskQueue.push(jp);
			else visited[jp] = true;
		}
	}
}



//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;

	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 2; comp++) {
		// Look for the tasks defining the makespan value
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp))
				taskQueue.push(this->schedule->lastTaskMachine[i]);
		}

		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();
			task = this->schedule->taskInfo[taskId];
			if (task.mp != -1 && task.mp != task.task->jp) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					taskQueue.push(task.mp);
					if (!added[task.mp]) {
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

			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp))
					taskQueue.push(task.task->jp);
			}
		}
	}

	return this->numNeighbours;
}





//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_MakespanIJSP::evaluateNeighbour(
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
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

	// Initialize variables
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

	// Update predecessors and successors
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

	// Update heads
	taskQueue.push(arc->y); 
	taskQueue.push(arc->x);	//TODO comment this

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		jpz = newSolution->taskInfo[z].task->jp;
		mpz = newSolution->taskInfo[z].mp;
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any predecessor is null
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
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
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
void NB_ParallelN1_MakespanIJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Update the current solution
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

	// Update tails for the next step
	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	taskQueue.push(arc->x);
	taskQueue.push(arc->y); //TODO comment this

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
		else if (msz != -1) {
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		}
		else if (jsz != -1) {
		 	newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		}
		if ( (msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail,	FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1)
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
			if (this->schedule->taskInfo[z].task->jp != -1)
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
		}
	}
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_MakespanIJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelN1_MakespanIJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessInterval(FuzzyFW::Interval(Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	//std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	//this->numNeighbours--;
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	unsigned int x = arc->x;
	unsigned int y = arc->y;

	mac = schedule->taskInfo[x].task->machine;
	mpy = schedule->taskInfo[y].mp;
	jpy = schedule->taskInfo[y].task->jp;
	msy = schedule->taskInfo[y].ms;
	msx = schedule->taskInfo[x].ms;
	mpx = schedule->taskInfo[x].mp;
	jpx = schedule->taskInfo[x].task->jp;

	if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
		jsx = -1;
	else
		jsx = schedule->taskInfo[x].task->js;
	if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
		jsy = -1;
	else
		jsy = schedule->taskInfo[y].task->js;
	
	// New tail for task X
	if (jsx != -1 && msy != -1)
		tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
			this->tails[msy] + schedule->taskInfo[msy].task->p,	FuzzyFW::Interval::M_COMPONENT);
	else if (jsx != -1)
		tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::Interval(0, 0);

	// New tail for task Y
	if (jsy != -1)
		tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
			tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
	else
		tailY = tailX + schedule->taskInfo[x].task->p;

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
			schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else if (mpx != -1)
		headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
	else headY = FuzzyFW::Interval(0, 0);

	// New head for task X
	if (jpx != -1)
		headX = maximum(headY + schedule->taskInfo[y].task->p,
			schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else headX = headY + schedule->taskInfo[y].task->p;

	makespan = maximum(headX + schedule->taskInfo[x].task->p + tailX,
		headY + schedule->taskInfo[y].task->p + tailY,
		FuzzyFW::Interval::M_COMPONENT);

	arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_MakespanIJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_MakespanIJSP::quickSort(const int left, const int right,
	const FuzzyFW::SharedVars *svars) {

	int pivot, pos;

	if (left >= right)
		return;

	pivot = svars->rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->getEstimation(i, svars)->isBetterThan(
			this->getEstimation(right, svars))) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, svars);
	this->quickSort(pos + 1, right, svars);
}


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_ParallelN1_MakespanIJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}



//=============================================================================
//
//	Class NB_ParallelN2_MakespanIJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN2_MakespanIJSP::NB_ParallelN2_MakespanIJSP(
	const NB_ParallelN2_MakespanIJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourIJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN2_MakespanIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_ESTIMATOR_HEADSTAILS) == 0)
		this->estimator = Estimator::ESTIM_HEADTAILS;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN2_MakespanIJSP::~NB_ParallelN2_MakespanIJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelN2_MakespanIJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Interval::Maximum maxType = FuzzyFW::Interval::M_COMPONENT;
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy IJSP Schedules are allowed.";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	ProblemIJSP * problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to IJSP problems";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;

	std::vector<char> visited(problem->getNumberTasks(), false);
	this->tails.resize(problem->getNumberTasks());

	// Look for tasks with no successors
	for (size_t i = 0; i < schedule->lastTaskJob.size(); i++) {
		taskIdx = schedule->lastTaskJob[i];
		mac = schedule->taskInfo[taskIdx].task->machine;
		if (taskIdx == schedule->lastTaskMachine[mac])
			taskQueue.push(taskIdx);
	}

	// Backwards propagation
	while (taskQueue.size() > 0) {
		taskIdx = taskQueue.front();
		taskQueue.pop();

		// Update tail
		ms = schedule->taskInfo[taskIdx].ms;
		js = schedule->taskInfo[taskIdx].task->js;

		if (ms != -1 && js != -1)
			this->tails[taskIdx] =
			maximum(this->tails[ms] + schedule->taskInfo[ms].task->p,
				this->tails[js] + schedule->taskInfo[js].task->p, maxType);
		else if (ms != -1)
			this->tails[taskIdx] =
			this->tails[ms] + schedule->taskInfo[ms].task->p;
		else if (js != -1)
			this->tails[js] + schedule->taskInfo[js].task->p;
		else this->tails[taskIdx] = FuzzyFW::Interval(0, 0);

		// Propagate
		mp = schedule->taskInfo[taskIdx].mp;
		jp = schedule->taskInfo[taskIdx].task->jp;

		if (mp != -1) {
			if (visited[mp] || schedule->taskInfo[mp].task->js == -1)
				taskQueue.push(mp);
			else visited[mp] = true;
		}

		if (jp != -1) {
			if (visited[jp] || schedule->taskInfo[jp].ms == -1)
				taskQueue.push(jp);
			else visited[jp] = true;
		}
	}
}



//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelN2_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
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

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 2; comp++) {
		// Look for the tasks defining the makespan value
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
						// We swap only the tasks at the extreme of a critical block
						if (mp.mp != -1) {
							mpmp = this->schedule->taskInfo[mp.mp];
						}
						if (task.ms != -1) {
							ms = this->schedule->taskInfo[task.ms];
						}

						// Add arc if it is at the boundary of a critical block:
						// either mp has no machine predecessor (first in block),
						// or task has no machine successor (last in block),
						// or mpmp is not on the critical path,
						// or ms is not on the critical path
						if (mp.mp == -1 || task.ms == -1
							|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
							|| !criticalPath[task.ms]
							|| !(task.head + task.task->p).EqualComponent(ms.head, comp)) {

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
					taskQueue.push(task.task->jp);
					criticalPath[task.task->jp] = true;
				}
			}
		}
	}

	return this->numNeighbours;
}





//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness * NB_ParallelN2_MakespanIJSP::evaluateNeighbour(
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
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

	// Initialize variables
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

	// Update predecessors and successors
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

	// Update heads
	taskQueue.push(arc->y); //TODO
	//taskQueue.push(arc->x);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		jpz = newSolution->taskInfo[z].task->jp;
		mpz = newSolution->taskInfo[z].mp;
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any predecessor is null
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
			if (msz != -1) {
				taskQueue.push(msz);
			}
			if (jsz != -1) {
				taskQueue.push(jsz);
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
	//std::cout << std::endl;
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
	std::queue<int> taskQueue;
	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Update the current solution
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

	// Update tails for the next step
	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	std::vector<int> tailsUpdated;
	tailsUpdated.resize(this->schedule->getScheduledTasks(), 0);

	taskQueue.push(arc->x);
	tailsUpdated[arc->x] = 1;
	taskQueue.push(arc->y);
	tailsUpdated[arc->y] = 1;

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		//std::cout <<"Tail "<< z << std::endl;
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
		else if (msz != -1) {
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		}

		else if (jsz != -1) {
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		}
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail, FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			if ((this->schedule->taskInfo[z].mp != -1)){// && !tailsUpdated[this->schedule->taskInfo[z].mp]){
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
				if (tailsUpdated[this->schedule->taskInfo[z].mp]) {
					//std::cout << "Repeated task:" << this->schedule->taskInfo[z].mp <<"\tRepetitions: "<< tailsUpdated[this->schedule->taskInfo[z].mp]<<"Tail value: "<< newTail<<std::endl;
				}
				tailsUpdated[this->schedule->taskInfo[z].mp]++;
			}
			if ((this->schedule->taskInfo[z].task->jp != -1)){// && !tailsUpdated[this->schedule->taskInfo[z].task->jp]) {
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
				if (tailsUpdated[this->schedule->taskInfo[z].task->jp]) {
					//std::cout << "Repeated task:" << this->schedule->taskInfo[z].task->jp <<"\tRepetitions: " << tailsUpdated[this->schedule->taskInfo[z].task->jp] << "Tail value: " << newTail << std::endl;
				}
				tailsUpdated[this->schedule->taskInfo[z].task->jp]++;
			}
		}
	}
	//std::cout<< std::endl;
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN2_MakespanIJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelN2_MakespanIJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessInterval(FuzzyFW::Interval(Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN2_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	//std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	//this->numNeighbours--;
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN2_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	unsigned int x = arc->x;
	unsigned int y = arc->y;

	mac = schedule->taskInfo[x].task->machine;
	mpy = schedule->taskInfo[y].mp;
	jpy = schedule->taskInfo[y].task->jp;
	msy = schedule->taskInfo[y].ms;
	msx = schedule->taskInfo[x].ms;
	mpx = schedule->taskInfo[x].mp;
	jpx = schedule->taskInfo[x].task->jp;

	if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
		jsx = -1;
	else
		jsx = schedule->taskInfo[x].task->js;
	if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
		jsy = -1;
	else
		jsy = schedule->taskInfo[y].task->js;

	// New tail for task X
	if (jsx != -1 && msy != -1)
		tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
			this->tails[msy] + schedule->taskInfo[msy].task->p, FuzzyFW::Interval::M_COMPONENT);
	else if (jsx != -1)
		tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::Interval(0, 0);

	// New tail for task Y
	if (jsy != -1)
		tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
			tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
	else
		tailY = tailX + schedule->taskInfo[x].task->p;

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
			schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else if (mpx != -1)
		headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
	else headY = FuzzyFW::Interval(0, 0);

	// New head for task X
	if (jpx != -1)
		headX = maximum(headY + schedule->taskInfo[y].task->p,
			schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else headX = headY + schedule->taskInfo[y].task->p;

	makespan = maximum(headX + schedule->taskInfo[x].task->p + tailX,
		headY + schedule->taskInfo[y].task->p + tailY,
		FuzzyFW::Interval::M_COMPONENT);

	arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN2_MakespanIJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN2_MakespanIJSP::quickSort(const int left, const int right,
	const FuzzyFW::SharedVars *svars) {

	int pivot, pos;

	if (left >= right)
		return;

	pivot = svars->rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->getEstimation(i, svars)->isBetterThan(
			this->getEstimation(right, svars))) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, svars);
	this->quickSort(pos + 1, right, svars);
}


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_ParallelN2_MakespanIJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}


//TODO
//=============================================================================
//
//	Class NB_ParallelN3_MakespanIJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN3_MakespanIJSP::NB_ParallelN3_MakespanIJSP(
	const NB_ParallelN3_MakespanIJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourIJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN3_MakespanIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_ESTIMATOR_HEADSTAILS) == 0)
		this->estimator = Estimator::ESTIM_HEADTAILS;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN3_MakespanIJSP::~NB_ParallelN3_MakespanIJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelN3_MakespanIJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Interval::Maximum maxType = FuzzyFW::Interval::M_COMPONENT;
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy IJSP Schedules are allowed.";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	ProblemIJSP * problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to IJSP problems";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;

	std::vector<char> visited(problem->getNumberTasks(), false);
	this->tails.resize(0);
	this->tails.resize(problem->getNumberTasks());

	// Look for tasks with no successors
	for (size_t i = 0; i < schedule->lastTaskJob.size(); i++) {
		taskIdx = schedule->lastTaskJob[i];
		mac = schedule->taskInfo[taskIdx].task->machine;
		if (taskIdx == schedule->lastTaskMachine[mac])
			taskQueue.push(taskIdx);
	}

	// Backwards propagation
	while (taskQueue.size() > 0) {
		taskIdx = taskQueue.front();
		taskQueue.pop();

		// Update tail
		ms = schedule->taskInfo[taskIdx].ms;
		js = schedule->taskInfo[taskIdx].task->js;

		if (ms != -1 && js != -1)
			this->tails[taskIdx] =
			maximum(this->tails[ms] + schedule->taskInfo[ms].task->p,
				this->tails[js] + schedule->taskInfo[js].task->p, maxType);
		else if (ms != -1)
			this->tails[taskIdx] =
			this->tails[ms] + schedule->taskInfo[ms].task->p;
		else if (js != -1)
			this->tails[js] + schedule->taskInfo[js].task->p;
		else this->tails[taskIdx] = FuzzyFW::Interval(0, 0);

		// Propagate
		mp = schedule->taskInfo[taskIdx].mp;
		jp = schedule->taskInfo[taskIdx].task->jp;

		if (mp != -1) {
			if (visited[mp] || schedule->taskInfo[mp].task->js == -1)
				taskQueue.push(mp);
			else visited[mp] = true;
		}

		if (jp != -1) {
			if (visited[jp] || schedule->taskInfo[jp].ms == -1)
				taskQueue.push(jp);
			else visited[jp] = true;
		}
	}
}



//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN3_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo  mpmpmp = {}, mpmp, ms;
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

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 2; comp++) {
		// Look for the tasks defining the makespan value
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp)) {
				taskQueue.push(this->schedule->lastTaskMachine[i]);
			}
		}

		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();

			task = this->schedule->taskInfo[taskId];
			//if there is at least one marchine predecesor  
			if (task.mp != -1) {
				mp = this->schedule->taskInfo[task.mp];
				// if we are in the critical path
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					taskQueue.push(task.mp);
					criticalPath[task.mp] = true;
					// We swap only the tasks at the exteme of a critical block
					if (mp.mp != -1) {
						mpmp = this->schedule->taskInfo[mp.mp];
					}
					if (task.ms != -1) {
						ms = this->schedule->taskInfo[task.ms];
					}
					//if the task is not added yet and mp is of different job
					if (!added0[task.mp] && task.mp != task.task->jp) {
						//critical block of 1 arch
						//if we are in the first arch, or the last one
						// or the mp->mp does not belong to the critical path
						// or the ms does not belong to the critical path
						if (mp.mp == -1 || task.ms == -1
							|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
							|| !(task.head + task.task->p).EqualComponent(ms.head, comp)) {

							this->addNeighbour(task.mp, taskId);
							added0[task.mp] = true;
						}

					}
				
					if (mp.mp != -1 && mpmp.mp!=-1) {
						mpmpmp = this->schedule->taskInfo[mpmp.mp];
					}
					//critic block of two or more arcs mpmp->mp->task
					// mp.mp belongs to critical path
					// and mpmp.mp does not belong to critical path or is the first task
					// or  ms does not belong to critical path or is the last task
					if (mp.mp !=-1 && (mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
						&& ((mpmp.mp == -1 || !(mpmpmp.head + mpmpmp.task->p).EqualComponent(mpmp.head, comp))
						|| (task.ms == -1 || !(task.head + task.task->p).EqualComponent(ms.head, comp))))  {
						if (!added3[task.mp]) {
							this->addNeighbour(mp.mp, task.mp, taskId, 3);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == NULL) {
								this->discardNeighbour(this->numNeighbours - 1);
							}
							else added3[task.mp] = true;
						}
						
						if (!added1[task.mp] && (mpmp.mp == -1 || !(mpmpmp.head + mpmpmp.task->p).EqualComponent(mpmp.head, comp))) {
							this->addNeighbour(mp.mp, task.mp, taskId, 1);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == NULL) {
								this->discardNeighbour(this->numNeighbours - 1);
							}
							else added1[task.mp] = true;
						}
						if (!added2[task.mp] && (task.ms == -1 || !(task.head + task.task->p).EqualComponent(ms.head, comp))) {
							this->addNeighbour(mp.mp, task.mp, taskId, 2);
							if (this->evaluateNeighbour(this->numNeighbours - 1, svars) == NULL) {
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
					taskQueue.push(task.task->jp);
					criticalPath[task.task->jp] = true;
				}
			}
		}
	}

	//std::cout << "total neighbours: " << this->numNeighbours << std::endl;
	/*
	for (int i = 0; i < this->numNeighbours; i++) {
		if (this->neighbours[i] == NULL) {
			std::cout << "Null neigh in position " << i << std::endl;
		}
	}
	std::cout << "end neigh cal" << this->numNeighbours << std::endl;
	*/
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

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0 || arc->z < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

	if (arc->tipo == 0) {
		// 0 ab to ->b->a->   solo un arco
		//   xy to ->y->x->
		// Initialize variables
		mac = newSolution->taskInfo[arc->x].task->machine;
		msy = newSolution->taskInfo[arc->y].ms;
		mpx = newSolution->taskInfo[arc->x].mp;

		// Update predecessors and successors
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

		// Update heads
		taskQueue.push(arc->y); //TODO
		//taskQueue.push(arc->x);
	}

	else if (arc->tipo == 1) {
		// 1 abc to ->b  c->a->   cuando  mpmpmp == -1 o no bloque critico
		//  xyz to ->y z->x->
		// Initialize variables
		mac = newSolution->taskInfo[arc->x].task->machine;
		msz = newSolution->taskInfo[arc->z].ms;
		mpx = newSolution->taskInfo[arc->x].mp;
		
		// Update predecessors and successors
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

		// Update heads
		taskQueue.push(arc->y);
		//taskQueue.push(arc->z);
		//taskQueue.push(arc->x);
		
		
	}
	else if (arc->tipo == 2) {
		// 2def to ->f->d  e-> cuando ms == -1 o no bloque critico
		// xyz to ->z->x y-> 
	    // Initialize variables
		mac = newSolution->taskInfo[arc->x].task->machine;
		mpx = newSolution->taskInfo[arc->x].mp;
		msz = newSolution->taskInfo[arc->z].ms;
		
		// Update predecessors and successors
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

		// Update heads
		taskQueue.push(arc->z);
		//taskQueue.push(arc->x);
		//taskQueue.push(arc->y);
		
		
	}
	else if (arc->tipo == 3) {
		//3 abc to ->c->b->a-> en los dos
		// xyz to ->z->y->x-> 
		// Initialize variables
		mac = newSolution->taskInfo[arc->x].task->machine;
		mpx = newSolution->taskInfo[arc->x].mp;
		msz = newSolution->taskInfo[arc->z].ms;

		// Update predecessors and successors
		if (mpx != -1)
			newSolution->taskInfo[mpx].ms = arc->z;
		newSolution->taskInfo[arc->y].mp = arc->z;
		newSolution->taskInfo[arc->z].ms = arc->y;
		newSolution->taskInfo[arc->x].mp = arc->y;
		newSolution->taskInfo[arc->y].ms = arc->x;
		newSolution->taskInfo[arc->x].ms = msz;
		if (msz != -1)
			newSolution->taskInfo[msz].mp = arc->x;
		else
			newSolution->lastTaskMachine[mac] = arc->x;

		// Update heads
		taskQueue.push(arc->z);
		//taskQueue.push(arc->y);
		//taskQueue.push(arc->x);
	}
	//std::cout <<"Start heads"<< std::endl;
	int numHeadsCalculated = 0;
	while (!taskQueue.empty()) {
		z = taskQueue.front();
		//std::cout <<"Head "<< z << std::endl;
		taskQueue.pop();
		numHeadsCalculated++;
		jpz = newSolution->taskInfo[z].task->jp;
		mpz = newSolution->taskInfo[z].mp;
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any predecessor is null
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
			//std::cout << "new Head: " << newHead <<std::endl;
			if (improvement && jsz == -1) {
				lowerBound->setValue(newSolution->taskInfo[z].head
					+ newSolution->taskInfo[z].task->p);
				if (lowerBound->isWorseThan(currentFitness)) {
					delete lowerBound;
					delete newSolution;
					return NULL;
				}
			}
			if (numHeadsCalculated > newSolution->getScheduledTasks() * 3) {
				//There is a loop (no puedo calcular el loop dependiendo del valor porque en Tabu se pueden escoger fitness peores)
				delete lowerBound;
				delete newSolution;
				return NULL;
			}
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}
	//std::cout <<"End heads"<< std::endl;
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
	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Update the current solution
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

	// Update tails for the next step
	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	switch (arc->tipo) {
		case 0: {
			taskQueue.push(arc->x);
			//taskQueue.push(arc->y);
		}
		break;
		case 1: {
			taskQueue.push(arc->x);
			//taskQueue.push(arc->z);
			//taskQueue.push(arc->y);
     	}
		break;
		case 2: {
			taskQueue.push(arc->y);
			//taskQueue.push(arc->x);
			//taskQueue.push(arc->z);
		}
		break;
		case 3: {
			taskQueue.push(arc->x);
			//taskQueue.push(arc->y);
			//taskQueue.push(arc->z);
		}
	};
	//std::cout << "Start tails " << std::endl;
	while (!taskQueue.empty()) {
		z = taskQueue.front();
		//std::cout <<"Tail "<< z << std::endl;
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
		else if (msz != -1) {
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		}
		else if (jsz != -1) {
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		}
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail, FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			//std::cout << "newTail " << newTail << std::endl;
			if (this->schedule->taskInfo[z].mp != -1)
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
			if (this->schedule->taskInfo[z].task->jp != -1)
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
		}
	}
	//std::cout << "End tails " << std::endl;
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN3_MakespanIJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	//std::cout << "get estimation for:" <<idx<<std::endl;
	//std::cout << "num nei" << this->numNeighbours << std::endl;


	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelN3_MakespanIJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessInterval(FuzzyFW::Interval(Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}

//-----  add neighbours  -----------------------------------------------------
void NB_ParallelN3_MakespanIJSP::addNeighbour(const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int tipo) {
	if (this->numNeighbours < this->neighbours.size() && this->neighbours[this->numNeighbours] != NULL) {
		this->neighbours[this->numNeighbours]->setValues(x, y, z, tipo);
		//std::cout << "1 adding nei in position" << this->numNeighbours << std::endl;
	}

	if (this->numNeighbours < this->neighbours.size() && this->neighbours[this->numNeighbours] == NULL) {
		this->neighbours[this->numNeighbours] = new NeighbourIJSP_Arc(x, y, z, tipo);
		//this->neighbours[this->numNeighbours]->setValues(x, y, z, tipo);
		//std::cout << "1 adding nei in position" << this->numNeighbours << std::endl;
	}
	else {
		this->neighbours.push_back(new NeighbourIJSP_Arc(x, y, z, tipo));

		//std::cout << "2 adding nei in position" << this->neighbours.size() << std::endl;
	}
	//std::cout << "Adding neigh in position: " << this->numNeighbours << std::endl;
	this->numNeighbours++;


}

//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN3_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	//std::cout << "Discarding neigh in position: " << idx << std::endl;
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	if (idx != this->numNeighbours - 1) {
		std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	}
	this->numNeighbours--;
	if (this->neighbours[idx] == NULL) {
		//std::cout << "nei is null" << idx << std::endl;
	}
	//std::cout << "Total neigh " << this->numNeighbours << std::endl;
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN3_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, tailZ, headX, headY, headZ;
    FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->tipo == 0) {
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		int jsx, jsy, jsz;
	
		int mac = schedule->taskInfo[x].task->machine;
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

		// New tail for task X
		if (jsx != -1 && msy != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msy] + schedule->taskInfo[msy].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msy != -1)
			tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		// New tail for task Y
		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailX + schedule->taskInfo[x].task->p;

		// New head for task Y
		if (mpx != -1 && jpy != -1)
			headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpy != -1)
			headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
		else headY = FuzzyFW::Interval(0, 0);

		// New head for task X
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
		// 1 abc to ->b  c->a->   cuando  mpmpmp == -1 o no bloque critico
		//  xyz to ->y z->x->
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int mac = schedule->taskInfo[x].task->machine;
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

		// New tail for task X
		if (jsx != -1 && msz != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msz != -1)
			tailX = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		// New tail for task Z
		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailX + schedule->taskInfo[x].task->p;

		// New tail for task Y
		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailZ + schedule->taskInfo[z].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailZ + schedule->taskInfo[z].task->p;

		// New head for task Y
		if (mpx != -1 && jpy != -1)
			headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpy != -1)
			headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
		else headY = FuzzyFW::Interval(0, 0);

		// New head for task Z
		if (jpz != -1)
			headZ = maximum(headY + schedule->taskInfo[y].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headZ = headY + schedule->taskInfo[y].task->p;

		// New head for task X
		if (jpx != -1)
			headX = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headZ + schedule->taskInfo[z].task->p;

		makespan =	maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX,	headY + schedule->taskInfo[y].task->p + tailY, FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ, FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}
	if (arc->tipo == 2) {
		// 2def to ->f->d  e-> cuando ms == -1 o no bloque critico
		// xyz to ->z->x y-> 
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int mac = schedule->taskInfo[x].task->machine;
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

		// New tail for task Y
		if (jsy != -1 && msz != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsy != -1)
			tailY = this->tails[jsy] + schedule->taskInfo[jsy].task->p;
		else if (msz != -1)
			tailY = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailY = FuzzyFW::Interval(0, 0);

		// New tail for task X
		if (jsx != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				tailY + schedule->taskInfo[y].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailX = tailY + schedule->taskInfo[y].task->p;

		// New tail for task Z
		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailX + schedule->taskInfo[x].task->p;

		// New head for task Z
		if (mpx != -1 && jpz != -1)
			headZ = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headZ = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpz != -1)
			headZ = schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p;
		else headZ = FuzzyFW::Interval(0, 0);

		// New head for task X
		if (jpx != -1)
			headX = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headZ + schedule->taskInfo[z].task->p;

		// New head for task Y
		if (jpy != -1)
			headY = maximum(headX + schedule->taskInfo[x].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headY = headX + schedule->taskInfo[x].task->p;

		makespan = maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX, headY + schedule->taskInfo[y].task->p + tailY, FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ, FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}
	if (arc->tipo == 3) {
		//3 abc to ->c->b->a-> 
	    // xyz to ->z->y->x-> 
		unsigned int x = arc->x;
		unsigned int y = arc->y;
		unsigned int z = arc->z;
		int jsx, jsy, jsz;

		int mac = schedule->taskInfo[x].task->machine;
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

		// New tail for task X
		if (jsx != -1 && msz != -1)
			tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
				this->tails[msz] + schedule->taskInfo[msz].task->p, FuzzyFW::Interval::M_COMPONENT);
		else if (jsx != -1)
			tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
		else if (msz != -1)
			tailX = this->tails[msz] + schedule->taskInfo[msz].task->p;
		else
			tailX = FuzzyFW::Interval(0, 0);

		// New tail for task Y
		if (jsy != -1)
			tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
				tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailY = tailX + schedule->taskInfo[x].task->p;

		// New tail for task Z
		if (jsz != -1)
			tailZ = maximum(this->tails[jsz] + schedule->taskInfo[jsz].task->p,
				tailY + schedule->taskInfo[y].task->p, FuzzyFW::Interval::M_COMPONENT);
		else
			tailZ = tailY + schedule->taskInfo[y].task->p;

		// New head for task Z
		if (mpx != -1 && jpz != -1)
			headZ = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
				schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else if (mpx != -1)
			headZ = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
		else if (jpz != -1)
			headZ = schedule->taskInfo[jpz].head + schedule->taskInfo[jpz].task->p;
		else headZ = FuzzyFW::Interval(0, 0);

		// New head for task Y
		if (jpy != -1)
			headY = maximum(headZ + schedule->taskInfo[z].task->p,
				schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headY = headZ + schedule->taskInfo[z].task->p;

		// New head for task X
		if (jpx != -1)
			headX = maximum(headY + schedule->taskInfo[y].task->p,
				schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
				FuzzyFW::Interval::M_COMPONENT);
		else headX = headY + schedule->taskInfo[y].task->p;

		makespan = maximum(
			maximum(headX + schedule->taskInfo[x].task->p + tailX, headY + schedule->taskInfo[y].task->p + tailY, FuzzyFW::Interval::M_COMPONENT),
			headZ + schedule->taskInfo[z].task->p + tailZ, FuzzyFW::Interval::M_COMPONENT);

		arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
		return;
	}
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN3_MakespanIJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN3_MakespanIJSP::quickSort(const int left, const int right,
	const FuzzyFW::SharedVars *svars) {

	int pivot, pos;

	if (left >= right)
		return;

	pivot = svars->rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->getEstimation(i, svars)->isBetterThan(
			this->getEstimation(right, svars))) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, svars);
	this->quickSort(pos + 1, right, svars);
}


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_ParallelN3_MakespanIJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}




//=============================================================================
//
//	Class NB_ParallelNH_MakespanIJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelNH_MakespanIJSP::NB_ParallelNH_MakespanIJSP(
	const NB_ParallelNH_MakespanIJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourIJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelNH_MakespanIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_ESTIMATOR_HEADSTAILS) == 0)
		this->estimator = Estimator::ESTIM_HEADTAILS;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelNH_MakespanIJSP::~NB_ParallelNH_MakespanIJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelNH_MakespanIJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Interval::Maximum maxType = FuzzyFW::Interval::M_COMPONENT;
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy IJSP Schedules are allowed.";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	ProblemIJSP * problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to IJSP problems";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;

	std::vector<char> visited(problem->getNumberTasks(), false);
	this->tails.resize(problem->getNumberTasks());

	// Look for tasks with no successors
	for (size_t i = 0; i < schedule->lastTaskJob.size(); i++) {
		taskIdx = schedule->lastTaskJob[i];
		mac = schedule->taskInfo[taskIdx].task->machine;
		if (taskIdx == schedule->lastTaskMachine[mac])
			taskQueue.push(taskIdx);
	}

	// Backwards propagation
	while (taskQueue.size() > 0) {
		taskIdx = taskQueue.front();
		taskQueue.pop();

		// Update tail
		ms = schedule->taskInfo[taskIdx].ms;
		js = schedule->taskInfo[taskIdx].task->js;

		if (ms != -1 && js != -1)
			this->tails[taskIdx] =
			maximum(this->tails[ms] + schedule->taskInfo[ms].task->p,
				this->tails[js] + schedule->taskInfo[js].task->p, maxType);
		else if (ms != -1)
			this->tails[taskIdx] =
			this->tails[ms] + schedule->taskInfo[ms].task->p;
		else if (js != -1)
			this->tails[js] + schedule->taskInfo[js].task->p;
		else this->tails[taskIdx] = FuzzyFW::Interval(0, 0);

		// Propagate
		mp = schedule->taskInfo[taskIdx].mp;
		jp = schedule->taskInfo[taskIdx].task->jp;

		if (mp != -1) {
			if (visited[mp] || schedule->taskInfo[mp].task->js == -1)
				taskQueue.push(mp);
			else visited[mp] = true;
		}

		if (jp != -1) {
			if (visited[jp] || schedule->taskInfo[jp].ms == -1)
				taskQueue.push(jp);
			else visited[jp] = true;
		}
	}
}



//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelNH_MakespanIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<int> blockFirst(this->schedule->getScheduledTasks());
	std::vector<char> added;

	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	for (unsigned int i = 0; i < nTasks; i++)
		blockFirst[i] = i;

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 2; comp++) {
		// Look for the tasks defining the makespan value
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp)) {
				taskQueue.push(this->schedule->lastTaskMachine[i]);
				blockFirst[this->schedule->lastTaskMachine[i]] = this->schedule->lastTaskMachine[i];
			}
		}

		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();
			task = this->schedule->taskInfo[taskId];
			//Si el predecesor en el camino cr�tico pertenece al mismo bloque (misma maquina) introducimos la tarea en la cola
			if (task.mp != -1) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					taskQueue.push(task.mp);
					blockFirst[task.mp] = blockFirst[taskId];
				}
			}
			//Si el predecesor en el camino critico pertenece a otro bloque 
			if (task.task->jp != -1 && task.task->jp != task.mp) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					taskQueue.push(task.task->jp);
					blockFirst[task.task->jp] = task.task->jp;
					//Por que se guarda una lista de las tareas a�adidas, no sobraria esto?
					//Para que sirve numNeighbours, estadisticas??
					if (!added[taskId] && taskId!= blockFirst[taskId]) {
						if (this->numNeighbours < this->neighbours.size()
							&& this->neighbours[this->numNeighbours] != NULL)
							this->neighbours[this->numNeighbours]->setValues(taskId,blockFirst[taskId]);
						else
							this->neighbours.push_back(new NeighbourIJSP_Arc(taskId,blockFirst[taskId]));
						this->numNeighbours++;
						added[taskId] = true;

					}
				}
			}
			//Si estamos en la primera tarea
			if (task.mp == -1 && task.task->jp == -1) {
				if (!added[taskId] && taskId != blockFirst[taskId]) {
					if (this->numNeighbours < this->neighbours.size()
						&& this->neighbours[this->numNeighbours] != NULL)
						this->neighbours[this->numNeighbours]->setValues(taskId, blockFirst[taskId]);
					else
						this->neighbours.push_back(new NeighbourIJSP_Arc(taskId, blockFirst[taskId]));
					this->numNeighbours++;
					added[taskId] = true;

				}
			}			
		}
	}
	return this->numNeighbours;
}



//-----  Fully evaluate the neighbour  ----------------------------------------
FuzzyFW::Fitness * NB_ParallelNH_MakespanIJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	FuzzyFW::FitnessInterval *lowerBound;
	FuzzyFW::Interval currentMakespan, newMakespan;
	ScheduleIJSP *newSolution;
	int job, mac;
	int jsx, jsy, jpx, jpy, mpx, msy;
	int msx, mpy;
	int z, mpz, jpz, msz, jsz;
	FuzzyFW::Interval newHead, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleIJSP(*this->schedule);
	newMakespan = FuzzyFW::Interval(0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessInterval *>(this->currentFitness->clone());

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	mpy = newSolution->taskInfo[arc->y].mp;
	msy = newSolution->taskInfo[arc->y].ms;
	mpx = newSolution->taskInfo[arc->x].mp;
	msx = newSolution->taskInfo[arc->x].ms;
	job = newSolution->taskInfo[arc->x].task->job;
	//jpx no se usa, para que vale?
	jpx = newSolution->taskInfo[arc->x].task->jp;
	if (newSolution->lastTaskJob[job] == arc->x)
		jsx = -1;
	else
		jsx = newSolution->taskInfo[arc->x].task->js;
	job = newSolution->taskInfo[arc->y].task->job;
	//jpy no se usa
	jpy = newSolution->taskInfo[arc->y].task->jp;
	if (newSolution->lastTaskJob[job] == arc->y)
		jsy = -1;
	else
		jsy = newSolution->taskInfo[arc->y].task->js;

	   	
	// Update predecessors and successors
	if (mpx != -1) {
		newSolution->taskInfo[mpx].ms = arc->y;
	}
	newSolution->taskInfo[arc->y].mp = mpx;
	newSolution->taskInfo[arc->x].ms = msy;
	if (msy != -1) {
		newSolution->taskInfo[msy].mp = arc->x;
	}
	else {
		newSolution->lastTaskMachine[mac] = arc->x;
	}

	if (msx == arc->y) {
		newSolution->taskInfo[arc->y].ms = arc->x;
		newSolution->taskInfo[arc->x].mp = arc->y;
	}
	else {
		newSolution->taskInfo[arc->y].ms = msx;
		newSolution->taskInfo[msx].mp = arc->y;
		newSolution->taskInfo[arc->x].mp = mpy;
		newSolution->taskInfo[mpy].ms = arc->x;
	}

	// Update heads
	taskQueue.push(arc->y);
	taskQueue.push(arc->x);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		jpz = newSolution->taskInfo[z].task->jp;
		mpz = newSolution->taskInfo[z].mp;
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any predecessor is null
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
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
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
void NB_ParallelNH_MakespanIJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	// Update the current solution
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

	// Update tails for the next step
	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	taskQueue.push(arc->x);
	taskQueue.push(arc->y);

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
		else if (msz != -1) {
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		}
		else if (jsz != -1) {
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		}
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail, FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1)
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
			if (this->schedule->taskInfo[z].task->jp != -1)
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
		}
	}
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelNH_MakespanIJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelNH_MakespanIJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessInterval(FuzzyFW::Interval(Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelNH_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	//std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	//this->numNeighbours--;
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelNH_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx];
	unsigned int x = arc->x;
	unsigned int y = arc->y;

	mac = schedule->taskInfo[x].task->machine;
	mpy = schedule->taskInfo[y].mp;
	jpy = schedule->taskInfo[y].task->jp;
	msy = schedule->taskInfo[y].ms;
	msx = schedule->taskInfo[x].ms;
	mpx = schedule->taskInfo[x].mp;
	jpx = schedule->taskInfo[x].task->jp;

	if (schedule->lastTaskJob[schedule->taskInfo[x].task->job] == x)
		jsx = -1;
	else
		jsx = schedule->taskInfo[x].task->js;
	if (schedule->lastTaskJob[schedule->taskInfo[y].task->job] == y)
		jsy = -1;
	else
		jsy = schedule->taskInfo[y].task->js;

	// New tail for task X
	if (jsx != -1 && msy != -1)
		tailX = maximum(this->tails[jsx] + schedule->taskInfo[jsx].task->p,
			this->tails[msy] + schedule->taskInfo[msy].task->p, FuzzyFW::Interval::M_COMPONENT);
	else if (jsx != -1)
		tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::Interval(0, 0);

	// New tail for task Y
	if (jsy != -1)
		tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
			tailX + schedule->taskInfo[x].task->p, FuzzyFW::Interval::M_COMPONENT);
	else
		tailY = tailX + schedule->taskInfo[x].task->p;

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
			schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else if (mpx != -1)
		headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
	else headY = FuzzyFW::Interval(0, 0);

	// New head for task X
	if (jpx != -1)
		headX = maximum(headY + schedule->taskInfo[y].task->p,
			schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
			FuzzyFW::Interval::M_COMPONENT);
	else headX = headY + schedule->taskInfo[y].task->p;

	makespan = maximum(headX + schedule->taskInfo[x].task->p + tailX,
		headY + schedule->taskInfo[y].task->p + tailY,
		FuzzyFW::Interval::M_COMPONENT);

	arc->setEstimatedQuality(new FuzzyFW::FitnessInterval(makespan, false));
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelNH_MakespanIJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelNH_MakespanIJSP::quickSort(const int left, const int right,
	const FuzzyFW::SharedVars *svars) {

	int pivot, pos;

	if (left >= right)
		return;

	pivot = svars->rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->getEstimation(i, svars)->isBetterThan(
			this->getEstimation(right, svars))) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, svars);
	this->quickSort(pos + 1, right, svars);
}


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_ParallelNH_MakespanIJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new IJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}
}
