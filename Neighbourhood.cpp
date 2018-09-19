/*
* Neighbourhood.cpp
*
*  Created on: Oct 12, 2017
*      Author: jjpalacios
*/

#include "Neighbourhood.h"

namespace FJSP {

//=============================================================================
//
//	Class NB_ParallelN1_MakespanFJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN1_MakespanFJSP::NB_ParallelN1_MakespanFJSP(const NB_ParallelN1_MakespanFJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourFJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN1_MakespanFJSP::setup(ParameterDB *parameters) {
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
		throw new FJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN1_MakespanFJSP::~NB_ParallelN1_MakespanFJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_MakespanFJSP::findNeighbours(Individual *solution,
	const SharedVars *svars) {

	unsigned int taskId, nTasks;
	TFN currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;

	// Save the current solution
	this->solution = solution;

	// Convert the schedule
	if (!solution->isEvaluated() || !solution->isPhenotypeUpdated()) {
		std::string errorMsg = "The individual must be evaluated before ";
		errorMsg = "applying any type of search";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->schedule = dynamic_cast<FuzzySchedule *>(solution->getPhenotype());
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy JSP Schedules are allowed.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->currentFitness = dynamic_cast<FitnessTFN *>(solution->getFitness());
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	currentMakespan = currentFitness->getValue();

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	this->schedule->updateTails(TFN::M_COMPONENT);
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
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
						if (this->numNeighbours < this->neighbours.size())
							this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
						else
							this->neighbours.push_back(new NeighbourFJSP_Arc(task.mp, taskId));
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
Individual * NB_ParallelN1_MakespanFJSP::evaluateNeighbour(
	const unsigned int idx, const SharedVars *svars, const bool improvement) {

	FitnessTFN *lowerBound;
	TFN currentMakespan, newMakespan;
	FuzzySchedule *newSolution;
	int job, mac;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int z, mpz, jpz, msz, jsz;
	TFN newHead, newTail, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg  = "Trying to access a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new FuzzySchedule(*this->schedule);
	newMakespan = TFN(0, 0, 0);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	mpy = newSolution->taskInfo[arc->y].mp;
	msy = newSolution->taskInfo[arc->y].ms;
	msx = newSolution->taskInfo[arc->x].ms;
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

	// Calculate the new heads and tails
	if(this->estimator != Estimator::ESTIM_HEADTAILS || !arc->hasEstimation())
		arc->updateHeadsTails(newSolution);

	// Update predecessors and successors
	if (mpx != -1)
		newSolution->taskInfo[mpx].ms = arc->y;
	newSolution->taskInfo[arc->y].mp = mpx;
	newSolution->taskInfo[arc->y].ms = arc->x;
	newSolution->taskInfo[arc->x].mp = arc->y;
	newSolution->taskInfo[arc->x].ms = msy;
	if (msy != -1)
		newSolution->taskInfo[msy].mp = arc->x;

	// Update head and tails values
	newSolution->taskInfo[arc->x].head = arc->newHeadX;
	newSolution->taskInfo[arc->y].head = arc->newHeadY;
	newSolution->taskInfo[arc->x].tail = arc->newTailX;
	newSolution->taskInfo[arc->y].tail = arc->newTailY;

	lowerBound = NULL;
	if (improvement) {
		lowerBound = new FitnessTFN(newSolution->taskInfo[arc->x].head
			+ newSolution->taskInfo[arc->x].task->p
			+ newSolution->taskInfo[arc->x].tail, false);
		if (lowerBound->isWorseThan(currentFitness)) {
			delete newSolution;
			delete lowerBound;
			return NULL;
		}
		lowerBound->setValue(newSolution->taskInfo[arc->y].head
			+ newSolution->taskInfo[arc->y].task->p
			+ newSolution->taskInfo[arc->y].tail);
		if (lowerBound->isWorseThan(currentFitness)) {
			delete newSolution;
			delete lowerBound;
			return NULL;
		}
	}

	// Propagation of heads
	if (jsx != -1) taskQueue.push(jsx);
	if (jsy != -1) taskQueue.push(jsy);
	if (msy != -1) taskQueue.push(msy);
	else
		newSolution->lastTaskMachine[mac] = arc->x;

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
				TFN::M_COMPONENT);
		else if (jpz == -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else 
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead, TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;

			if (improvement) {
				lowerBound->setValue(newSolution->taskInfo[z].head
					+ newSolution->taskInfo[z].task->p
					+ newSolution->taskInfo[z].tail);
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

	// Propagation of tails
	if (jpx != -1) taskQueue.push(jpx);
	if (jpy != -1) taskQueue.push(jpy);
	if (mpx != -1) taskQueue.push(mpx);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any successor is null
		if (jsz != -1 && msz != -1)
			newTail = maximum(newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail,
				newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail,
				TFN::M_COMPONENT);
		else if (jsz == -1) 
			newTail = newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail;
		else 
			newTail = newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail;

		if (!(newSolution->taskInfo[z].tail.isEqualTo(newTail, TFN::C_COMPONENT))) {
			newSolution->taskInfo[z].tail = newTail;
			if (newSolution->taskInfo[z].mp != -1) 
				taskQueue.push(newSolution->taskInfo[z].mp); // machine
			if (newSolution->taskInfo[z].task->jp != -1) 
				taskQueue.push(newSolution->taskInfo[z].task->jp); // job
		}
	}

	Individual *newIndiv = this->solution->clone();
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++) {
		newMakespan = maximum(newMakespan, newSolution->getCTJob(i), TFN::M_COMPONENT);
	}
	newSolution->setSorted(false);
	newIndiv->updatePhenotype(newSolution);
	newIndiv->updateFitness(new FitnessTFN(newMakespan, false));

	if (improvement &&
		newIndiv->getFitness()->isWorseOrEqualTo(currentFitness)) {
		delete newIndiv;
		return NULL;
	}

	delete lowerBound;
	return newIndiv;
}




//-----  Get the estimation  --------------------------------------------------
Fitness * NB_ParallelN1_MakespanFJSP::getEstimation(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->hasEstimation()) {
		if (this->estimator == NB_ParallelN1_MakespanFJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(new FitnessTFN(TFN(Infd, Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_MakespanFJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours-1]);
	this->numNeighbours--;
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_MakespanFJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	arc->updateHeadsTails(this->schedule);
	TFN makespan = maximum(arc->newHeadX + this->schedule->taskInfo[arc->x].task->p + arc->newTailX,
		arc->newHeadY + this->schedule->taskInfo[arc->y].task->p + arc->newTailY,
		TFN::M_COMPONENT);
	arc->setEstimatedQuality(new FitnessTFN(makespan, false));
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_MakespanFJSP::sortByEstimation(const SharedVars *svars) {
	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours-1, svars->rng);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_MakespanFJSP::quickSort(const int left, const int right, Random *rng) {
	int pivot, pos;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if(this->neighbours[i]->getEstimatedQuality()->isBetterThan(
			this->neighbours[right]->getEstimatedQuality())) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, rng);
	this->quickSort(pos + 1, right, rng);
}





//=============================================================================
//
//	Class NB_ParallelN1_AIavgFJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN1_AIavgFJSP::NB_ParallelN1_AIavgFJSP(const NB_ParallelN1_AIavgFJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourFJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN1_AIavgFJSP::setup(ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_NONE) == 0)
		this->estimator = Estimator::NONE;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN1_AIavgFJSP::~NB_ParallelN1_AIavgFJSP() {
	for (size_t i = 0; i < neighbours.size(); i++) {
		if (neighbours[i] != NULL)
			delete neighbours[i];
	}
	neighbours.clear();
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_AIavgFJSP::findNeighbours(Individual *solution,
	const SharedVars *svars) {

	unsigned int taskId, nTasks;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;
	FuzzyProblem * fuzzyProb;

	// Save the current solution
	this->solution = solution;

	// Convert the schedule
	if (!solution->isEvaluated() || !solution->isPhenotypeUpdated()) {
		std::string errorMsg = "The individual must be evaluated before ";
		errorMsg = "applying any type of search";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->schedule = dynamic_cast<FuzzySchedule *>(solution->getPhenotype());
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy JSP Schedules are allowed.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->currentFitness = dynamic_cast<FitnessDouble *>(solution->getFitness());
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	fuzzyProb =	dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	this->schedule->updateTails(TFN::M_COMPONENT);
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the AI is lesser than 1
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			if(fuzzyProb->getDueDate(job)->agreementIndex(
				this->schedule->getCTJob(job)) < 1)
				taskQueue.push(this->schedule->lastTaskJob[job]);
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
						if (this->numNeighbours < this->neighbours.size())
							this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
						else
							this->neighbours.push_back(new NeighbourFJSP_Arc(task.mp, taskId));
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
Individual * NB_ParallelN1_AIavgFJSP::evaluateNeighbour(
	const unsigned int idx, const SharedVars *svars, const bool improvement) {

	FuzzySchedule *newSolution;
	int job, mac;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int z, mpz, jpz, msz, jsz;
	TFN newHead, newTail, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	FuzzyProblem *fuzzyProb = dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	newSolution = new FuzzySchedule(*this->schedule);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	mpy = newSolution->taskInfo[arc->y].mp;
	msy = newSolution->taskInfo[arc->y].ms;
	msx = newSolution->taskInfo[arc->x].ms;
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

	// Calculate the new heads and tails
	if (//this->estimator != Estimator::ESTIM_HEADTAILS ||
		!arc->hasEstimation())
		arc->updateHeadsTails(newSolution);

	// Update predecessors and successors
	if (mpx != -1)
		newSolution->taskInfo[mpx].ms = arc->y;
	newSolution->taskInfo[arc->y].mp = mpx;
	newSolution->taskInfo[arc->y].ms = arc->x;
	newSolution->taskInfo[arc->x].mp = arc->y;
	newSolution->taskInfo[arc->x].ms = msy;
	if (msy != -1)
		newSolution->taskInfo[msy].mp = arc->x;

	// Update head and tails values
	newSolution->taskInfo[arc->x].head = arc->newHeadX;
	newSolution->taskInfo[arc->y].head = arc->newHeadY;
	newSolution->taskInfo[arc->x].tail = arc->newTailX;
	newSolution->taskInfo[arc->y].tail = arc->newTailY;

	// Propagation of heads
	if (jsx != -1) taskQueue.push(jsx);
	if (jsy != -1) taskQueue.push(jsy);
	if (msy != -1) taskQueue.push(msy);
	else
		newSolution->lastTaskMachine[mac] = arc->x;

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
				TFN::M_COMPONENT);
		else if (jpz == -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead, TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	// Propagation of tails
	if (jpx != -1) taskQueue.push(jpx);
	if (jpy != -1) taskQueue.push(jpy);
	if (mpx != -1) taskQueue.push(mpx);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any successor is null
		if (jsz != -1 && msz != -1)
			newTail = maximum(newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail,
				newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail,
				TFN::M_COMPONENT);
		else if (jsz == -1)
			newTail = newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail;
		else
			newTail = newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail;

		if (!(newSolution->taskInfo[z].tail.isEqualTo(newTail, TFN::C_COMPONENT))) {
			newSolution->taskInfo[z].tail = newTail;
			if (newSolution->taskInfo[z].mp != -1)
				taskQueue.push(newSolution->taskInfo[z].mp); // machine
			if (newSolution->taskInfo[z].task->jp != -1)
				taskQueue.push(newSolution->taskInfo[z].task->jp); // job
		}
	}

	Individual *newIndiv = this->solution->clone();
	double sumAI = 0.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++)
		sumAI += fuzzyProb->getDueDate(i)->agreementIndex(newSolution->getCTJob(i));

	newSolution->setSorted(false);
	newIndiv->updatePhenotype(newSolution);
	newIndiv->updateFitness(new FitnessDouble(
		sumAI / newSolution->lastTaskJob.size(), true));

	if (improvement &&
		newIndiv->getFitness()->isWorseOrEqualTo(currentFitness)) {
		delete newIndiv;
		return NULL;
	}

	return newIndiv;
}




//-----  Get the estimation  --------------------------------------------------
Fitness * NB_ParallelN1_AIavgFJSP::getEstimation(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->hasEstimation()) {
		this->neighbours[idx]->setEstimatedQuality(new FitnessDouble(-1, true));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_AIavgFJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	this->numNeighbours--;
}


//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_AIavgFJSP::sortByEstimation(const SharedVars *svars) {
	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars->rng);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_AIavgFJSP::quickSort(const int left, const int right, Random *rng) {
	int pivot, pos;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);

	std::swap(this->neighbours[pivot], this->neighbours[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if (this->neighbours[i]->getEstimatedQuality()->isBetterThan(
			this->neighbours[right]->getEstimatedQuality())) {
			std::swap(this->neighbours[i], this->neighbours[pos]);
			pos++;
		}
	}

	std::swap(neighbours[pos], neighbours[right]);
	this->quickSort(left, pos - 1, rng);
	this->quickSort(pos + 1, right, rng);
}





//=============================================================================
//
//	Class NB_ParallelN1_AIminFJSP
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_AIminFJSP::findNeighbours(Individual *solution,
	const SharedVars *svars) {

	unsigned int taskId, nTasks;
	double jobAI, currentAImin;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;
	FuzzyProblem * fuzzyProb;

	// Save the current solution
	this->solution = solution;

	// Convert the schedule
	if (!solution->isEvaluated() || !solution->isPhenotypeUpdated()) {
		std::string errorMsg = "The individual must be evaluated before ";
		errorMsg = "applying any type of search";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->schedule = dynamic_cast<FuzzySchedule *>(solution->getPhenotype());
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy JSP Schedules are allowed.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	this->currentFitness = dynamic_cast<FitnessDouble *>(solution->getFitness());
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	currentAImin = this->currentFitness->getValue();

	// Get the problem in its true form
	fuzzyProb = dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	this->schedule->updateTails(TFN::M_COMPONENT);
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the AI is 
		// equal to the current AImin
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			jobAI = fuzzyProb->getDueDate(job)->agreementIndex(
				this->schedule->getCTJob(job));
			if (compareDouble(jobAI, currentAImin) <= 0)
				taskQueue.push(this->schedule->lastTaskJob[job]);
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
						if (this->numNeighbours < this->neighbours.size())
							this->neighbours[this->numNeighbours]->setValues(task.mp, taskId);
						else
							this->neighbours.push_back(new NeighbourFJSP_Arc(task.mp, taskId));
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
Individual * NB_ParallelN1_AIminFJSP::evaluateNeighbour(
	const unsigned int idx, const SharedVars *svars, const bool improvement) {

	FuzzySchedule *newSolution;
	double jobAI;
	int job, mac;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int z, mpz, jpz, msz, jsz;
	TFN newHead, newTail, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	FuzzyProblem *fuzzyProb = dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	newSolution = new FuzzySchedule(*this->schedule);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	mpy = newSolution->taskInfo[arc->y].mp;
	msy = newSolution->taskInfo[arc->y].ms;
	msx = newSolution->taskInfo[arc->x].ms;
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

	// Calculate the new heads and tails
	if (//this->estimator != Estimator::ESTIM_HEADTAILS ||
		!arc->hasEstimation())
		arc->updateHeadsTails(newSolution);

	// Update predecessors and successors
	if (mpx != -1)
		newSolution->taskInfo[mpx].ms = arc->y;
	newSolution->taskInfo[arc->y].mp = mpx;
	newSolution->taskInfo[arc->y].ms = arc->x;
	newSolution->taskInfo[arc->x].mp = arc->y;
	newSolution->taskInfo[arc->x].ms = msy;
	if (msy != -1)
		newSolution->taskInfo[msy].mp = arc->x;

	// Update head and tails values
	newSolution->taskInfo[arc->x].head = arc->newHeadX;
	newSolution->taskInfo[arc->y].head = arc->newHeadY;
	newSolution->taskInfo[arc->x].tail = arc->newTailX;
	newSolution->taskInfo[arc->y].tail = arc->newTailY;

	// Propagation of heads
	if (jsx != -1) taskQueue.push(jsx);
	if (jsy != -1) taskQueue.push(jsy);
	if (msy != -1) taskQueue.push(msy);
	else
		newSolution->lastTaskMachine[mac] = arc->x;

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
				TFN::M_COMPONENT);
		else if (jpz == -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead, TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	// Propagation of tails
	if (jpx != -1) taskQueue.push(jpx);
	if (jpy != -1) taskQueue.push(jpy);
	if (mpx != -1) taskQueue.push(mpx);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = newSolution->taskInfo[z].ms;
		job = newSolution->taskInfo[z].task->job;
		if (newSolution->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = newSolution->taskInfo[z].task->js;

		// Check if any successor is null
		if (jsz != -1 && msz != -1)
			newTail = maximum(newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail,
				newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail,
				TFN::M_COMPONENT);
		else if (jsz == -1)
			newTail = newSolution->taskInfo[msz].task->p + newSolution->taskInfo[msz].tail;
		else
			newTail = newSolution->taskInfo[jsz].task->p + newSolution->taskInfo[jsz].tail;

		if (!(newSolution->taskInfo[z].tail.isEqualTo(newTail, TFN::C_COMPONENT))) {
			newSolution->taskInfo[z].tail = newTail;
			if (newSolution->taskInfo[z].mp != -1)
				taskQueue.push(newSolution->taskInfo[z].mp); // machine
			if (newSolution->taskInfo[z].task->jp != -1)
				taskQueue.push(newSolution->taskInfo[z].task->jp); // job
		}
	}

	Individual *newIndiv = this->solution->clone();
	double minAI = 2.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++) {
		jobAI = fuzzyProb->getDueDate(i)->agreementIndex(newSolution->getCTJob(i));
		if (jobAI < minAI)
			minAI = jobAI;
	}

	newSolution->setSorted(false);
	newIndiv->updatePhenotype(newSolution);
	newIndiv->updateFitness(new FitnessDouble(minAI, true));

	if (improvement &&
		newIndiv->getFitness()->isWorseOrEqualTo(currentFitness)) {
		delete newIndiv;
		return NULL;
	}

	return newIndiv;
}

}

