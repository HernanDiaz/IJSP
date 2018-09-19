/*
* FuzzySGS.h
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/
#ifndef FJSPPROBLEM_FUZZYSGS_H_
#define FJSPPROBLEM_FUZZYSGS_H_

#include "FuzzySchedule.h"
#include "SharedVars.h"


namespace FJSP {

/*
* Parameters:
*	Maximum type to use in Append SGS 
*	Comparison method to compare starting times
*	Delta parameter for G&T based SGS 
*/
#define SGS_APPEND_MAXIMUM "sgs.append.maximum" // For Append SGS
#define SGS_DENSE_COMPARISON "sgs.dense.compare" // For Dense SGS
#define SGS_GYT_DELTA "sgs.gyt.delta" // For Dense SGS





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
* @author Juan Jose Palacios
*
*/

class FuzzySGS
{
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Partial schedule built until the moment
	*/
	FuzzySchedule * schedule;

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
	FuzzySGS(const ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	FuzzySGS(const FuzzySGS &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const = 0;


	/*
	* Read the user parameters if needed
	*/
	virtual void setup(const ParameterDB *params) { } // Nothing to load


	/*
	* Destructor
	*/
	virtual ~FuzzySGS() {
		delete this->schedule;	// The only thing to delete...
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the schedule available until now
	*/
	FuzzySchedule * getSchedule() const {
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
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order) = 0;


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
	virtual TFN scheduleTask(const FuzzyTask *task,
		const int taskIdx) = 0;
};





//=============================================================================
//
//	Class FuzzySGS_Insertion
//
//=============================================================================
/**
* The insertion SGS is introduced and detailed in the following paper:
* "Schedule generation schemes for job shop problems with fuzziness.
* JJ Palacios, CR Vela, I González-Rodríguez, J Puente - Proceedings of the
* Twenty-first European Conference on Artificial Intelligence, 687-692,
* 2014"
*
* In brief, it uses an insertion strategy to find holes in the schedule
* that can be used to introduce the new task with no possibility of
* delaying any other.
*
* @author Juan Jose Palacios
*
*/

class FuzzySGS_Insertion : public FuzzySGS
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
	FuzzySGS_Insertion(const ParameterDB *params = NULL)
		: FuzzySGS(params) { }; // Nothing new here


	/*
	* Copy constructor
	*/
	FuzzySGS_Insertion(const FuzzySGS_Insertion &source)
		: FuzzySGS(source) { }; // Here neither


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const {
		return new FuzzySGS_Insertion(*this);
	}


	/*
	* Destructor... not much to destroy though
	*/
	virtual ~FuzzySGS_Insertion() { } 



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Insertion");
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* The tasks are scheduled following the exact same order given by
	* the array.
	*/
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order);


protected:
	/*
	* Looks for a space in the current schedule where the task can be
	* scheduled without delaying any other in any scenario. If there is no
	* such a space, schedules the new task at the end.
	*/
	virtual TFN scheduleTask(const FuzzyTask *task,
		const int taskIdx);
};





//=============================================================================
//
//	Class FuzzySGS_Append
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

class FuzzySGS_Append : public FuzzySGS
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
	TFN::Maximum tfnMaximum;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FuzzySGS_Append(const ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	FuzzySGS_Append(const FuzzySGS_Append &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const {
		return new FuzzySGS_Append(*this);
	}


	/*
	* Loads the Maximum strategy to follow
	*/
	virtual void setup(const ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~FuzzySGS_Append() { }  // No pointers, no destruction



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
		setup.push_back(";Maximum;"+TFN::getMaximum(this->tfnMaximum));
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
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order);



protected:
	/*
	* Schedules a specific task just after the last task scheduled in its
	* required machine and the last task of its job
	*/
	virtual TFN scheduleTask(const FuzzyTask *task,
		const int taskIdx);
};





//=============================================================================
//
//	Class FuzzySGS_Dense
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

class FuzzySGS_Dense : public FuzzySGS_Append
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
	TFN::Compare tfnCompare;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FuzzySGS_Dense(const ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	FuzzySGS_Dense(const FuzzySGS_Dense &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const {
		return new FuzzySGS_Dense(*this);
	}


	/*
	* Load the Comparison parameter
	*/
	virtual void setup(const ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~FuzzySGS_Dense() { }  // No pointers, no destruction



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
		setup.push_back(";Maximum;" + TFN::getMaximum(this->tfnMaximum));
		setup.push_back(";Comparisons;" + TFN::getComparison(this->tfnCompare));
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
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order);
};





//=============================================================================
//
//	Class FuzzySGS_fGYT1
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

class FuzzySGS_fGYT1 : public FuzzySGS_Append
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
	FuzzySGS_fGYT1(const ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	FuzzySGS_fGYT1(const FuzzySGS_fGYT1 &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const {
		return new FuzzySGS_fGYT1(*this);
	}


	/*
	* This class does not need any parameter. 
	* Overrides the method to not read the maximim parameter
	* neither, since it is fixed
	*/
	virtual void setup(const ParameterDB *params);


	/*
	* Destructor.... nope
	*/
	virtual ~FuzzySGS_fGYT1() { } 



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
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order);
};





//=============================================================================
//
//	Class FuzzySGS_fGYT2
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

class FuzzySGS_fGYT2 : public FuzzySGS_Append
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
	FuzzySGS_fGYT2(const ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	FuzzySGS_fGYT2(const FuzzySGS_fGYT2 &source)
		: FuzzySGS_Append(source) { };


	/*
	* Clone method to replicate inherited instances
	*/
	virtual FuzzySGS * clone() const {
		return new FuzzySGS_fGYT2(*this);
	}


	/*
	* This class does not need any parameter.
	* Overrides the method to not read the maximim parameter
	* neither, since it is fixed
	*/
	virtual void setup(const ParameterDB *params);


	/*
	* Destructor.... nope
	*/
	virtual ~FuzzySGS_fGYT2() { }



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
	virtual FuzzySchedule * buildSchedule(const SharedVars * const svars,
		std::vector<int> &order);
};


}

#endif /* FJSPPROBLEM_FUZZYSGS_H_ */

