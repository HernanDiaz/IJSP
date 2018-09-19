/*
* FuzzyTask.h
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/
#ifndef PROBLEM_FUZZYTASK_H_
#define PROBLEM_FUZZYTASK_H_

#include "TFN.h"


namespace FJSP {

//=============================================================================
//
//	Class FuzzyTask
//
//=============================================================================
/**
* Objects of this class contain the constant values for operations
* of a fuzzy job shop scheduling instance. That is, it represents operation
* such that processing times are Fuzzy Numbers (TFNs in this versions)
*
* @author Juan Jose Palacios
*
*/

class FuzzyTask {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* Task identifier
	*/
	unsigned int id;

	/**
	* Job to which it belongs
	*/
	int job;

	/**
	* Machine to be run in
	*/
	int machine;

	/**
	* Preceding task in its job (-1 if none)
	*/
	int jp;

	/**
	* Succeeding task in its job (-1 if none)
	*/
	int js;

	/**
	* Task processing time
	*/
	TFN p;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	FuzzyTask();

	/**
	* Full constructor
	*/
	FuzzyTask(int id, int job, int machine, int jobPred = -1, int jobSuc = -1);

	/**
	* Duration constructor
	*/
	FuzzyTask(int id, int job, int machine, int jobPred, int jobSuc, TFN pt);

	/**
	* Copy constructor
	*/
	FuzzyTask(const FuzzyTask &source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	FuzzyTask & operator=(const FuzzyTask &src);
};

}

#endif /* PROBLEM_FUZZYTASK_H_ */
