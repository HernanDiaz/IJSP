/*
* NeighbourhoodIJSP_N2.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2_MakespanIJSP
//
//=============================================================================
/**
* N2 neighbourhood for Makespan minimization in IJSP. Considers reversal of
* arcs at the extremes of critical blocks shared by at least one extreme
* critical path.
*
* @author hdiaz
*/
class NB_ParallelN2_MakespanIJSP : public NB_ParallelBase_MakespanIJSP {
public:
	NB_ParallelN2_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanIJSP(parameters) { }

	NB_ParallelN2_MakespanIJSP(const NB_ParallelN2_MakespanIJSP &source)
		: NB_ParallelBase_MakespanIJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelN2_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N2", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

private:
	/**
	* Everything applyArc() writes to the schedule, so that revertArc() can
	* put it back exactly. Evaluating a neighbour used to mean deep-copying
	* the whole schedule, reversing the arc on the copy, propagating the heads
	* and handing the copy to the neighbour, which the tabu list then cloned
	* again for every move it recorded; that copying was about a fifth of the
	* run. The move is now applied to the live schedule and undone.
	*
	* The head log can hold the same task more than once, because the
	* propagation may raise a head in several steps. Restoring in reverse
	* order therefore ends on the value the task had before the move.
	*/
	struct ArcUndo {
		int x, y, mpx, msy, mac;
		int old_mpx_ms, old_msy_mp, old_last;
		int old_y_mp, old_y_ms, old_x_mp, old_x_ms;
		std::vector<std::pair<int, FuzzyFW::Crisp>> heads;
	} undo;

	bool applyArc(const NeighbourIJSP_Arc *arc, const bool improvement);
	void revertArc();
};

} // namespace IJSP
