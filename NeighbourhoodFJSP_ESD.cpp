/*
* Neighbourhood_ESD.cpp
*
*  Created on: Oct 12, 2017
*      Author: jjpalacios
*/

#include "NeighbourhoodFJSP_ESD.h"

namespace FJSP {
//=============================================================================
//
//	Class NB_ParallelN1_ESDavgFJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NB_ParallelN1_ESDavgFJSP::NB_ParallelN1_ESDavgFJSP(
	const NB_ParallelN1_ESDavgFJSP & source)
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
void NB_ParallelN1_ESDavgFJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Neighbourhood::setup(parameters);

	std::string estimatorValue =
		parameters->getStringLower(this->estimatorLabel);

	if (estimatorValue.length() < 1 ||
		estimatorValue.compare(NB_ESTIMATOR_ESD_NONE) == 0)
		this->estimator = Estimator::NONE;
	else if (estimatorValue.compare(NB_ESTIMATOR_ESD_HEADSTAILS) == 0)
		this->estimator = Estimator::ESTIM_HEADTAILS;
	else {
		std::string errorMsg = "Estimation method unknown: \'";
		errorMsg += estimatorValue + "\'";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelN1_ESDavgFJSP::~NB_ParallelN1_ESDavgFJSP() {
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
void NB_ParallelN1_ESDavgFJSP::setInitialSolution(FuzzyFW::Solution *solution,
	FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars) {

	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy JSP Schedules are allowed.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness =
		dynamic_cast<FuzzyFW::FitnessDouble *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "ESDavg";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	ProblemFJSP * problem = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to FJSP problems";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;
	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;

	std::vector<char> visited(problem->getNumberTasks(), false);
	this->tails.clear();
	this->tails.resize(problem->getNumberTasks());

	for (unsigned int i = 0; i < problem->getNumberTasks(); i++) {
		this->tails[i].resize(problem->getNumberJobs(), FuzzyFW::TFN(-1,-1,-1));
	}

	// Look for the last task of each job
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

		// Intermiedate node
		for (unsigned int job = 0; job < problem->getNumberJobs(); job++) {
			if (ms != -1 && js != -1) {
					this->tails[taskIdx][job] =
					maximum(this->tails[ms][job] + schedule->taskInfo[ms].task->p,
						this->tails[js][job] + schedule->taskInfo[js].task->p, maxType);
			}

			// Last node of the machine, but not last of the job
			else if (js != -1) {
					this->tails[taskIdx][job] =
					this->tails[js][job] + schedule->taskInfo[js].task->p;
			}

			// Last node of the job
			else if (ms != -1) {
					if (job == schedule->taskInfo[taskIdx].task->job) {
						this->tails[taskIdx][job] =
							maximum(this->tails[ms][job] + schedule->taskInfo[ms].task->p,
								FuzzyFW::TFN(0, 0, 0), maxType);
					}
					else {
						this->tails[taskIdx][job] =
							this->tails[ms][job] + schedule->taskInfo[ms].task->p;
					}
			}

			// Last node of the job and the machine
			else {
					if (job == schedule->taskInfo[taskIdx].task->job)
						this->tails[taskIdx][job] = FuzzyFW::TFN(0, 0, 0);
					else
						this->tails[taskIdx][job] = FuzzyFW::TFN(-Infd, -Infd, -Infd);
			}
		}

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
unsigned int NB_ParallelN1_ESDavgFJSP::findNewNeighbours(
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
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the ESD is lesser than 1
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			if (fuzzyProb->getTimeWindow(job)->agreementIndex(
				this->schedule->getCTJob(job).expectedValue()) < 1)
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
FuzzyFW::Fitness * NB_ParallelN1_ESDavgFJSP::evaluateNeighbour(
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
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	ProblemFJSP *fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
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
	if (this->schedule->taskInfo[arc->y].ms != -1)
		taskQueue.push(this->schedule->taskInfo[arc->y].ms);

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
			newHead = FuzzyFW::TFN(0,0,0);

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	double sumESD = 0.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++)
		sumESD += fuzzyProb->getTimeWindow(i)->agreementIndex(
			newSolution->getCTJob(i).expectedValue());


	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessDouble(
			sumESD / newSolution->lastTaskJob.size(), true));

	return this->neighbours[idx]->getEvaluatedFitness();
}



//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN1_ESDavgFJSP::acceptNeighbour(const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {
	char tailChange;
	int taskJob;
	int z, msz, jsz;
	FuzzyFW::TFN newTail;
	std::queue<int> taskQueue;
	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;

	if (idx < 0 || idx > this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	ProblemFJSP * problem = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to FJSP problems";
		throw new FJSPException("Neighbourhood", errorMsg);
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
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessDouble *>
		(this->neighbours[idx]->getEvaluatedFitness()->clone());
	

	// Update tails
	NeighbourFJSP_Arc *arc = this->neighbours[idx];

	taskQueue.push(arc->x);
	taskQueue.push(arc->y);
	if (this->schedule->taskInfo[arc->y].mp != -1)
		taskQueue.push(this->schedule->taskInfo[arc->y].mp);

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = this->schedule->taskInfo[z].ms;
		taskJob = this->schedule->taskInfo[z].task->job;
		if (this->schedule->lastTaskJob[taskJob] == z)
			jsz = -1;
		else jsz = this->schedule->taskInfo[z].task->js;

		tailChange = false;
		for (unsigned int job = 0; job < problem->getNumberJobs(); job++) {
			// Intermiedate node
			if (msz != -1 && jsz != -1) {
				newTail = maximum(this->tails[msz][job] + schedule->taskInfo[msz].task->p,
						this->tails[jsz][job] + schedule->taskInfo[jsz].task->p, maxType);
			}
			// Last node of the machine, but not last of the job
			else if (jsz != -1) {
				newTail = this->tails[jsz][job] + schedule->taskInfo[jsz].task->p;
			}
			// Last node of the job
			else if (msz != -1) {
				if (job == taskJob) {
					newTail =
						maximum(this->tails[msz][job] + schedule->taskInfo[msz].task->p,
							FuzzyFW::TFN(0, 0, 0), maxType);
				}
				else {
					newTail = this->tails[msz][job] + schedule->taskInfo[msz].task->p;
				}
			}

			// Last node of the job and the machine
			else {
				if (job == taskJob)
					newTail = FuzzyFW::TFN(0, 0, 0);
				else
					newTail = FuzzyFW::TFN(-Infd, -Infd, -Infd);
			}
			if (!(this->tails[z][job].isEqualTo(newTail, FuzzyFW::TFN::C_COMPONENT))) {
				tailChange = true;
				this->tails[z][job] = newTail;
			}
		}

		// Propagate
		if (tailChange) {
			if (this->schedule->taskInfo[z].mp != -1)
				taskQueue.push(this->schedule->taskInfo[z].mp); // machine
			if (this->schedule->taskInfo[z].task->jp != -1)
				taskQueue.push(this->schedule->taskInfo[z].task->jp); // job
		}
	}
	//this->checkColas(svars);
}



//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness * NB_ParallelN1_ESDavgFJSP::getEstimation(
	const unsigned int idx,
	const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	ProblemFJSP * problem = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to FJSP problems";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == NB_ParallelN1_ESDavgFJSP::ESTIM_HEADTAILS)
			this->estimateHeadsTails(problem, idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessTFN(FuzzyFW::TFN(Infd, Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}



//-----  Discard a neighbour  --------------------------------------------------
void NB_ParallelN1_ESDavgFJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	delete this->neighbours[idx];
	this->neighbours[idx] = NULL;
}


//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_ESDavgFJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelN1_ESDavgFJSP::quickSort(const int left, const int right,
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
FuzzyFW::Neighbour* NB_ParallelN1_ESDavgFJSP::getNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == NULL) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx];
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_ESDavgFJSP::estimateHeadsTails(const ProblemFJSP *problem,
	const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	FuzzyFW::TFN tailX, tailY, headX, headY;
	FuzzyFW::TFN Ctime;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	unsigned int x = arc->x;
	unsigned int y = arc->y;
	double sum = 0.0;
	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;

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

	for (unsigned int job = 0; job < problem->getNumberJobs(); job++) {
		// Intermiedate node X
		if (msy != -1 && jsx != -1) {
			tailX = maximum(this->tails[msy][job] + schedule->taskInfo[msy].task->p,
				this->tails[jsx][job] + schedule->taskInfo[jsx].task->p, maxType);
		}
		// Last node of the machine, but not last of the job
		else if (jsx != -1) {
			tailX = this->tails[jsx][job] + schedule->taskInfo[jsx].task->p;
		}
		// Last node of the job
		else if (msy != -1) {
			if (job == schedule->taskInfo[arc->x].task->job) {
				tailX =
					maximum(this->tails[msy][job] + schedule->taskInfo[msy].task->p,
						FuzzyFW::TFN(0, 0, 0), maxType);
			}
			else {
				tailX = this->tails[msy][job] + schedule->taskInfo[msy].task->p;
			}
		}
		// Last node of the job and the machine
		else {
			if (job == schedule->taskInfo[arc->x].task->job)
				tailX = FuzzyFW::TFN(0, 0, 0);
			else
				tailX = FuzzyFW::TFN(-Infd, -Infd, -Infd);
		}


		// Intermiedate node Y
		if (jsy != -1) {
			tailY = maximum(tailX + schedule->taskInfo[arc->x].task->p,
				this->tails[jsy][job] + schedule->taskInfo[jsy].task->p, maxType);
		}
		// Last node of the job
		else {
			if (job == schedule->taskInfo[arc->y].task->job) {
				tailY =
					maximum(tailX + schedule->taskInfo[arc->x].task->p,
						FuzzyFW::TFN(0, 0, 0), maxType);
			}
			else {
				tailY = tailX + schedule->taskInfo[arc->x].task->p;
			}
		}

		// Calculate the ESD for the current job
		if (tailX.expectedValue() >= 0 || tailY.expectedValue() >= 0) {
			Ctime = maximum(headX + schedule->taskInfo[x].task->p + tailX,
				headY + schedule->taskInfo[y].task->p + tailY,
				FuzzyFW::TFN::M_COMPONENT);
		}
		else
			Ctime = this->schedule->getCTJob(job);

		sum += problem->getTimeWindow(job)->agreementIndex(Ctime.expectedValue());
	}

	arc->setEstimatedQuality(
		new FuzzyFW::FitnessDouble(sum / problem->getNumberJobs(), true));
}





