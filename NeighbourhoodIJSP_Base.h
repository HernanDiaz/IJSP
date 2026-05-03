/*
 * NeighbourhoodIJSP_Base.h
 *
 *  Created on: May 3, 2026
 *      Author: hdiaz
 */
#pragma once

#include "NeighbourIJSP.h"
#include "Neighbourhood.h"
#include "NeighbourhoodIJSP_helpers.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelBase_MakespanIJSP
//
//=============================================================================
/**
 * Abstract base for all Makespan IJSP parallel neighbourhood classes.
 * Holds shared fields (schedule, fitness, neighbours, tails, estimator) and
 * implements the infrastructure methods that are identical across N1/N2/N3/NH:
 * setup, copy ctor, destructor, setInitialSolution, getEstimation,
 * discardNeighbour, estimateHeadsTails (tipo-0), sortByEstimation, quickSort,
 * getNeighbour, getCurrentSolution.
 *
 * Subclasses must implement: findNewNeighbours, evaluateNeighbour,
 * acceptNeighbour, clone.
 *
 * @author hdiaz
 */
class NB_ParallelBase_MakespanIJSP : public FuzzyFW::Neighbourhood {
protected:
	enum Estimator { NONE, ESTIM_HEADTAILS };

	std::string estimatorLabel;
	Estimator estimator;
	ScheduleIJSP *schedule;
	FuzzyFW::FitnessInterval *currentFitness;
	std::vector<NeighbourIJSP_Arc *> neighbours;
	std::vector<FuzzyFW::Interval> tails;

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	NB_ParallelBase_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }

	NB_ParallelBase_MakespanIJSP(const NB_ParallelBase_MakespanIJSP &source);

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual ~NB_ParallelBase_MakespanIJSP();

	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->schedule;
		solution.second = this->currentFitness;
		return solution;
	}

	virtual FuzzyFW::Neighbour *getNeighbour(const unsigned int idx);

	virtual void setInitialSolution(FuzzyFW::Solution *solution,
		FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	virtual void discardNeighbour(const unsigned int idx);

	virtual void sortByEstimation(const FuzzyFW::SharedVars *svars);

	// Pure virtual — each neighbourhood defines its own search and move
	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars) = 0;
	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false) = 0;
	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars) = 0;

protected:
	virtual void estimateHeadsTails(const unsigned int idx);
	void quickSort(const int left, const int right,
		const FuzzyFW::SharedVars *svars);
};

} // namespace IJSP
