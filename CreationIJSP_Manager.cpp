/*
 * CreationIJSP_Manager.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_Manager.h"

namespace IJSP {

//=============================================================================
//
//	Class CreationManagerIntervalMkSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationManagerIntervalMkSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	CreationRandomSchedule::setup(parameters);
	this->SPJFSchedule.setup(parameters);
	this->LRTFSchedule.setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationManagerIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	int rand = svars->rng->getInteger(0, 100);
	if (rand <= 50)
		return CreationRandomSchedule::createIndividual(svars);
	if (rand % 2)
		return this->LRTFSchedule.createIndividual(svars);
	return this->SPJFSchedule.createIndividual(svars);
}

} // namespace IJSP
