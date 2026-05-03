/*
* NeighbourhoodIJSP_N3.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN3_MakespanIJSP
//
//=============================================================================
/**
* N3 neighbourhood for Makespan minimization in IJSP. Restricts N2 to arcs
* that are boundary in critical blocks on both extreme graphs simultaneously.
* Supports multi-arc moves (tipo 0-3).
*
* @author hdiaz
*/
class NB_ParallelN3_MakespanIJSP : public NB_ParallelBase_MakespanIJSP {
public:
	NB_ParallelN3_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanIJSP(parameters) { }

	NB_ParallelN3_MakespanIJSP(const NB_ParallelN3_MakespanIJSP &source)
		: NB_ParallelBase_MakespanIJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelN3_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N3", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	// N3 compacts the neighbour array on discard (needed during findNewNeighbours)
	virtual void discardNeighbour(const unsigned int idx);

	// N3 handles 4 move types; overrides the base tipo-0 implementation
	virtual void estimateHeadsTails(const unsigned int idx);

protected:
	void addNeighbour(const unsigned int x, const unsigned int y,
		const unsigned int z = 0, const unsigned int tipo = 0);
};

} // namespace IJSP
