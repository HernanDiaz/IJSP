/*
* ScheduleFJSP.h
*
*  Created on: May 16, 2017
*      Author: Juan Jose Palacios
*/
#pragma once

#include "ProblemIJSP.h"
#include "Solution.h"


namespace IJSP {


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
	const TaskIJSP *task;	// Scheduled task
	FuzzyFW::TFN head;		// Starting time (head)
	int mp;		// Machine predecessor
	int ms;		// Machine successor
};





//=========================================================================
//
//	Class ScheduleFJSP
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
class ScheduleIJSP : public FuzzyFW::Solution
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
	const ProblemIJSP *problem;


	/**
	* Various flags
	*/
	char isSorted;



	//=====================================================================
	//		CONSTRUCTORS
	//=====================================================================
public:
	/**
	* Main constructor
	*/
	ScheduleIJSP(const ProblemIJSP * problem = NULL);

	/**
	* Copy constructor
	*/
	ScheduleIJSP(const ScheduleIJSP & source);

	/**
	* Destructor
	*/
	virtual ~ScheduleIJSP() {	}


	/**
	* Clone methods (like in Java)
	*/
	virtual FuzzyFW::Solution * clone() const {
		return new ScheduleIJSP(*this);
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
	std::vector<int> & getTaskOrder(FuzzyFW::Random *rng = NULL);

	/**
	* Get the completion time of a machine
	*/
	FuzzyFW::TFN getCTMachine(const unsigned int machine) const;

	/**
	* Get the completion time of a job
	*/
	FuzzyFW::TFN getCTJob(const unsigned int job) const;


	/**
	* Get the completion time of a job
	*/
	void setSorted(bool flag) {
		this->isSorted = flag;
	}



	//=====================================================================
	//		OPERATORS
	//=====================================================================
public:
	/**
	* Assignment overload
	*/
	ScheduleIJSP & operator=(const ScheduleIJSP & source);

	/**
	* Index access. Makes faster to code the access to the static
	* information of the scheduled tasks. The dynamic information
	* can be accessed through the public field itself
	*/
	const TaskIJSP * operator[](const unsigned int index) const;

	/**
	* Converts the solution to a string
	*/
	virtual std::string toString() const {
		std::string str;
		if (this->taskOrder.size() <= 0)
			return "";
		str = valueToString(this->taskOrder[0]);
		for (size_t i = 1; i < this->taskOrder.size(); i++)
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
	void addTask(const int taskIdx, FuzzyFW::TFN & ST, const int macSuc);

	/**
	* Clears the schedule
	*/
	void reset();


protected:
	/**
	* Updates the topological order of the tasks
	*/
	void updateTopologicalOrder(FuzzyFW::Random *rng = NULL);

	/**
	* Compute the topological order using a quuicksort algorithm
	*/
	void quicksortTasks(int left, int right, FuzzyFW::Random *rng);
};

}
