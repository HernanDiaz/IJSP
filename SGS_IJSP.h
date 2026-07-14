/*
* SGS_IJSP.h
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#pragma once

#include "ScheduleIJSP.h"
#include "SharedVars.h"


namespace IJSP {

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

/*
* Parameters:
*	How to compare objective functions that are Intervals
*/
#define IJSP_SGS_COMPARE "sgs.interval.comparison"

class SGS_IJSP
{
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Partial schedule built until the moment
	*/
	ScheduleIJSP * schedule;

	/*
	* Flag indicating if the schedule is initialized
	*/
	char isCreated;

	/*
	* Label to indicate the operator to compare intervals
	*/
	const std::string compareLabel;

	/*
	* Operator used to compare intervals in the SGS (component by default)
	*/
	FuzzyFW::Interval::Compare cpComp;

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_IJSP(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_IJSP(const SGS_IJSP &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_IJSP * clone() const = 0;


	/*
	* Read the user parameters if needed
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params) { 
		// Load comparison strategy parameter
		std::string compareName = params->getString(this->compareLabel);
		
		if (compareName.length() == 0) {
			std::string errorMsg = this->compareLabel + " parameter not found.";
			throw IJSPException("SGS_IJSP", errorMsg);
		}
		this->cpComp = FuzzyFW::Interval::getComparison(compareName);
		if (this->cpComp == FuzzyFW::Interval::C_Err) {
			std::string errorMsg = "Invalid value for parameter ";
			errorMsg += "\'" + this->compareLabel + "\': \'";
			errorMsg += compareName + "\'";
			throw IJSPException("SGS", errorMsg);
		}
	
	}


	/*
	* Destructor
	*/
	virtual ~SGS_IJSP() {
		delete this->schedule;	// The only thing to delete...
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the schedule available until now
	*/
	ScheduleIJSP * getSchedule() const {
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
	virtual ScheduleIJSP * buildSchedule(
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
	virtual FuzzyFW::Interval scheduleTask(const TaskIJSP *task,
		const int taskIdx) = 0;

	/*
	* Hook called at the end of buildSchedule(). Override to add
	* post-construction validation or repair (e.g. verifyScheduling).
	*/
	virtual void postBuild() { }
};






}
