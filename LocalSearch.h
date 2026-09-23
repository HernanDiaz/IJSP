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
// How the tails feeding the heads&tails estimate are maintained.
//   "incremental" (default, unchanged), or "full": recomputed before every
//   estimation. See LocalSearch::setup for the measurement behind this.
#define  FUZZYFW_LOCAL_SEARCH_TAILS "localsearch.tails"
// I-005: how a tie at the best neighbourhood value is broken.
//   "first" (default, unchanged): whichever the sort reached first
//   "frequency": among the tied moves, the arc used fewest times in this run
#define  FUZZYFW_LOCAL_SEARCH_TIEBREAK "localsearch.tiebreak"
// I-010: what a tabu call does when NO neighbour is admissible, every move
// being tabu without meeting the aspiration criterion or being the reverse of
// the last one.
//   "stop"   (default, unchanged): end the call
//   "escape" : take the best neighbour anyway, ignoring the tabu status, and
//              carry on, which is the classical way out of an all-tabu state
// Measured 2026-09-22: every deep call ends this way, after 75 to 92 moves,
// so the depth of this search is capped by the dead end and not by
// localsearch.bad-iterations. Raising that limit from 15 to 30000 changes
// nothing at all.
#define  FUZZYFW_LOCAL_SEARCH_DEADEND "localsearch.deadend"

// I-013. Which neighbour of N2 the tabu search accepts.
//   "best"  (default, unchanged): the best of the whole neighbourhood. The
//           sweep sorts by the heads&tails estimate and prunes, so it reads
//           about a fifth of the neighbourhood and still returns the best.
//   "first" : visit N2 in its own order from a ROTATING start and accept the
//           first eligible neighbour that improves the current solution; if
//           none does, fall back to the best eligible of the full sweep so the
//           search still moves. The sort and the prune are skipped, because
//           both exist to find the best and the point here is not to.
// Why this and not another ordering rule: under a best-improvement rule the
// visiting order cannot change which move is taken, which is why I-004
// (exact bound), the tails defect (scrambled order) and I-005 (tie-break)
// each measured zero. Order can only decide something once the rule goes.
#define  FUZZYFW_LOCAL_SEARCH_SELECT "localsearch.select"

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

	std::string tailsLabel;
	bool fullTails;

	std::string deadEndLabel;
	bool deadEndEscape;

	std::string selectLabel;
	bool firstImprovement;

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
