/*
* NeighbourhoodIJSP_N2Minus.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_N2.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2Minus_MakespanIJSP
//
//=============================================================================
/**
* N2- neighbourhood for Makespan minimization in IJSP.
*
* Restricts N2 to boundary arcs of critical blocks from G-(sigma) only
* (the extreme graph using lower processing times p-).
* Connectivity is guaranteed by the Nowicki-Smutnicki (1996) result applied
* to G- as a deterministic JSP instance.
*
* @author hdiaz
*/
class NB_ParallelN2Minus_MakespanIJSP : public NB_ParallelN2_MakespanIJSP {
public:
	NB_ParallelN2Minus_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanIJSP(parameters) { }

	NB_ParallelN2Minus_MakespanIJSP(const NB_ParallelN2Minus_MakespanIJSP & source)
		: NB_ParallelN2_MakespanIJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2Minus_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N2Minus", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);
};

}
