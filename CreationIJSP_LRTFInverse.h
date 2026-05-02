/*
 * CreationIJSP_LRTFInverse.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "CreationIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class CreationLRTFInverseIntervalMkSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * Longest Remaining Time First Inverse schedule and then codifying it with the respective strategy
 *(gives priority to the shortest remaining)
 * @author hdiaz
 *
 */
class CreationLRTFInverseIntervalMkSchedule : public CreationRandomSchedule {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit CreationLRTFInverseIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: CreationRandomSchedule(parameters) { }

	CreationLRTFInverseIntervalMkSchedule(const CreationLRTFInverseIntervalMkSchedule &source)
		: CreationRandomSchedule(source) { }

	virtual Creation * clone() const {
		return new CreationLRTFInverseIntervalMkSchedule(*this);
	}

	virtual ~CreationLRTFInverseIntervalMkSchedule() = default;


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Creates an individual
	*
	* @param individualType Type of individuals to create
	* @param svars Shared variables for the algorithm
	* @return A new born individual
	*/
	virtual FuzzyFW::Individual * createIndividual(
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		std::vector<std::string> sgsName = this->sgs->getName();
		setup.push_back("LRTFInverse");
		setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
		setup.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}
};

}
