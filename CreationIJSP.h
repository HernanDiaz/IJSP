/*
 * Creation.h
 *
 *  Created on: Aug 4, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Creation.h"
#include "FJSPClassRegister.h"
#include "EncoderFJSP.h"

#include "ProblemIJSP.h"
#include "IJSPException.h"
#include "SGS_IJSP.h"

namespace IJSP {

// Creation parameters defined in this header file
#define CREATION_SGS "creation.sgs"


//=============================================================================
//
//	Class CreationRandomSchedule
//
//=============================================================================
/**
 * This class generates an initial individual/population by creating a
 * random schedule and then codifying it with the respective strategy
 *
 * @author jjpalacios
 *
 */
class CreationRandomSchedule : public FuzzyFW::Creation {
protected:
	//=============================================================================
	//		COMMON FIELDS
	//=============================================================================
	/*
	* Label to identify the SGS type
	*/
	const std::string sgsLabel;

	/*
	* SGS to create schedules from task orderings
	*/
	SGS_IJSP * sgs;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit CreationRandomSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: sgsLabel(CREATION_SGS),sgs(NULL), Creation(parameters) { }

	/**
	* Copy constructor
	*/
	CreationRandomSchedule(const CreationRandomSchedule &source)
		: Creation(source), sgsLabel(CREATION_SGS), sgs(NULL) { }

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
		return new CreationRandomSchedule(*this);
	}

	/**
	 * Destructor
	 */
	virtual ~CreationRandomSchedule() { } 	// Nothing to destroy here


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
		setup.push_back("Random");
		setup.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}
};

}