//=============================================================================
//
//	Class NB_ParallelN1_ESDminFJSP
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Find neighbours  -----------------------------------------------------
unsigned int NB_ParallelN1_ESDminFJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	ScheduledTaskInfo task, mp, jp;
	std::queue<int> taskQueue;
	std::vector<char> added;
	ProblemFJSP * fuzzyProb;
	double jobESD, currentESDmin;

	// Get the problem in its true form
	fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);
	if (nTasks < 2)
		return 1;

	currentESDmin = this->currentFitness->getValue();
	this->numNeighbours = 0;

	// Look for critical paths in each parallell graph:
	for (short int comp = 1; comp <= 3; comp++) {
		// Look for the last task of each job such that the ESD is 
		// equal to the current ESDmin
		for (unsigned int job = 0; job < fuzzyProb->getNumberJobs(); job++) {
			jobESD = fuzzyProb->getTimeWindow(job)->agreementIndex(
				this->schedule->getCTJob(job).expectedValue());
			if (compareDouble(jobESD, currentESDmin) <= 0)
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
FuzzyFW::Fitness * NB_ParallelN1_ESDminFJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {


	ScheduleFJSP *newSolution;
	int job, mac;
	int jsx, jsy, mpx, msy;
	int z, mpz, jpz, msz, jsz;
	double jobESD;
	FuzzyFW::TFN newHead;
	std::queue<int> taskQueue;

	if (idx < 0 || idx > this->numNeighbours) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	// Get the problem in its true form
	ProblemFJSP *fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Neighbourhood", errorMsg);
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
	if(this->schedule->taskInfo[arc->y].ms != -1)
		taskQueue.push(this->schedule->taskInfo[arc->y].ms);

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
		else if (jpz == -1)
			newHead = newSolution->taskInfo[mpz].head + newSolution->taskInfo[mpz].task->p;
		else
			newHead = newSolution->taskInfo[jpz].head + newSolution->taskInfo[jpz].task->p;

		if (!(newSolution->taskInfo[z].head.isEqualTo(newHead,
			FuzzyFW::TFN::Compare::C_COMPONENT))) {
			newSolution->taskInfo[z].head = newHead;
			if (msz != -1) taskQueue.push(msz);
			if (jsz != -1) taskQueue.push(jsz);
		}
	}

	double minESD = 2.0;
	for (size_t i = 0; i < newSolution->lastTaskJob.size(); i++) {
		jobESD = fuzzyProb->getTimeWindow(i)->agreementIndex(
			newSolution->getCTJob(i).expectedValue());
		if (jobESD < minESD)
			minESD = jobESD;
	}

	newSolution->setSorted(false);
	this->neighbours[idx]->setEvaluation(newSolution,
		new FuzzyFW::FitnessDouble(minESD, true));

	return this->neighbours[idx]->getEvaluatedFitness();
}



//-----  Estimate the quality of a neighbour through heads and tails  ---------
void NB_ParallelN1_ESDminFJSP::estimateHeadsTails(const ProblemFJSP *problem,
	const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw new FJSPException("Neighbourhood", errorMsg);
	}

	NeighbourFJSP_Arc *arc = this->neighbours[idx];
	FuzzyFW::TFN tailX, tailY, headX, headY;
	FuzzyFW::TFN Ctime;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	unsigned int x = arc->x;
	unsigned int y = arc->y;
	double esd, minESD = 2.0;
	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;

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

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(schedule->taskInfo[mpx].head + schedule->taskInfo[mpx].task->p,
			schedule->taskInfo[jpy].head + schedule->taskInfo[jpy].task->p, maxType);
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

	for (unsigned int job = 0; job < problem->getNumberJobs(); job++) {
		// Intermiedate node X
		if (msy != -1 && jsx != -1) {
			tailX = maximum(this->tails[msy][job] + schedule->taskInfo[msy].task->p,
				this->tails[jsx][job] + schedule->taskInfo[jsx].task->p, maxType);
		}
		// Last node of the machine, but not last of the job
		else if (jsx != -1) {
			tailX = this->tails[jsx][job] + schedule->taskInfo[jsx].task->p;
		}
		// Last node of the job
		else if (msy != -1) {
			if (job == schedule->taskInfo[arc->x].task->job) {
				tailX =
					maximum(this->tails[msy][job] + schedule->taskInfo[msy].task->p,
						FuzzyFW::TFN(0, 0, 0), maxType);
			}
			else {
				tailX = this->tails[msy][job] + schedule->taskInfo[msy].task->p;
			}
		}
		// Last node of the job and the machine
		else {
			if (job == schedule->taskInfo[arc->x].task->job)
				tailX = FuzzyFW::TFN(0, 0, 0);
			else
				tailX = FuzzyFW::TFN(-Infd, -Infd, -Infd);
		}


		// Intermiedate node Y
		if (jsy != -1) {
			tailY = maximum(tailX + schedule->taskInfo[arc->x].task->p,
				this->tails[jsy][job] + schedule->taskInfo[jsy].task->p, maxType);
		}
		// Last node of the job
		else if (job == schedule->taskInfo[arc->y].task->job) {
			tailY =
				maximum(tailX + schedule->taskInfo[arc->x].task->p,
					FuzzyFW::TFN(0, 0, 0), maxType);
		}
		else {
			tailY = tailX + schedule->taskInfo[arc->x].task->p;
		}

		// Calculate the ESD for the current job
		if (tailX.expectedValue() >= 0 || tailY.expectedValue() > 0) {
			Ctime = maximum(headX + schedule->taskInfo[x].task->p + tailX,
				headY + schedule->taskInfo[y].task->p + tailY,
				FuzzyFW::TFN::M_COMPONENT);
		}
		else
			Ctime = this->schedule->getCTJob(job);
		esd = problem->getTimeWindow(job)->agreementIndex(Ctime.expectedValue());
		if (esd < minESD)
			minESD = esd;
	}

	arc->setEstimatedQuality(
		new FuzzyFW::FitnessDouble(minESD, true));
}




