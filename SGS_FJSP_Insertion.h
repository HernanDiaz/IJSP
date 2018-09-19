/*
* SGS_FJSP_Insertion.h
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#pragma once

#include "SGS_FJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class SGS_FJSP_Insertion
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

class SGS_FJSP_Insertion : public SGS_FJSP
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
	SGS_FJSP_Insertion(const FuzzyFW::ParameterDB *params = NULL)
		: SGS_FJSP(params) { }; // Nothing new here


	/*
	* Copy constructor
	*/
	SGS_FJSP_Insertion(const SGS_FJSP_Insertion &source)
		: SGS_FJSP(source) { }; // Here neither


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_FJSP * clone() const {
		return new SGS_FJSP_Insertion(*this);
	}


	/*
	* Destructor... not much to destroy though
	*/
	virtual ~SGS_FJSP_Insertion() { }



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
	virtual ScheduleFJSP * buildSchedule(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);


protected:
	/*
	* Looks for a space in the current schedule where the task can be
	* scheduled without delaying any other in any scenario. If there is no
	* such a space, schedules the new task at the end.
	*/
	virtual FuzzyFW::TFN scheduleTask(const TaskFJSP *task,
		const int taskIdx);
};

}
