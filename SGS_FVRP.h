/*
* SGS_FVRP.h
*
*  Created on: October 31, 2017
*      Author: Juan Jose Palacios
*/

#pragma once

#include "RouteFVRP.h"
#include "SharedVars.h"
#include "Fitness.h"


namespace FVRP {

//=============================================================================
//
//	Abstract class SGS_FVRP
//
//=============================================================================
/**
* We define a SGS as an algorithm that is able to build a solution to the
* VRP from a given ordering of cutomers, which indicates a partial
* visiting order.
* There exists many different SGS algorithms, some of them being deterministic
* and many others having stochastic components. To ensure that results
* can be replicate, that means that the class may need acces to the RNG.
*
* @author Juan Jose Palacios
*
*/

class SGS_FVRP
{
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Partial plan built until the moment
	*/
	RouteFVRP * routePlan;

	/*
	* Flag indicating if the solution is initialized
	*/
	char isCreated;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FVRP(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FVRP(const SGS_FVRP &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FVRP * clone() const = 0;


	/*
	* Read the user parameters if needed
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params) { } // Nothing to load


	/*
	* Destructor
	*/
	virtual ~SGS_FVRP() {
		delete this->routePlan;	// The only thing to delete...
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the planification made until now
	*/
	RouteFVRP * getSchedule() const {
		return this->routePlan;
	}

	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const = 0;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of customers and creates a route to visit them
	* This method requires shared variables, as for instance, the problem
	*/
	virtual RouteFVRP * buildPlan(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order) = 0;


	/*
	* Clear all data structures to reuse the class
	*/
	virtual void reset();


protected:
	/*
	* Adds a specific customer into the planned route.
	* Returns the visit time assigned to the customer
	*/
	virtual FuzzyFW::TFN addCustomer(ProblemFVRP *problem,
		const unsigned int customerId,
		const unsigned int vehicle) = 0;
};






}
