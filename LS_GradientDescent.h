/*
* LS_GradientDescent.h
*
*  Created on: Oct 11, 2017
*/
#pragma once

#include "LocalSearch.h"
#include "LS_HillClimbing.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class LS_GradientDescent
//
//=============================================================================
/**
* Gradient Descent local search: generates the neighbourhood and finds the
* best improving neighbour at each iteration.
*/
class LS_GradientDescent : public LocalSearch {
public:
	explicit LS_GradientDescent(ParameterDB *parameters = NULL)
		: LocalSearch(parameters) { }

	LS_GradientDescent(const LS_HillClimbing &source)
		: LocalSearch(source) { }

	virtual void setup(ParameterDB *parameters) {
		LocalSearch::setup(parameters);
	}

	virtual LS_GradientDescent * clone() const {
		return new LS_GradientDescent(*this);
	}

	virtual ~LS_GradientDescent() { }

	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("HillClimbing");
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
