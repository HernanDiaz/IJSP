/*
* TaskFJSP.h
*
*  Created on: May 12, 2017
*/
#pragma once

#include "TFN.h"


namespace FJSP {

//=============================================================================
//
//	Class TaskFJSP
//
//=============================================================================
/**
* Objects of this class contain the constant values for operations
* of a fuzzy job shop scheduling instance. That is, it represents operation
* such that processing times are Fuzzy Numbers (TFNs in this versions)
*
*
*/

class TaskFJSP {

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
	FuzzyFW::TFN p;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	TaskFJSP();

	/**
	* Full constructor
	*/
	TaskFJSP(int id, int job, int machine, int jobPred = -1, int jobSuc = -1);

	/**
	* Duration constructor
	*/
	TaskFJSP(int id, int job, int machine, int jobPred, int jobSuc, 
		FuzzyFW::TFN pt);

	/**
	* Copy constructor
	*/
	TaskFJSP(const TaskFJSP &source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	TaskFJSP & operator=(const TaskFJSP &src);
};

}
