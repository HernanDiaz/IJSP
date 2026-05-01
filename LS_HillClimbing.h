/*
* LS_HillClimbing.h
*
*  Created on: Oct 11, 2017
*/
#pragma once

#include "LocalSearch.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class LS_HillClimbing
//
//=============================================================================
/**
* Hill Climbing local search: generates the neighbourhood and iterates
* randomly until it finds the first neighbour that improves the current
* solution.
*/
class LS_HillClimbing : public LocalSearch {
public:
	explicit LS_HillClimbing(ParameterDB *parameters = NULL)
		: LocalSearch(parameters) { }

	LS_HillClimbing(const LS_HillClimbing &source)
		: LocalSearch(source) { }

	virtual void setup(ParameterDB *parameters) {
		LocalSearch::setup(parameters);
	}

	virtual LS_HillClimbing * clone() const {
		return new LS_HillClimbing(*this);
	}

	virtual ~LS_HillClimbing() { }

	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("BF-HillClimbing");
		if (this->maxIterations >= 0)
			setup.push_back(";Max. Iterations:;"
				+ valueToString(this->maxIterations));
		if (this->maxEvaluations >= 0)
			setup.push_back(";Max. Evaluations:;"
				+ valueToString(this->maxEvaluations));
		if (this->maxTime >= 0)
			setup.push_back(";Max. Time:;"
				+ valueToString(this->maxTime));
		if (this->estimationGuided)
			setup.push_back(";Guide:;Estimations");
		else
			setup.push_back(";Guide:;Real fitness");
		if (this->estimationFilter)
			setup.push_back(";Filter:;Yes");
		else
			setup.push_back(";Filter:;No");
		return setup;
	}
};

}
