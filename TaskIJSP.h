/*
* TaskIJSP.h
*
*  Created on: June 18, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include "TFN.h"


namespace IJSP {

//=============================================================================
//
//	Class TaskIJSP
//
//=============================================================================
/**
* Objects of this class contain the constant values for operations
* of an interval job shop scheduling instance. That is, it represents operation
* such that processing times are Intervals (Interval in this versions)
*
* @author hdiaz
*
*/

class TaskIJSP {

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
	TaskIJSP();

	/**
	* Full constructor
	*/
	TaskIJSP(int id, int job, int machine, int jobPred = -1, int jobSuc = -1);

	/**
	* Duration constructor
	*/
	TaskIJSP(int id, int job, int machine, int jobPred, int jobSuc, 
		FuzzyFW::TFN pt);

	/**
	* Copy constructor
	*/
	TaskIJSP(const TaskIJSP &source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	TaskIJSP & operator=(const TaskIJSP &src);
};

}
