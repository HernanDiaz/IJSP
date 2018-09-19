/*
* SGS_FJSP_Append.h
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#pragma once

#include "SGS_FJSP.h"

namespace FJSP {

/*
* Parameters:
*	Maximum type to use in Append SGS 
*	Comparison method to compare starting times
*	Delta parameter for G&T based SGS 
*/
#define FJSP_SGS_APPEND_MAXIMUM "sgs.append.maximum" // For Append SGS
#define FJSP_SGS_DENSE_COMPARISON "sgs.dense.compare" // For Dense SGS
#define FJSP_SGS_GYT_DELTA "sgs.gyt.delta" // For Dense SGS



//=============================================================================
//
//	Class SGS_FJSP_Append
//
//=============================================================================
/**
* An Append SGS is the simplest one. It schedules the tasks sequentailly
* following the given task order. Each task is scheduled immediately
* after its job and machine predecessors. 
* To do that, the algorithm will need to compute the maximum between the 
* completion time of those tasks, thus its behaviour may change depending
* on how that maximum is computed. This strategy is a parameter of the
* algorithm.
* In its simplest case, this SGS will produce semi-active schedules.
*
* @author Juan Jose Palacios
*
*/

class SGS_FJSP_Append : public SGS_FJSP
{
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Name of the parameter with the type of maximum to use
	*/
	const std::string maximumLabel;

	/*
	* Type of maximum to use during the scheduling process
	*/
	FuzzyFW::TFN::Maximum tfnMaximum;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FJSP_Append(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FJSP_Append(const SGS_FJSP_Append &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const {
		return new SGS_FJSP_Append(*this);
	}


	/*
	* Loads the Maximum strategy to follow
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~SGS_FJSP_Append() { }  // No pointers, no destruction



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Append");
		setup.push_back(";Maximum;"+
			FuzzyFW::TFN::getMaximum(this->tfnMaximum));
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and creates a schedule from them by
	* appending them at "the end" of the schedule. The method requires the
	* shared variables to have access to the problem to solve
	*/
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);



protected:
	/*
	* Schedules a specific task just after the last task scheduled in its
	* required machine and the last task of its job
	*/
	virtual FuzzyFW::TFN scheduleTask(const TaskFJSP *task,
		const int taskIdx);
};





//=============================================================================
//
//	Class SGS_FJSP_Dense
//
//=============================================================================
/**
* A schedule is dense if there is no idle time when a machine is available
* to be scheduled there. This SGS generates that type of schedules, making
* sure that each time a machine is idle, a task is assigned to it (if 
* possible). This is the same as scheduling everytime the operation that can 
* start earlier. In case of a tie, the task ordering is used.
*
* @author Juan Jose Palacios
*
*/

class SGS_FJSP_Dense : public SGS_FJSP_Append
{
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Name of the parameter with the comparison method to use
	*/
	const std::string compareLabel;

	/*
	* Method of comparison to use during the scheduling process
	*/
	FuzzyFW::TFN::Compare tfnCompare;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FJSP_Dense(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FJSP_Dense(const SGS_FJSP_Dense &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const {
		return new SGS_FJSP_Dense(*this);
	}


	/*
	* Load the Comparison parameter
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~SGS_FJSP_Dense() { }  // No pointers, no destruction



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Dense");
		setup.push_back(";Maximum;" +
			FuzzyFW::TFN::getMaximum(this->tfnMaximum));
		setup.push_back(";Comparisons;" +
			FuzzyFW::TFN::getComparison(this->tfnCompare));
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and creates a schedule from them by
	* appending them at "the end" of the schedule. The method requires the
	* shared variables to have access to the problem to solve
	*/
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);
};





//=============================================================================
//
//	Class SGS_FJSP_fGYT1
//
//=============================================================================
/**
* This SGS is based on the well-known G&T algorithm for generating active
* schedules in crisp Job Shop problems. The details on how this adaptation is
* done are explained in the following paper:
*
* "Schedule generation schemes for job shop problems with fuzziness.
* JJ Palacios, CR Vela, I González-Rodríguez, J Puente - Proceedings of the
* Twenty-first European Conference on Artificial Intelligence, 687-692,
* 2014"
*
* It calculates the earliest completion time for every non scheduled task and
* takes the conflict set as the tasks which Starting Time has at least
* one component lesser than a component of the ECT
*
* @author Juan Jose Palacios
*
*/

class SGS_FJSP_fGYT1 : public SGS_FJSP_Append
{
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Name of the parameter with the delta value
	*/
	const std::string deltaLabel;

	/*
	* Delta value
	*/
	double delta;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FJSP_fGYT1(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FJSP_fGYT1(const SGS_FJSP_fGYT1 &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const {
		return new SGS_FJSP_fGYT1(*this);
	}


	/*
	* This class does not need any parameter. 
	* Overrides the method to not read the maximim parameter
	* neither, since it is fixed
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor.... nope
	*/
	virtual ~SGS_FJSP_fGYT1() { }



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("fG&T-1");
		setup.push_back(";Delta:;" + valueToString(this->delta));
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and cuses it as a strategy to choose an
	* operation in the conflict set. It can be considered as a strategy
	* to solve draws
	*/
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);
};





//=============================================================================
//
//	Class SGS_FJSP_fGYT2
//
//=============================================================================
/**
* This SGS is based on the well-known G&T algorithm for generating active
* schedules in crisp Job Shop problems. The details on how this adaptation is
* done are explained in the following paper:
*
* "Schedule generation schemes for job shop problems with fuzziness.
* JJ Palacios, CR Vela, I González-Rodríguez, J Puente - Proceedings of the
* Twenty-first European Conference on Artificial Intelligence, 687-692,
* 2014"
*
* It calculates the earliest completion time for every non scheduled task and
* takes both the minimum and the operations that donates a component to that
* minimum. The conflict set contains the tasks which Starting Time has at least
* one component lesser than the Completion times of all tasks contributing to
* calculate the minimum C*
*
* @author Juan Jose Palacios
*
*/

class SGS_FJSP_fGYT2 : public SGS_FJSP_Append
{
	//=========================================================================
	//		FIELDS
	//=========================================================================

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_FJSP_fGYT2(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_FJSP_fGYT2(const SGS_FJSP_fGYT2 &source)
		: SGS_FJSP_Append(source) { };


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const {
		return new SGS_FJSP_fGYT2(*this);
	}


	/*
	* This class does not need any parameter.
	* Overrides the method to not read the maximim parameter
	* neither, since it is fixed
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor.... nope
	*/
	virtual ~SGS_FJSP_fGYT2() { }



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("fG&T-2");
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and cuses it as a strategy to choose an
	* operation in the conflict set. It can be considered as a strategy
	* to solve draws
	*/
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);
};


}
