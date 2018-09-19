/*
* FuzzySchedule.h
*
*  Created on: May 16, 2017
*      Author: Juan Jose Palacios
*/

#ifndef FJSPPROBLEM_FUZZYSCHEDULE_H_
#define FJSPPROBLEM_FUZZYSCHEDULE_H_

#include "FuzzyProblem.h"
#include "Solution.h"


namespace FJSP {

//=========================================================================
//
//	Struct TaskInfo
//
//=========================================================================
/**
* This is an auxiliary struct containing all data about tasks that it
* isn't constant. That is, all information that may vary from one
* solution to another.
*
* @author Juan Jose Palacios
*/
struct ScheduledTaskInfo {
	const FuzzyTask *task;	// Scheduled task
	TFN head;		// Starting time (head)
	TFN tail;		// Minimum remaining time after the task
	int mp;		// Machine predecessor
	int ms;		// Machine successor
};





//=========================================================================
//
//	Class FuzzySchedule
//
//=========================================================================
/**
* Objects of this class represent a [partial] solution to a FJSP. We
* consider a partial solution as an assignment of starting times to a
* set of tasks in such a way that all problem constraints hold. There are
* other elements and values in a schedule that are computed repeatedly,
* as the topological order, the tail of the tasks, predecessors and
* successors, etc... To avoid redundand calculations, those common values
* are stored as well within the class.
*
* The class stores all the tasks in an array, conserving the positions that
* they have in the Problem class to improve the access to them. When a
* task is added to the schedule, it's dynamic information is updated.
*
* @author Juan Jose Palacios
*
*/
class FuzzySchedule : public Solution
{
	//=====================================================================
	//		FIELDS
	//=====================================================================
public:
	/**
	* Dynamic information of already scheduled tasks
	*/
	std::vector<ScheduledTaskInfo> taskInfo;

	/**
	* Last scheduled task in each machine/job. It is public so methods
	* such as local search can easily update it
	*/
	std::vector<int> lastTaskMachine;
	std::vector<int> lastTaskJob;


protected:
	/**
	* Number of scheduled tasks
	*/
	unsigned int nScheduledTasks;

	/**
	* Topological order of tasks
	*/
	std::vector<int> taskOrder;

	/**
	* Problem that this schedule solves
	*/
	const FuzzyProblem *problem;


	/**
	* Various flags
	*/
	char tailsUpdated;
	char isSorted;



	//=====================================================================
	//		CONSTRUCTORS
	//=====================================================================
public:
	/**
	* Main constructor
	*/
	FuzzySchedule(const FuzzyProblem * problem = NULL);

	/**
	* Copy constructor
	*/
	FuzzySchedule(const FuzzySchedule & source);

	/**
	* Destructor
	*/
	virtual ~FuzzySchedule() { }


	/**
	* Clone methods (like in Java)
	*/
	virtual Solution * clone() const {
		return new FuzzySchedule(*this);
	}



	//=====================================================================
	//		GET/SET METHODS
	//=====================================================================
public:
	/**
	* Get the number of already scheduled tasks
	*/
	unsigned int getScheduledTasks() const {
		return this->nScheduledTasks;
	}

	/**
	* Get the topoligal order of the tasks using a given comparison
	* system
	*/
	std::vector<int> & getTaskOrder();

	/**
	* Get the completion time of a machine
	*/
	TFN getCTMachine(const unsigned int machine) const;

	/**
	* Get the completion time of a job
	*/
	TFN getCTJob(const unsigned int job) const;



	//=====================================================================
	//		OPERATORS
	//=====================================================================
public:
	/**
	* Assignment overload
	*/
	FuzzySchedule & operator=(const FuzzySchedule & source);

	/**
	* Index access. Makes faster to code the access to the static
	* information of the scheduled tasks. The dynamic information
	* can be accessed through the public field itself
	*/
	const FuzzyTask * operator[](const int index) const;

	/**
	* Converts the solution to a string
	*/
	virtual std::string toString() const {
		std::string str;
		if (this->taskOrder.size() <= 0)
			return "";
		str = valueToString(this->taskOrder[0]);
		for (size_t i = 0; i < this->taskOrder.size(); i++)
			str += " " + valueToString(this->taskOrder[i]);
		return str;
	}



	//=====================================================================
	//		METHODS
	//=====================================================================
public:
	/**
	* Add a new task to the schedule
	*/
	void addTask(const int taskIdx, TFN & ST, const int macSuc);

	/**
	* Update the heads and tails for future use. It requires the type
	* of maximum to use during the computations
	*/
	void updateTails(const TFN::Maximum maxType);

	/**
	* Clears the schedule
	*/
	void reset();


protected:
	/**
	* Updates the topological order of the tasks
	*/
	void updateTopologicalOrder();

	/**
	* Compute the topological order using a quuicksort algorithm
	*/
	void quicksortTasks(int left, int right, Random *rng);
};

}

#endif /* FJSPPROBLEM_FUZZYSCHEDULE_H_ */
