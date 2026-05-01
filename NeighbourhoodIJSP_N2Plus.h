/*
* NeighbourhoodIJSP_N2Plus.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_N2.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2Plus_MakespanIJSP
//
//=============================================================================
/**
* N2+ neighbourhood for Makespan minimization in IJSP.
*
* Restricts N2 to boundary arcs of critical blocks from G+(sigma) only
* (the extreme graph using upper processing times p+).
* Connectivity is guaranteed by the Nowicki-Smutnicki (1996) result applied
* to G+ as a deterministic JSP instance.
*
* @author hdiaz
*/
class NB_ParallelN2Plus_MakespanIJSP : public NB_ParallelN2_MakespanIJSP {
public:
	NB_ParallelN2Plus_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanIJSP(parameters) { }

	NB_ParallelN2Plus_MakespanIJSP(const NB_ParallelN2Plus_MakespanIJSP & source)
		: NB_ParallelN2_MakespanIJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2Plus_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-N2Plus");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
		setup.push_back(value);
		return setup;
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);
};

}
