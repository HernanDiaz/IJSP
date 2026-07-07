/*
* NeighbourhoodIJSP_N2ME.h
*
*  Created on: Jul 5, 2026
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_N2.h"
#include "FitnessMO.h"
#include "ProblemIJSP.h"
#include <memory>

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2ME_MakespanEnergyIJSP
//
//=============================================================================
/**
* N2ME bi-critical neighbourhood for lexicographic (makespan, NPE)
* minimisation in the IJSP.
*
* Candidate arcs are the union of:
*  - the N2 arcs: boundary arcs of extreme critical blocks (makespan
*    component, inherited generation logic), and
*  - the energy-critical arcs E(sigma): tight disjunctive arcs lying on
*    sustaining chains of (i) right endpoints of positive idle gaps and
*    (ii) the last task of every machine, in either extreme graph.
*
* Feasibility of every reversal follows from arc tightness (the ASOC
* feasibility argument only uses tightness). Neighbours are evaluated with
* a lexicographic fitness [makespan, NPE], both FitnessInterval ranked by
* the globally configured comparison (LEX2), so the tabu search itself
* becomes energy-aware inside makespan plateaus.
*
* Requires the individual fitness to be a FitnessLexicographic (pair with
* the 'ijsp.makespan-energy' evaluation). Estimations (heads&tails) are
* wrapped as [estimated makespan, 0]: optimistic in energy, so the
* estimation filter never discards an energy move by mistake.
*
* @author hdiaz
*/
class NB_ParallelN2ME_MakespanEnergyIJSP : public NB_ParallelN2_MakespanIJSP {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Full lexicographic fitness [makespan, NPE] of the current solution.
	* The base-class currentFitness holds the makespan component so the
	* inherited pruning machinery keeps working.
	*/
	FuzzyFW::FitnessLexicographic *currentLexFitness;

	/*
	* Problem (for machine passive powers), cached at setInitialSolution
	*/
	const ProblemIJSP *problem;

	/*
	* Restricted energy-arc generation (default). The full sustaining-chain
	* set does not scale to large instances; the restricted variant seeds
	* only the machine-last task and the largest-gap successor per machine
	* and adds one tight machine arc per seed. Disable with
	* neighbourhood.energy-restricted = no
	*/
	bool restrictedE;

	/*
	* Optional makespan goal cap (SweepPareto phase B): when set, the
	* primary component of every neighbour fitness is clamped to
	* max(Cmax, cap) per interval component, so all under-cap solutions
	* tie on makespan and the tie-break (NPE) drives the search.
	*/
	bool hasGoalCap;
	FuzzyFW::Interval goalCap;

public:
	void setGoalCap(const FuzzyFW::Interval &cap) {
		this->hasGoalCap = true;
		this->goalCap = cap;
	}
	void clearGoalCap() { this->hasGoalCap = false; }

	/*
	* Phase-B override: with the makespan capped, the energy arcs are the
	* engine of the search and the full sustaining-chain set may pay off
	*/
	void setEnergyRestricted(const bool restricted) {
		this->restrictedE = restricted;
	}
	bool isEnergyRestricted() const { return this->restrictedE; }

protected:

	/*
	* Lexicographic wrappers of the per-neighbour estimations, rebuilt on
	* demand (owned here; base estimations stay owned by the neighbours)
	*/
	std::vector<std::unique_ptr<FuzzyFW::FitnessLexicographic> > lexEstimations;

	//=========================================================================
	//		CONSTRUCTORS / DESTRUCTOR
	//=========================================================================
public:
	NB_ParallelN2ME_MakespanEnergyIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanIJSP(parameters),
		currentLexFitness(NULL), problem(NULL), restrictedE(true),
		hasGoalCap(false), goalCap(0, 0) { }

	NB_ParallelN2ME_MakespanEnergyIJSP(
		const NB_ParallelN2ME_MakespanEnergyIJSP & source)
		: NB_ParallelN2_MakespanIJSP(source),
		currentLexFitness(source.currentLexFitness == NULL ? NULL :
			dynamic_cast<FuzzyFW::FitnessLexicographic *>(
				source.currentLexFitness->clone())),
		problem(source.problem), restrictedE(source.restrictedE),
		hasGoalCap(source.hasGoalCap), goalCap(source.goalCap) { }

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual ~NB_ParallelN2ME_MakespanEnergyIJSP() {
		if (this->currentLexFitness != NULL)
			delete this->currentLexFitness;
	}

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2ME_MakespanEnergyIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("MakespanEnergy-N2ME",
			this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	//=========================================================================
	//		METHODS
	//=========================================================================
	virtual void setInitialSolution(FuzzyFW::Solution *solution,
		FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->schedule;
		solution.second = this->currentLexFitness;
		return solution;
	}

	virtual FuzzyFW::Fitness *getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);
};

}
