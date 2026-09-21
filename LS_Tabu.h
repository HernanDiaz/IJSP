/*
* LS_Tabu.h
*
*  Created on: Oct 11, 2017
*/
#pragma once

#include "LocalSearch.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class LS_Tabu
//
//=============================================================================
/**
* Tabu Search: generates the neighbourhood and finds the best neighbour,
* avoiding moves in the tabu list. Stops after a fixed number of iterations
* without improving the best solution found so far.
*/
class LS_Tabu : public LocalSearch {
protected:
	TabuList *tabuList;
	unsigned int maxBadIterations;
	std::string badIterationsLabel;
	unsigned int badIterations;

public:
		//-----  DIAGNOSTIC 2026-09-21  --------------------------------------
		// Does the order in which N2's neighbours are visited decide anything?
		// It cannot change which VALUE is best, but isBetterThan is strict, so
		// among several neighbours attaining that value the scan keeps the
		// FIRST one it reaches, and with the estimation filter on it never even
		// evaluates the others. These counters measure how often that choice
		// exists, and whether the heads&tails estimate is a true lower bound.
		// Run them with localsearch.filter = no, so every neighbour is seen.
		static unsigned long diagTieSum;        // ties at the best eligible value
		static unsigned long diagIters;         // tabu iterations counted
		static unsigned long diagScanned;       // neighbours evaluated
		static unsigned long diagBoundBreaks;   // estimation strictly worse than the real value
		static unsigned long diagTieMax;        // the largest tie seen

	explicit LS_Tabu(ParameterDB *parameters = NULL);
	LS_Tabu(const LS_Tabu &source);

	virtual void setup(ParameterDB *parameters);

	virtual LS_Tabu * clone() const {
		return new LS_Tabu(*this);
	}

	virtual ~LS_Tabu() {
		delete this->tabuList;
	}

protected:
	virtual bool stoppingCriteria();

public:
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup, tabu;
		tabu = this->tabuList->getName();
		setup.push_back("Tabu Search");
		for (size_t i = 0; i < tabu.size(); i++)
			setup.push_back(";" + tabu[i]);
		setup.push_back(";Max. Iterations without Improve:;"
			+ valueToString(this->maxBadIterations));
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
