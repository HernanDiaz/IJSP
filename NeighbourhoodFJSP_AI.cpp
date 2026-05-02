/*
* Neighbourhood.cpp
*
*  Created on: Oct 12, 2017
*/

#include "NeighbourhoodFJSP_AI.h"

namespace FJSP {

//=============================================================================
//
//	Class NB_ParallelN1_AIavgFJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN1_AIavgFJSP::NB_ParallelN1_AIavgFJSP(
	const NB_ParallelN1_AIavgFJSP & source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != NULL)
			neighbours.push_back(new NeighbourFJSP_Arc(*source.neighbours[i]));
		else
			neighbours.push_back(NULL);
	}
}



//-----  Setup method  --------------------------------------------------------
void NB_ParallelN1_AIavgFJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_AI_NONE) == 0)
		this->estimator = Estimator::NONE;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw FJSPException("Neighbourhood", errorMsg);
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
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelN1_AIavgFJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy JSP Schedules are allowed.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = fitness;
}



//-----  Find enighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_AIavgFJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;
	ProblemFJSP * fuzzyProb;

	// Get the problem in its true form
	fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the AI is lesser than 1
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			if (fuzzyProb->getTimeWindow(job)->agreementIndex(
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
						if (this->numNeighbours < this->neighbours.size()
							&& this->neighbours[this->numNeighbours] != NULL)
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
FuzzyFW::Fitness * NB_ParallelN1_AIavgFJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {


	ScheduleFJSP *newSolution;
	int job, mac;
	int jsx, jsy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	FuzzyFW::TFN newHead;
	std::queue<int> taskQueue;
	
	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	ProblemFJSP *fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	newSolution = new ScheduleFJSP(*this->schedule);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	msy = newSolution->taskInfo[arc->y].ms;
	mpx = newSolution->taskInfo[arc->x].mp;
	job = newSolution->taskInfo[arc->x].task->job;
	if (newSolution->lastTaskJob[job] == arc->x)
		jsx = -1;
	else
		jsx = newSolution->taskInfo[arc->x].task->js;
	job = newSolution->taskInfo[arc->y].task->job;
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
				FuzzyFW::TFN::M_COMPONENT);
		else if (mpz != -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::TFN(0, 0, 0);

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	double sumAI = 0.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++)
		sumAI += fuzzyProb->getTimeWindow(i)->agreementIndex(newSolution->getCTJob(i));


	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessDouble(
			sumAI / newSolution->lastTaskJob.size(), true));

	return this->neighbours[idx]->getEvaluatedFitness();
}



//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN1_AIavgFJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	FuzzyFW::TFN newTail;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	// Update the current solution
	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleFJSP *>
		(this->neighbours[idx]->getEvaluation()->clone());
	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = this->neighbours[idx]->getEvaluatedFitness()->clone();
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_AIavgFJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		this->neighbours[idx]->setEstimatedQuality(
			new FuzzyFW::FitnessDouble(-1, true));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_AIavgFJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
	//std::swap(this->neighbours[idx], this->neighbours[this->numNeighbours - 1]);
	//this->numNeighbours--;
}


//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_AIavgFJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars->rng);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_AIavgFJSP::quickSort(const int left, const int right,
	FuzzyFW::Random *rng) {

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


//-----  Gets an specific neighbour  ------------------------------------------
FuzzyFW::Neighbour* NB_ParallelN1_AIavgFJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
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
unsigned int NB_ParallelN1_AIminFJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;
	ProblemFJSP * fuzzyProb;
	double jobAI, currentAImin;

	// Get the problem in its true form
	fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	if (nTasks < 2)
		return 1;

	currentAImin = this->currentFitness->toDouble();
	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the AI is 
		// equal to the current AImin
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			jobAI = fuzzyProb->getTimeWindow(job)->agreementIndex(
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
						if (this->numNeighbours < this->neighbours.size()
							&& this->neighbours[this->numNeighbours] != NULL)
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
FuzzyFW::Fitness * NB_ParallelN1_AIminFJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {


	ScheduleFJSP *newSolution;
	int job, mac;
	int jsx, jsy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	double jobAI;
	FuzzyFW::TFN newHead;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	ProblemFJSP *fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	newSolution = new ScheduleFJSP(*this->schedule);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	msy = newSolution->taskInfo[arc->y].ms;
	mpx = newSolution->taskInfo[arc->x].mp;
	job = newSolution->taskInfo[arc->x].task->job;
	if (newSolution->lastTaskJob[job] == arc->x)
		jsx = -1;
	else
		jsx = newSolution->taskInfo[arc->x].task->js;
	job = newSolution->taskInfo[arc->y].task->job;
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
				FuzzyFW::TFN::M_COMPONENT);
		else if (mpz != -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::TFN(0, 0, 0);

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	double minAI = 2.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++) {
		jobAI = fuzzyProb->getTimeWindow(i)->agreementIndex(newSolution->getCTJob(i));
		if (jobAI < minAI)
			minAI = jobAI;
	}

	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessDouble(minAI, true));

	return this->neighbours[idx]->getEvaluatedFitness();
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_AIminFJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		this->neighbours[idx]->setEstimatedQuality(
			new FuzzyFW::FitnessDouble(-1, true));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}





