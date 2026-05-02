/*
 * CreationIJSP_Manager.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "CreationIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class CreationManagerIntervalMkSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * Shortest Planned Job First schedule and then codifying it with the respective strategy
 *
 * @author hdiaz
 *
 */
class CreationManagerIntervalMkSchedule : public FuzzyFW::Creation {
	//=============================================================================
	//		COMMON FIELDS
	//=============================================================================
	const std::string sgsLabel;

	double randomRatio;

	/*
	* SGS to create schedules from task orderings
	*/
	std::unique_ptr<SGS_IJSP> sgs;


	CreationRandomSchedule randomSchedule;
	CreationSPJFIntervalMkSchedule SPJFSchedule;
	CreationLRTFIntervalMkSchedule LRTFSchedule;

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit CreationManagerIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: sgsLabel(CREATION_SGS), Creation(parameters), randomRatio(0) { }

	/**
	* Copy constructor
	*/
	CreationManagerIntervalMkSchedule(const CreationManagerIntervalMkSchedule &source)
		: Creation(source), sgsLabel(CREATION_SGS), randomRatio(source.randomRatio) { }

	/**
	* Loads the needed parameters: Read the minimum/maximum
	 * values that each gene may take
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Loads the needed parameters: Read the minimum/maximum
	* values that each gene may take
	*/
	virtual Creation * clone() const {
		return new CreationManagerIntervalMkSchedule(*this);
	}

	/**
	 * Destructor
	 */
	virtual ~CreationManagerIntervalMkSchedule() { } 	// Nothing to destroy here

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
		setup.push_back("Manager");
		setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
		setup.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}

};

}
