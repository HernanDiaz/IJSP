/*
* Neighbourhood.cpp
*
*  Created on: Oct 12, 2017
*/

#include "NeighbourhoodFJSP_Cmax.h"

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
NB_ParallelN1_MakespanFJSP::NB_ParallelN1_MakespanFJSP(
	const NB_ParallelN1_MakespanFJSP & source)
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
void NB_ParallelN1_MakespanFJSP::setup(FuzzyFW::ParameterDB *parameters) {
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
		throw FJSPException("Neighbourhood", errorMsg);
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
//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelN1_MakespanFJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;
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
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessTFN *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	ProblemFJSP * problem = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to FJSP problems";
		throw FJSPException("Neighbourhood", errorMsg);
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
		else this->tails[taskIdx] = FuzzyFW::TFN(0, 0, 0);

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
unsigned int NB_ParallelN1_MakespanFJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::TFN currentMakespan;
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
FuzzyFW::Fitness * NB_ParallelN1_MakespanFJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	FuzzyFW::FitnessTFN *lowerBound;
	FuzzyFW::TFN currentMakespan, newMakespan;
	ScheduleFJSP *newSolution;
	int job, mac;
	int jsx, jsy, jpx, jpy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	FuzzyFW::TFN newHead, lower;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	if (arc->x < 0 || arc->y < 0)
		return NULL;

	currentMakespan = this->currentFitness->getValue();
	newSolution = new ScheduleFJSP(*this->schedule);
	newMakespan = FuzzyFW::TFN(0, 0, 0);
	lowerBound = dynamic_cast<FuzzyFW::FitnessTFN *>(this->currentFitness->clone());

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
			FuzzyFW::TFN::M_COMPONENT);
	}

	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessTFN(newMakespan, false));

	delete lowerBound;
	return this->neighbours[idx]->getEvaluatedFitness();
}



//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN1_MakespanFJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	int job;
	int z, msz, jsz;
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
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessTFN *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());

	// Update tails for the next step
	NeighbourFJSP_Arc *arc = this->neighbours[idx];
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

		// Check if any successor is null
		if (jsz != -1 && msz != -1)
			newTail = maximum(this->schedule->taskInfo[msz].task->p + this->tails[msz],
				this->schedule->taskInfo[jsz].task->p + this->tails[jsz],
				FuzzyFW::TFN::M_COMPONENT);
		else if (jsz == -1)
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		else
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];

		if (!(this->tails[z].isEqualTo(newTail,	FuzzyFW::TFN::C_COMPONENT))) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1)
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
			if (this->schedule->taskInfo[z].task->jp != -1)
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
		}
	}
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_MakespanFJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelN1_MakespanFJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessTFN(FuzzyFW::TFN(Infd, Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_MakespanFJSP::discardNeighbour(const unsigned int idx) {
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
void NB_ParallelN1_MakespanFJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::TFN tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::TFN makespan;

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
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
			this->tails[msy] + schedule->taskInfo[msy].task->p,	FuzzyFW::TFN::M_COMPONENT);
	else if (jsx != -1)
		tailX = this->tails[jsx] + schedule->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = this->tails[msy] + schedule->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::TFN(0, 0, 0);

	// New tail for task Y
	if (jsy != -1)
		tailY = maximum(this->tails[jsy] + schedule->taskInfo[jsy].task->p,
			tailX + schedule->taskInfo[x].task->p, FuzzyFW::TFN::M_COMPONENT);
	else
		tailY = tailX + schedule->taskInfo[x].task->p;

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
			schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p,
			FuzzyFW::TFN::M_COMPONENT);
	else if (mpx != -1)
		headY = schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p;
	else headY = FuzzyFW::TFN(0, 0, 0);

	// New head for task X
	if (jpx != -1)
		headX = maximum(headY + schedule->taskInfo[y].task->p,
			schedule->taskInfo[jpx].head + schedule->taskInfo[jpx].task->p,
			FuzzyFW::TFN::M_COMPONENT);
	else headX = headY + schedule->taskInfo[y].task->p;

	makespan = maximum(headX + schedule->taskInfo[x].task->p + tailX,
		headY + schedule->taskInfo[y].task->p + tailY,
		FuzzyFW::TFN::M_COMPONENT);

	arc->setEstimatedQuality(new FuzzyFW::FitnessTFN(makespan, false));
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_MakespanFJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_MakespanFJSP::quickSort(const int left, const int right,
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
FuzzyFW::Neighbour* NB_ParallelN1_MakespanFJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw FJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}

}
