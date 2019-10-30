/*
* LocalSearch.cpp
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/

#include "LocalSearch.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class LocalSearch
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LocalSearch::LocalSearch(ParameterDB *parameters)
	: iterationLabel(FUZZYFW_LOCAL_SEARCH_ITER),
	evaluationLabel(FUZZYFW_LOCAL_SEARCH_EVAL),
	evaluations(0), timeLabel(FUZZYFW_LOCAL_SEARCH_TIME), maxTime(0.0),
	neighbours(0), iterations(0), neighbourhood(NULL),
	guideLabel(FUZZYFW_LOCAL_SEARCH_DRIVE), estimationGuided(false),
	filterLabel(FUZZYFW_LOCAL_SEARCH_FILTER), estimationFilter(false)
	{
	if (parameters != NULL)
		this->setup(parameters);
}


//-----  Copy constructor  ----------------------------------------------------
LocalSearch::LocalSearch(const LocalSearch &source)
	: iterationLabel(source.iterationLabel), iterations(source.iterations),
	evaluationLabel(source.evaluationLabel), evaluations(source.evaluations),
	maxIterations(source.maxIterations), maxEvaluations(source.maxEvaluations),
	timeLabel(source.timeLabel), maxTime(source.maxTime),
	guideLabel(source.guideLabel), estimationGuided(source.estimationGuided),
	filterLabel(source.filterLabel), estimationFilter(source.estimationFilter),
	neighbours(source.neighbours) {

	if (source.neighbourhood != NULL)
		neighbourhood = source.neighbourhood->clone();
	else neighbourhood = NULL;
}



//-----  Setup method  --------------------------------------------------------
void LocalSearch::setup(ParameterDB *parameters) {
	// Loads the maximum number of iterations
	this->maxIterations = parameters->getInteger(this->iterationLabel, -1);
	this->maxEvaluations = parameters->getInteger(this->evaluationLabel, -1);
	// Loads the maximumtime run
	this->maxTime = parameters->getDouble(this->timeLabel, -1.0);

	// Loads the main drive
	this->estimationGuided = parameters->getBoolean(this->guideLabel, false);

	// Loads the filter mechanism
	this->estimationFilter = parameters->getBoolean(this->filterLabel, false);
}


//-----  Stopping criteria  ---------------------------------------------------
bool LocalSearch::stoppingCriteria() {
	if (this->maxEvaluations >= 0 &&
		this->evaluations >= this->maxEvaluations)
		return true;
	if (this->maxIterations >= 0 &&
		this->iterations >= this->maxIterations)
		return true;

	double currentRuntime = this->runtime / (double)CLOCKS_PER_SEC;
	if (this->maxTime > 0 &&
		compareDouble(currentRuntime, this->maxTime) >= 0)
		return true;

	return false;
}





//=============================================================================
//
//	Class LS_HillClimbing
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
FullSolution LS_HillClimbing::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int next, index, nNeighbours;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;

	FullSolution current;
	Fitness *estimation, *realValue;
	std::vector<int> randomArray;

	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();

	bool improves = true;
	while (improves && !this->stoppingCriteria()) {
		improves = false;
		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		randomArray = svars->rng->getRandomVector(0, nNeighbours - 1);

		while (!improves && nNeighbours > 0) {
			// Pick a random number
			next = svars->rng->getInteger(0, nNeighbours - 1);
			index = randomArray[next];

			// Estimate the quality of a neighbour
			estimation = this->neighbourhood->getEstimation(index, svars);
			
			if (!this->estimationFilter || estimation->isBetterThan(current.second)) {
				if (this->estimationGuided) {
					if (estimation->isBetterThan(current.second)) {
						improves = true;
						this->neighbourhood->acceptNeighbour(index, svars);
						current = this->neighbourhood->getCurrentSolution();
						this->evaluations++;
						this->iterations++;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);

					this->evaluations++;
					if (realValue->isBetterThan(current.second)) {
						improves = true;
						this->neighbourhood->acceptNeighbour(index, svars);
						current = this->neighbourhood->getCurrentSolution();
						this->iterations++;
					}
				}
			}
			if (!improves) {
				//this->neighbourhood->discardNeighbour(next);
				std::swap(randomArray[next], randomArray[randomArray.size() - 1]);
				nNeighbours--;
			}
		}
	}

	current.first = current.first->clone();
	current.second = current.second->clone();
	return current;
}





//=============================================================================
//
//	Class LS_GradientDescent
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
FullSolution LS_GradientDescent::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;

	FullSolution current;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;

	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();

	bool improves = true;
	while (improves && !this->stoppingCriteria()) {
		improves = false;
		bestNeighbor = -1;
		best = current.second;

		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		this->neighbourhood->sortByEstimation(svars);

		index = 0;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);

			if (!this->estimationFilter || estimation->isBetterThan(best)) {
				if (this->estimationGuided) {
					if (estimation->isBetterThan(best)) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);
					this->evaluations++;
					if (realValue->isBetterThan(best)) {
						best = realValue;
						bestNeighbor = index;
					}
				}
				index++;
			}
			else
				index = nNeighbours;
		}
		if (bestNeighbor >= 0) {
			improves = true;
			this->neighbourhood->acceptNeighbour(bestNeighbor, svars);
			current = this->neighbourhood->getCurrentSolution();
			this->iterations++;
			if (this->estimationGuided)
				this->evaluations++;
		}
	}

	current.first = current.first->clone();
	current.second = current.second->clone();
	return current;
}





//=============================================================================
//
//	Class LS_Tabu
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LS_Tabu::LS_Tabu(ParameterDB *parameters)
	: LocalSearch(parameters), maxBadIterations(0),
	badIterationsLabel(FUZZYFW_LOCAL_SEARCH_TABUITER),
	badIterations(0)
{
	tabuList = new TabuList(parameters);
}


//-----  Copy constructor  ----------------------------------------------------
LS_Tabu::LS_Tabu(const LS_Tabu &source)
	: LocalSearch(source), maxBadIterations(source.maxBadIterations),
	badIterationsLabel(badIterationsLabel),
	badIterations(source.badIterations)
{
	tabuList = source.tabuList->clone();
}



//-----  Setup method  --------------------------------------------------------
void LS_Tabu::setup(ParameterDB *parameters) {
	LocalSearch::setup(parameters);
	// Loads the maximum number of iterations without improvement
	this->maxBadIterations = parameters->getInteger(this->badIterationsLabel, -1);

	if (this->maxBadIterations < 0) {
		std::string errorMsg = "Number of iterations without improvement not";
		errorMsg += " specified";
		throw new FuzzyFWException("Tabu Search",errorMsg);
	}
}


//-----  Stopping criteria  ---------------------------------------------------
bool LS_Tabu::stoppingCriteria() {
	if (LocalSearch::stoppingCriteria())
		return true;
	if (this->badIterations >= this->maxBadIterations)
		return true;

	return false;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply method  --------------------------------------------------------
FullSolution LS_Tabu::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	FullSolution current, bestSolution;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;
	Neighbour *lastNeighbour = NULL;

	unsigned int isTabu, isLastNeighbour;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	this->badIterations = 0;

	// Set the initial solution of the nieghbourhood
	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();
	bestSolution.first = solution->clone();
	bestSolution.second = fitness->clone();

	bool improves = true;
	this->tabuList->clear();
	while (!this->stoppingCriteria()) {
		improves = false;
		bestNeighbor = -1;
		best = NULL;

		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		this->neighbourhood->sortByEstimation(svars);

		index = 0;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);
			isTabu = this->tabuList->isTabu(this->neighbourhood->getNeighbour(index));
			if(lastNeighbour != NULL)
				isLastNeighbour = lastNeighbour->isReverse(this->neighbourhood->getNeighbour(index));

			if (!this->estimationFilter || best == NULL || estimation->isBetterThan(best)) {
				if (this->estimationGuided) {
					if ((best == NULL || estimation->isBetterThan(best))
						&& (!isTabu || estimation->isBetterThan(bestSolution.second))) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, false);
					this->evaluations++;
					if (realValue != NULL
						&& (best == NULL || realValue->isBetterThan(best))
						&& (!isTabu || realValue->isBetterThan(bestSolution.second))
						&& (lastNeighbour == NULL || !isLastNeighbour)) {
						best = realValue;
						bestNeighbor = index;
					}
				}
				index++;
			}
			else
				index = nNeighbours;
		}

		if (bestNeighbor >= 0) {
			if (lastNeighbour != NULL)
				delete lastNeighbour;
			lastNeighbour = this->neighbourhood->getNeighbour(bestNeighbor)->clone();

			this->tabuList->addNeighbour(this->neighbourhood->getNeighbour(bestNeighbor));
			this->neighbourhood->acceptNeighbour(bestNeighbor, svars);
			current = this->neighbourhood->getCurrentSolution();
			this->iterations++;
			if (this->estimationGuided)
				this->evaluations++;

			if (current.second->isBetterThan(bestSolution.second)) {
				delete bestSolution.first;
				delete bestSolution.second;
				bestSolution.first = current.first->clone();
				bestSolution.second = current.second->clone();
				improves = true;
				this->badIterations = 0;
			}
			else
				this->badIterations++;
		}
		// No neihgbours: Dead end
		else
			this->badIterations = this->maxBadIterations;
	}

	if(lastNeighbour != NULL)
		delete lastNeighbour;
	return bestSolution;
}

}

