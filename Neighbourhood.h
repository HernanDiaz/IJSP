/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "SharedVars.h"
#include "Neighbour.h"


namespace FuzzyFW {

// Creation parameters defined in this header file
#define NEIGHBOURHOOD_ESTIMATOR "neighbourhood.estimator"
	

//=============================================================================
//
//	Abstract class Neighbourhood
//
//=============================================================================
/**
* This class defines the neighbourhood structure that is needed for any
* local search algorithm. It contains both the neighbours and the methods
* that allow the local search to accept a solution, or discard.
* It also may contain different estimation functions to approximate the
* quality of the neighbours before fully evaluating them
*
* @author jjpalacios
*
*/
class Neighbourhood {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Current number of neighbours
	*/
	unsigned int numNeighbours;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	explicit Neighbourhood(ParameterDB *parameters = NULL)
		: numNeighbours(0) {
		if (parameters != NULL)
			this->setup(parameters);
	}


	/*
	* Copy constructor
	*/
	Neighbourhood(const Neighbourhood & source)
		: numNeighbours(source.numNeighbours) { }


	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters) { }


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const = 0;


	/*
	* Destructor
	*/
	~Neighbourhood() { };



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Neighbourhood size
	*/
	unsigned int size() const {
		return this->numNeighbours;
	}


	/*
	* Gets the current solution
	*/
	virtual FullSolution getCurrentSolution() = 0;


	/*
	* Name of the Neighbourhood structure
	*/
	virtual std::vector<std::string> getName() = 0;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Set initial solution to create the neighbourhood from
	*/
	virtual void setInitialSolution(Solution *solution, Fitness *fitness,
		const SharedVars *svars) = 0;


	/*
	* Find the neighbours of the given solution
	* Returns the number of neighbours found
	*/
	virtual unsigned int findNewNeighbours(const SharedVars *svars) = 0;

	/*
	* Generates the new solution using the one given before as base
	*/
	virtual Fitness * evaluateNeighbour(const unsigned int idx,
		const SharedVars *svars, const bool improvement = false) = 0;

	/*
	* Accept a neighbour and moves towards it
	*/
	virtual void acceptNeighbour(const unsigned int idx,
		const SharedVars *svars) = 0;


	/*
	* If an estimator is used, an estimation can be used
	*/
	virtual Fitness * getEstimation(const unsigned int idx,
		const SharedVars *svars) = 0;

	/*
	* Sort all neighbours by their estimation
	*/
	virtual void sortByEstimation(const SharedVars *svars) = 0;

	/*
	* Discard a specific neighbour. This deletes all the structures
	* created for that neighbour. That is: estimation and evaluation
	*/
	virtual void discardNeighbour(const unsigned int idx) = 0;


	/*
	* Gets a neighbour from the neighbourhood
	*/
	virtual Neighbour* getNeighbour(const unsigned int idx) = 0;
};

}

