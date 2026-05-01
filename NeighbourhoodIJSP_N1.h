/*
* NeighbourhoodIJSP_N1.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourIJSP.h"
#include "Neighbourhood.h"

#ifndef NB_ESTIMATOR_NONE
#define NB_ESTIMATOR_NONE "none"
#define NB_ESTIMATOR_HEADSTAILS "heads&tails"
#endif

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN1_MakespanIJSP
//
//=============================================================================
/**
* N1 neighbourhood for Makespan minimization in IJSP. Considers the reversal
* of all arcs that belong to at least one extreme critical path.
*
* @author hdiaz
*/
class NB_ParallelN1_MakespanIJSP : public FuzzyFW::Neighbourhood {
protected:
	enum Estimator { NONE, ESTIM_HEADTAILS };

	std::string estimatorLabel;
	Estimator estimator;
	ScheduleIJSP *schedule;
	FuzzyFW::FitnessInterval *currentFitness;
	std::vector<NeighbourIJSP_Arc *> neighbours;
	std::vector<FuzzyFW::Interval> tails;

public:
	NB_ParallelN1_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }

	NB_ParallelN1_MakespanIJSP(const NB_ParallelN1_MakespanIJSP & source);

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_MakespanIJSP(*this);
	}

	~NB_ParallelN1_MakespanIJSP();

	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->schedule;
		solution.second = this->currentFitness;
		return solution;
	}

	virtual FuzzyFW::Neighbour* getNeighbour(const unsigned int idx);

	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
		setup.push_back(value);
		return setup;
	}

	virtual void setInitialSolution(FuzzyFW::Solution *solution,
		FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars);

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness * evaluateNeighbour(const unsigned int idx,
			const FuzzyFW::SharedVars *svars, const bool improvement = false);

	virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	virtual void discardNeighbour(const unsigned int idx);

	virtual void sortByEstimation(const FuzzyFW::SharedVars *svars);

protected:
	virtual void estimateHeadsTails(const unsigned int idx);

	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);
};

}
