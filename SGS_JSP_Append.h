/*
* SGS_JSP_Append.h
*
*  Created on: Jul 2, 2019
*      Author: Hernan Diaz Rodriguez
*/

#pragma once

#include "SGS_JSP.h"

namespace JSP {

/*
* Parameters:
*	Maximum type to use in Append SGS 
*	Comparison method to compare starting times
*	Delta parameter for G&T based SGS 
*/
#define JSP_SGS_APPEND_MAXIMUM "sgs.append.maximum" // For Append SGS
#define JSP_SGS_DENSE_COMPARISON "sgs.dense.compare" // For Dense SGS
#define JSP_SGS_GYT_DELTA "sgs.gyt.delta" // For Dense SGS



//=============================================================================
//
//	Class SGS_JSP_Append
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
* @author hdiaz
*
*/

class SGS_JSP_Append : public SGS_JSP
{
	//=========================================================================
	//		FIELDS
	//=========================================================================
	// No maximum strategy: on crisp times std::max is the maximum. The
	// sgs.append.maximum key is still accepted in a setup file and ignored.

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_JSP_Append(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_JSP_Append(const SGS_JSP_Append &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_JSP * clone() const {
		return new SGS_JSP_Append(*this);
	}


	/*
	* Loads the Maximum strategy to follow
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~SGS_JSP_Append() { }  // No pointers, no destruction



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
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
protected:
	/*
	* Schedules a specific task just after the last task scheduled in its
	* required machine and the last task of its job
	*/
	virtual FuzzyFW::Crisp scheduleTask(const TaskJSP *task,
		const int taskIdx);
};


}
