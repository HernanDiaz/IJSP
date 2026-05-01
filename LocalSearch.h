/*
* LocalSearch.h
*
*  Created on: Oct 11, 2017
*/
#pragma once

#include "Neighbourhood.h"
#include "TabuList.h"


namespace FuzzyFW {

#define FUZZYFW_LOCAL_SEARCH_ITER "localsearch.max-iterations"
#define FUZZYFW_LOCAL_SEARCH_EVAL "localsearch.max-evaluations"
#define FUZZYFW_LOCAL_SEARCH_TIME "localsearch.max-time"

#define  FUZZYFW_LOCAL_SEARCH_DRIVE "localsearch.estimation-guide"
#define  FUZZYFW_LOCAL_SEARCH_FILTER "localsearch.filter"

#define FUZZYFW_LOCAL_SEARCH_TABUITER "localsearch.bad-iterations"


//=============================================================================
//
//	Abstract class LocalSearch
//
//=============================================================================
/**
* Abstract base for Local Search strategies.
*
* Subclasses implement apply() and getName(). Parameters recognised:
*   localsearch.max-iterations, localsearch.max-evaluations,
*   localsearch.max-time, localsearch.estimation-guide,
*   localsearch.filter
*/
class LocalSearch {
protected:
	std::string iterationLabel;
	int maxIterations;

	std::string evaluationLabel;
	int maxEvaluations;

	std::string timeLabel;
	double maxTime;

	std::string guideLabel;
	char estimationGuided;

	std::string filterLabel;
	char estimationFilter;

	Neighbourhood * neighbourhood;

	unsigned int evaluations;
	unsigned int neighbours;
	unsigned int iterations;
	clock_t runtime;



public:
	explicit LocalSearch(ParameterDB *parameters = NULL);
	LocalSearch(const LocalSearch &source);

	virtual void setup(ParameterDB *parameters);

	virtual LocalSearch * clone() const = 0;

	virtual ~LocalSearch() {
		delete neighbourhood;
	}


public:
	unsigned int getEvaluations() const {
		return this->evaluations;
	}

	unsigned int getIterations() const {
		return this->iterations;
	}

	unsigned int getNeighbours() const {
		return this->neighbours;
	}

	void setNeighbourhood(Neighbourhood *n) {
		this->neighbourhood = n;
	}


protected:
	virtual bool stoppingCriteria();


public:
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars) = 0;

	virtual std::vector<std::string> getName() const = 0;
};

}

#include "LS_HillClimbing.h"
#include "LS_GradientDescent.h"
#include "LS_Tabu.h"
