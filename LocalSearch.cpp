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
	neighbours(0), iterations(0), neighbourhood(NULL) {
	if (parameters != NULL)
		this->setup(parameters);
}


//-----  Copy constructor  ----------------------------------------------------
LocalSearch::LocalSearch(const LocalSearch &source)
	: iterationLabel(source.iterationLabel), iterations(source.iterations),
	evaluationLabel(source.evaluationLabel), evaluations(source.evaluations),
	maxIterations(source.maxIterations), maxEvaluations(source.maxEvaluations),
	timeLabel(source.timeLabel), maxTime(source.maxTime),
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

	int next;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;

	FullSolution current, neighbour;
	current.first = solution->clone();
	current.second = fitness->clone();

	bool improves = true;

	while (improves && !this->stoppingCriteria()) {
		improves = false;
		this->neighbours +=
			this->neighbourhood->findNeighbours(
				current.first, current.second, svars);

		while (!improves && this->neighbourhood->size() > 0) {
			// Pick a random number
			next = svars->rng->getInteger(0, this->neighbourhood->size() - 1);
			this->evaluations++;

			// Move to the new neighbour
			neighbour = this->neighbourhood->evaluateNeighbour(next, svars, true);

			if (neighbour.first != NULL
				&& neighbour.second->isBetterThan(current.second)) {
				improves = true;
				delete current.first;
				delete current.second;
				current.first = neighbour.first;
				current.second = neighbour.second;
				this->iterations++;
			}
			else {
				this->neighbourhood->discardNeighbour(next);
				if (neighbour.first != NULL) {
					delete neighbour.first;
					delete neighbour.second;
				}
			}
		}
	}

	return current;
}


}