//=============================================================================
//
//	Class NB_ParallelN1_AIminICAE
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_AIminICAE::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::FitnessLexicographic *fitness, *fl;
	fl = dynamic_cast<FuzzyFW::FitnessLexicographic *>(this->currentFitness);
	if (fl == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "lexicographic fitness types.";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	
	if (!this->neighbours[idx]->isEstimated()) {
		fitness = new FuzzyFW::FitnessLexicographic();
		for (unsigned int i = 0; i < fl->getNumberFunctions(); i++)
			fitness->addFitness(new FuzzyFW::FitnessDouble(-1, true));
		this->neighbours[idx]->setEstimatedQuality(fitness);
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_AIminICAE::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	ScheduledTaskInfo task, mp, jp;
	FuzzyFW::FitnessLexicographic *fl;
	std::queue<int> taskQueue;
	std::vector<char> added;
	ProblemFJSP * fuzzyProb;
	double jobAI, currentAImin;

	// Get the problem in its true form
	fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	if (nTasks < 2)
		return 1;

	fl = dynamic_cast<FuzzyFW::FitnessLexicographic *>(this->currentFitness);
	if (fl == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "lexicographic fitness types.";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	currentAImin = fl->getFitness(0)->toDouble();
	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the AI is 
		// equal to the current AImin
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			jobAI = fuzzyProb->getTimeWindow(job)->agreementIndex(
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
						if (this->numNeighbours < this->neighbours.size()
							&& this->neighbours[this->numNeighbours] != NULL)
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
FuzzyFW::Fitness * NB_ParallelN1_AIminICAE::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	FuzzyFW::FitnessLexicographic *fitness;
	std::list<double> AIlist;
	std::list<double>::iterator listIter;

	ScheduleFJSP *newSolution;
	int job, mac;
	int jsx, jsy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	double jobAI;
	FuzzyFW::TFN newHead;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	ProblemFJSP *fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	newSolution = new ScheduleFJSP(*this->schedule);

	// Initialize variables
	mac = newSolution->taskInfo[arc->x].task->machine;
	msy = newSolution->taskInfo[arc->y].ms;
	mpx = newSolution->taskInfo[arc->x].mp;
	job = newSolution->taskInfo[arc->x].task->job;
	if (newSolution->lastTaskJob[job] == arc->x)
		jsx = -1;
	else
		jsx = newSolution->taskInfo[arc->x].task->js;
	job = newSolution->taskInfo[arc->y].task->job;
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
				FuzzyFW::TFN::M_COMPONENT);
		else if (mpz != -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else if (jpz != -1)
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;
		else
			newHead = FuzzyFW::TFN(0, 0, 0);

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	// Compute the list of AI values
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		jobAI = fuzzyProb->getTimeWindow(i)->agreementIndex(newSolution->getCTJob(i));
		// Find the insertion position inside the list
		listIter = AIlist.begin();
		while (listIter != AIlist.end() && jobAI > *listIter)
			listIter++;
		AIlist.insert(listIter, jobAI);
	}
	
	newSolution->setSorted(false);

	// Create the lexicographical fitness function
	fitness = new FuzzyFW::FitnessLexicographic();
	for (listIter = AIlist.begin(); listIter != AIlist.end(); listIter++)
		fitness->addFitness(new FuzzyFW::FitnessDouble(*listIter, true));

	this->neighbours[idx]->setEvaluation(newSolution, fitness);

	return this->neighbours[idx]->getEvaluatedFitness();
}

}
