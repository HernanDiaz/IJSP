/*
* NeighbourhoodIJSP_NH.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourIJSP.h"
#include "Neighbourhood.h"
#include "NeighbourhoodIJSP_helpers.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelNH_MakespanIJSP
//
//=============================================================================
/**
* NH (extended) neighbourhood for Makespan minimization in IJSP. Considers
* boundary arcs from all extreme critical paths across both extreme graphs.
*
* @author hdiaz
*/
class NB_ParallelNH_MakespanIJSP : public FuzzyFW::Neighbourhood {
protected:
	enum Estimator { NONE, ESTIM_HEADTAILS };

	std::string estimatorLabel;
	Estimator estimator;
	ScheduleIJSP *schedule;
	FuzzyFW::FitnessInterval *currentFitness;
	std::vector<NeighbourIJSP_Arc *> neighbours;
	std::vector<FuzzyFW::Interval> tails;

public:
	NB_ParallelNH_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }

	NB_ParallelNH_MakespanIJSP(const NB_ParallelNH_MakespanIJSP & source);

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual Neighbourhood * clone() const {
		return new NB_ParallelNH_MakespanIJSP(*this);
	}

	~NB_ParallelNH_MakespanIJSP();

	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->schedule;
		solution.second = this->currentFitness;
		return solution;
	}

	virtual FuzzyFW::Neighbour* getNeighbour(const unsigned int idx);

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-NH", this->estimator == Estimator::ESTIM_HEADTAILS);
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
