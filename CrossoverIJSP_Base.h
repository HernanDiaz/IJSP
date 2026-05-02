/*
 * CrossoverIJSP_Base.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "EncoderIJSP.h"
#include "Crossover.h"
#include "IJSPException.h"
#include "Individual.h"


namespace IJSP {

//=============================================================================
//
//	Class CrossoverIJSP_Base
//
//=============================================================================
/**
 * Abstract base class for all IJSP crossover operators.
 * Provides the encoding-type dispatch in apply(), delegating to the
 * appropriate pure-virtual applyPermutation() or applyJobPermutation()
 * depending on whether svars->encoder is an Order or JobOrder encoder.
 */
class CrossoverIJSP_Base : public FuzzyFW::Crossover {
public:
	CrossoverIJSP_Base(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) {}

	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual ~CrossoverIJSP_Base() {}

protected:
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const = 0;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const = 0;
};

} // namespace IJSP
