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
class CreationLRTFInverseIntervalMkSchedule : public FuzzyFW::Creation {
protected:
	//=============================================================================
	//		COMMON FIELDS
	//=============================================================================
	/*
	* Label to identify the SGS type
	*/
	const std::string sgsLabel;

	double randomRatio;

	/*
	* SGS to create schedules from task orderings
	*/
	std::unique_ptr<SGS_IJSP> sgs;


	CreationRandomSchedule randomSchedule;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit CreationLRTFInverseIntervalMkSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: sgsLabel(CREATION_SGS), Creation(parameters), randomRatio(0) { }

	/**
	* Copy constructor
	*/
	CreationLRTFInverseIntervalMkSchedule(const CreationLRTFInverseIntervalMkSchedule &source)
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
		return new CreationLRTFInverseIntervalMkSchedule(*this);
	}

	/**
	 * Destructor
	 */
	virtual ~CreationLRTFInverseIntervalMkSchedule() { } 	// Nothing to destroy here


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
