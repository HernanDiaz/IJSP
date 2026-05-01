/*
* SGS_FJSP.h
*
*  Created on: June 1, 2017
*/

#pragma once

#include "ScheduleFJSP.h"
#include "SharedVars.h"


namespace FJSP {

//=============================================================================
//
//	Abstract class FuzzySGS
//
//=============================================================================
/**
* We define a SGS as an algorithm that is able to build a schedule
* from a given ordering of tasks, which indicates priority among them
* There exists many different SGS algorithms, some of them being deterministic
* and many others having stochastic components. To ensure that results
* can be replicate, that means that the class will need acces to the RNG.
*
*
*/

class SGS_FJSP
{
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Partial schedule built until the moment
	*/
	ScheduleFJSP * schedule;

	/*
	* Flag indicating if the schedule is initialized
	*/
	char isCreated;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FJSP(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FJSP(const SGS_FJSP &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const = 0;


	/*
	* Read the user parameters if needed
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params) { } // Nothing to load


	/*
	* Destructor
	*/
	virtual ~SGS_FJSP() {
		delete this->schedule;	// The only thing to delete...
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the schedule available until now
	*/
	ScheduleFJSP * getSchedule() const {
		return this->schedule;
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
	* Takes an order of operations and creates a schedule from them.
	* This method requires shared variables, as for instance, the problem
	*/
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order) = 0;


	/*
	* Clear all data structures to reuse the class
	*/
	virtual void reset();


protected:
	/*
	* Schedules a specific task in the current schedule. There are different
	* strategies to do this, so this method must be adapted for each SGS.
	* Returns the starting time assigned to the operation
	*/
	virtual FuzzyFW::TFN scheduleTask(const TaskFJSP *task,
		const int taskIdx) = 0;
};






}
