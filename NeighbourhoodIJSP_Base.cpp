/*
 * NeighbourhoodIJSP_Base.cpp
 *
 *  Created on: May 3, 2026
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_Base.h"
#include "ProblemIJSP.h"
#include "IJSPException.h"
#include "heading.h"

namespace IJSP {

//-----  Copy constructor  ----------------------------------------------------
NB_ParallelBase_MakespanIJSP::NB_ParallelBase_MakespanIJSP(
	const NB_ParallelBase_MakespanIJSP &source)
	: Neighbourhood(source), estimator(source.estimator),
	estimatorLabel(source.estimatorLabel), schedule(NULL),
	currentFitness(NULL)
{
	for (size_t i = 0; i < source.neighbours.size(); i++) {
		if (source.neighbours[i] != nullptr)
			neighbours.push_back(std::make_unique<NeighbourIJSP_Arc>(*source.neighbours[i]));
		else
			neighbours.push_back(nullptr);
	}
}


//-----  Setup method  --------------------------------------------------------
void NB_ParallelBase_MakespanIJSP::setup(FuzzyFW::ParameterDB *parameters) {
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
		throw IJSPException("Neighbourhood", errorMsg);
	}
}


//-----  Destructor  ----------------------------------------------------------
NB_ParallelBase_MakespanIJSP::~NB_ParallelBase_MakespanIJSP() {
	neighbours.clear();
}


//-----  Set the Initial Solution  --------------------------------------------
void NB_ParallelBase_MakespanIJSP::setInitialSolution(
	FuzzyFW::Solution *solution, FuzzyFW::Fitness *fitness,
	const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Interval::Maximum maxType = FuzzyFW::Interval::M_COMPONENT;
	if (this->schedule != NULL)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (this->schedule == NULL) {
		std::string errorMsg = "Type of solution not valid for this type";
		errorMsg += " of neighbourhood. Only Fuzzy IJSP Schedules are allowed.";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentFitness != NULL)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(fitness);
	if (this->currentFitness == NULL) {
		std::string errorMsg = "The fitness of the solution is not the ";
		errorMsg += "makespan";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	ProblemIJSP *problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (problem == NULL) {
		std::string errorMsg = "This negihbourhood can be applied only";
		errorMsg += " to IJSP problems";
		throw IJSPException("Neighbourhood", errorMsg);
	}

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
			this->tails[taskIdx] = this->tails[js] + schedule->taskInfo[js].task->p;
		else this->tails[taskIdx] = FuzzyFW::Interval(0, 0);

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


//-----  Get the estimation  --------------------------------------------------
FuzzyFW::Fitness *NB_ParallelBase_MakespanIJSP::getEstimation(
	const unsigned int idx, const FuzzyFW::SharedVars *svars) {

	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEstimated()) {
		if (this->estimator == ESTIM_HEADTAILS)
			this->estimateHeadsTails(idx);
		else
			this->neighbours[idx]->setEstimatedQuality(
				new FuzzyFW::FitnessInterval(FuzzyFW::Interval(Infd, Infd), false));
	}
	return this->neighbours[idx]->getEstimatedQuality();
}


//-----  Discard a neighbour  -------------------------------------------------
void NB_ParallelBase_MakespanIJSP::discardNeighbour(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}
	this->neighbours[idx].reset();
}


//-----  Estimate quality through heads and tails (tipo-0 arc swap)  ----------
void NB_ParallelBase_MakespanIJSP::estimateHeadsTails(const unsigned int idx) {
	if (idx < 0 || idx >= this->numNeighbours) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::Interval tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::Interval makespan;

	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
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
			this->tails[msy] + schedule->taskInfo[msy].task->p,
			FuzzyFW::Interval::M_COMPONENT);
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


//-----  Sort by estimation  --------------------------------------------------
void NB_ParallelBase_MakespanIJSP::sortByEstimation(
	const FuzzyFW::SharedVars *svars) {

	if (this->estimator == Estimator::NONE)
		return;
	this->quickSort(0, this->numNeighbours - 1, svars);
}


//-----  Quick sort  ----------------------------------------------------------
void NB_ParallelBase_MakespanIJSP::quickSort(const int left, const int right,
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


//-----  Gets a specific neighbour  -------------------------------------------
FuzzyFW::Neighbour *NB_ParallelBase_MakespanIJSP::getNeighbour(
	const unsigned int idx) {

	if (idx < 0 || idx >= this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to acces a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}
	return this->neighbours[idx].get();
}

} // namespace IJSP