/*
bool NB_ParallelN1_ESDavgFJSP::checkColas(const FuzzyFW::SharedVars *svars) {

	ProblemFJSP * problem = dynamic_cast<ProblemFJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to FJSP problems";
		throw new FJSPException("Neighbourhood", errorMsg);
	}
	std::vector< std::vector<FuzzyFW::TFN> > supertails;

	// Compute the tails of the operations
	int mp, jp, ms, js, taskIdx, mac;
	std::queue<int> taskQueue;
	FuzzyFW::TFN::Maximum maxType = FuzzyFW::TFN::M_COMPONENT;

	std::vector<char> visited(problem->getNumberTasks(), false);
	supertails.resize(problem->getNumberTasks());

	for (unsigned int i = 0; i < problem->getNumberTasks(); i++) {
		supertails[i].resize(problem->getNumberJobs(), FuzzyFW::TFN(-1, -1, -1));
	}

	// Look for the last task of each job
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

		// Intermiedate node
		for (unsigned int job = 0; job < problem->getNumberJobs(); job++) {
			if (ms != -1 && js != -1) {
				supertails[taskIdx][job] =
					maximum(supertails[ms][job] + schedule->taskInfo[ms].task->p,
						supertails[js][job] + schedule->taskInfo[js].task->p, maxType);
			}

			// Last node of the machine, but not last of the job
			else if (js != -1) {
				supertails[taskIdx][job] =
					supertails[js][job] + schedule->taskInfo[js].task->p;
			}

			// Last node of the job
			else if (ms != -1) {
				if (job == schedule->taskInfo[taskIdx].task->job) {
					supertails[taskIdx][job] =
						maximum(supertails[ms][job] + schedule->taskInfo[ms].task->p,
							FuzzyFW::TFN(0, 0, 0), maxType);
				}
				else {
					supertails[taskIdx][job] =
						supertails[ms][job] + schedule->taskInfo[ms].task->p;
				}
			}

			// Last node of the job and the machine
			else {
				if (job == schedule->taskInfo[taskIdx].task->job)
					supertails[taskIdx][job] = FuzzyFW::TFN(0, 0, 0);
				else
					supertails[taskIdx][job] = FuzzyFW::TFN(-Infd, -Infd, -Infd);
			}
		}

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

	for (size_t i = 0; i < supertails.size(); i++)
		for (size_t j = 0; j < supertails[i].size(); j++)
			if (!supertails[i][j].isEqualTo(this->tails[i][j], FuzzyFW::TFN::Compare::C_COMPONENT))
				std::cout << "Error";
	return false;
}
*/
}
