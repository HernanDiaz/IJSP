/*
 * CreationIJSP_SPJFInverse.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "CreationIJSP_Base.h"


namespace IJSP {

//=============================================================================
//
//	Class CreationSPJFInverseIntervalMkSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * Shortest Planned Job First Inverse schedule and then codifying it with the respective strategy
 * (gives priority to the jobs with the longest makespan)
 * @author hdiaz
 *
 */
class CreationSPJFInverseIntervalMkSchedule : public CreationRandomSchedule {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit CreationSPJFInverseIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: CreationRandomSchedule(parameters) { }

	CreationSPJFInverseIntervalMkSchedule(const CreationSPJFInverseIntervalMkSchedule &source)
		: CreationRandomSchedule(source) { }

	virtual Creation * clone() const {
		return new CreationSPJFInverseIntervalMkSchedule(*this);
	}

	virtual ~CreationSPJFInverseIntervalMkSchedule() = default;

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
		setup.push_back("SPJFInverse");
		setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
		setup.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}

};

}
