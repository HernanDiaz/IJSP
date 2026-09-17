/*
* LS_TabuBackJump.h
*
*  Created on: Sep 17, 2026
*      Author: Hernan Diaz Rodriguez
*/
#pragma once

#include <deque>
#include "LS_Tabu.h"

namespace FuzzyFW {

#define FUZZYFW_BACKJUMP_SIZE "localsearch.backjump.size"
#define FUZZYFW_BACKJUMP_MAXJUMPS "localsearch.backjump.max-jumps"

//=============================================================================
//
//	Class LS_TabuBackJump
//
//=============================================================================
/**
* Tabu search with back-jump tracking, in the sense of Nowicki and Smutnicki's
* TSAB.
*
* Plain tabu search stops the moment it spends maxBadIterations iterations
* without improving, and everything it learned on the way is thrown away. Most
* of that effort was spent walking away from the best solution found, and the
* decision that started each such walk was a single move chosen among several
* admissible ones. Back-jump tracking remembers those decision points: whenever
* the search sits on the best solution known and has to take a move that does
* not improve it, the solution and the move taken are pushed on a bounded
* stack. When the inner tabu run stagnates, the search returns to the most
* recent such point and starts again with that move forbidden, so it is forced
* down a different branch instead of repeating the walk it already made.
*
* The outer loop keeps jumping back until the local search budget
* (localsearch.max-time / max-iterations / max-evaluations) is spent or the
* stack is empty, so this class costs no more time than plain tabu search with
* the same budget -- it only spends that budget better.
*
* The number of jumps per call is capped by 'localsearch.backjump.max-jumps'
* (default 3). The cap matters more than it looks: inside a population-based
* algorithm the local search is called once per individual per generation, so a
* local search that spends its whole time allowance keeps the population from
* evolving at all. Measured on ta01 with a 60 s budget, an uncapped back-jump
* search completed ONE generation where plain tabu search completed 183, and
* the gap to the lower bound went from 0.28% to 6.65%. Capping the jumps keeps
* the cost of a call within a small factor of plain tabu search.
*
* The stack size is read from 'localsearch.backjump.size' and defaults to 5.
*/
class LS_TabuBackJump : public LS_Tabu {
	//=====================================================================
	//		FIELDS
	//=====================================================================
protected:
	/**
	* One remembered decision point: the solution the search was sitting on,
	* its fitness, and the move that was taken from it and must be forbidden
	* when the search comes back.
	*/
	struct BackJumpPoint {
		Solution *solution;
		Fitness *fitness;
		Neighbour *move;
	};

	std::string backJumpSizeLabel;
	unsigned int maxBackJumpPoints;

	std::string maxJumpsLabel;
	unsigned int maxJumps;
	std::deque<BackJumpPoint> backJumpStack;

	//=====================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=====================================================================
public:
	explicit LS_TabuBackJump(ParameterDB *parameters = NULL);
	LS_TabuBackJump(const LS_TabuBackJump &source);

	virtual void setup(ParameterDB *parameters);

	virtual LS_TabuBackJump * clone() const {
		return new LS_TabuBackJump(*this);
	}

	virtual ~LS_TabuBackJump() {
		this->clearBackJumpStack();
	}

	//=====================================================================
	//		METHODS
	//=====================================================================
protected:
	/**
	* Releases every remembered decision point.
	*/
	void clearBackJumpStack();

	/**
	* Remembers a decision point, dropping the oldest one when the stack is
	* already full.
	*/
	void pushBackJumpPoint(const Solution *solution, const Fitness *fitness,
		const Neighbour *move);

public:
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup = LS_Tabu::getName();
		setup[0] = "Tabu Search with back-jump tracking";
		setup.push_back(";Back-jump stack size:;"
			+ valueToString(this->maxBackJumpPoints));
		setup.push_back(";Max. back jumps per call:;"
			+ valueToString(this->maxJumps));
		return setup;
	}
};

}
