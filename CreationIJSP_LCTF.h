/*
 * CreationIJSP_LCTF.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "CreationIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class CreationLCTFIntervalMkSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * Longest Completion Time First schedule and then codifying it with the respective strategy
 *
 * @author hdiaz
 *
 */
class CreationLCTFIntervalMkSchedule : public CreationRandomSchedule {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit CreationLCTFIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: CreationRandomSchedule(parameters) { }

	CreationLCTFIntervalMkSchedule(const CreationLCTFIntervalMkSchedule &source)
		: CreationRandomSchedule(source) { }

	virtual Creation * clone() const {
		return new CreationLCTFIntervalMkSchedule(*this);
	}

	virtual ~CreationLCTFIntervalMkSchedule() = default;

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
		setup.push_back("LCTF");
		setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
		setup.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}

};

}
