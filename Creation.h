/*
 * Creation.h
 *
 *  Created on: Aug 4, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_ECOPERATORS_CREATION_H_
#define SRC_ECOPERATORS_CREATION_H_

#include "Population.h"
#include "SchedulingClassRegister.h"
#include "Encoder.h"

namespace FJSP {

// Creation parameters defined in this header file
#define CREATION_SGS "creation.sgs"


//=============================================================================
//
//	Abstract class Creation
//
//=============================================================================
/**
 * This class provides the framework to implement different strategies
 * to create initial solutions to a problem, and therefore populations
 *
 * @author jjpalacios
 *
 */
class Creation {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Creation(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Creation(const Creation &source) { }

	/**
	 * Loads the needed parameters: No parameters needed
	 */
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method for inheriting classes
	*/
	virtual Creation * clone() const = 0;

	/**
	 * Destructor
	 */
	virtual ~Creation() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Creates an initial population using the given individual type and the
	 * size of the population
	 *
	 * @param individualType Type of individuals to create
	 * @param size Number of individuals to create
	 * @param svars Shared variables for the algorithm
	 * @return A bunch of filthy individuals
	 */
	virtual Population * createPopulation(const unsigned int popSize,
		const SharedVars *svars) const;

	/**
	 * Creates an individual
	 *
	 * @param individualType Type of individuals to create
	 * @param svars Shared variables for the algorithm
	 * @return A new born individual
	 */
	virtual Individual * createIndividual(const SharedVars *svars) const=0;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;

};





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
class CreationRandomSchedule : public Creation {
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
	FuzzySGS * sgs;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit CreationRandomSchedule(ParameterDB *parameters = NULL)
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
	virtual void setup(ParameterDB *parameters);

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
	virtual Individual * createIndividual(const SharedVars *svars) const;

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
		for (size_t i = 0; i < sgsName.size(); i++)
			setup.push_back(";" + sgsName[i]);
		return setup;
	}
};


}


#endif /* SRC_ECOPERATORS_CREATION_H_ */
