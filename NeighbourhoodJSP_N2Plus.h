/*
* NeighbourhoodJSP_N2Plus.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodJSP_N2.h"

namespace JSP {

//=============================================================================
//
//	Class NB_ParallelN2Plus_MakespanJSP
//
//=============================================================================
/**
* N2+ neighbourhood for Makespan minimization in JSP.
*
* Restricts N2 to boundary arcs of critical blocks from G+(sigma) only
* (the extreme graph using upper processing times p+).
* Connectivity is guaranteed by the Nowicki-Smutnicki (1996) result applied
* to G+ as a deterministic JSP instance.
*
* @author hdiaz
*/
class NB_ParallelN2Plus_MakespanJSP : public NB_ParallelN2_MakespanJSP {
public:
	NB_ParallelN2Plus_MakespanJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanJSP(parameters) { }

	NB_ParallelN2Plus_MakespanJSP(const NB_ParallelN2Plus_MakespanJSP & source)
		: NB_ParallelN2_MakespanJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2Plus_MakespanJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N2Plus", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);
};

}
