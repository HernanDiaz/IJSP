/*
* LocalSearch.cpp
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/

#include "LocalSearch.h"

namespace FJSP {

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
	: iterationLabel(LOCAL_SEARCH_ITER), evaluationLabel(LOCAL_SEARCH_EVAL),
	evaluations(0), timeLabel(LOCAL_SEARCH_TIME), maxTime(0.0),
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
Individual * LS_HillClimbing::apply(const Individual *individual,
	const SharedVars *svars) {

	int next;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;

	Individual *current = individual->clone();
	Individual *neighbour;
	Fitness *currentFitness = individual->getFitness();

	bool improves = true;

	while (improves && !this->stoppingCriteria()) {
		improves = false;
		this->neighbours +=
			this->neighbourhood->findNeighbours(current, svars);

		while (!improves && this->neighbourhood->size() > 0) {
			// Pick a random number
			next = svars->rng->getInteger(0, this->neighbourhood->size() - 1);
			this->evaluations++;

			// Move to the new neighbour
			neighbour = this->neighbourhood->evaluateNeighbour(next, svars, true);

			if (neighbour != NULL && neighbour->getFitness()->isBetterThan(currentFitness)) {
				improves = true;
				delete current;
				current = neighbour;
				currentFitness = neighbour->getFitness();
				this->iterations++;
			}
			else {
				this->neighbourhood->discardNeighbour(next);
				if(neighbour != NULL)
					delete neighbour;
			}
		}
	}

	return current;
}


}
