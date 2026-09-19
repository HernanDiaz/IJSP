/*
* SGS_JSP.h
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#pragma once

#include "ScheduleJSP.h"
#include "SharedVars.h"


namespace JSP {

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
* @author hdiaz
*
*/

class SGS_JSP
{
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Partial schedule built until the moment
	*/
	ScheduleJSP * schedule;

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
	SGS_JSP(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_JSP(const SGS_JSP &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_JSP * clone() const = 0;


	/*
	* Read the user parameters if needed.
	*
	* There is nothing left to read. The SGS used to select a strategy for
	* comparing intervals through sgs.interval.comparison; on crisp times there
	* is one order, so the setting selected the only option there is. Setups
	* that still carry the key load unchanged and it is ignored.
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params) { }


	/*
	* Destructor
	*/
	virtual ~SGS_JSP() {
		delete this->schedule;	// The only thing to delete...
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the schedule available until now
	*/
	ScheduleJSP * getSchedule() const {
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
	virtual ScheduleJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);

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
	virtual FuzzyFW::Crisp scheduleTask(const TaskJSP *task,
		const int taskIdx) = 0;

	/*
	* Hook called at the end of buildSchedule(). Override to add
	* post-construction validation or repair (e.g. verifyScheduling).
	*/
	virtual void postBuild() { }
};






}
