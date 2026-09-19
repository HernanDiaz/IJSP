/*
* TaskJSP.h
*
*  Created on: June 18, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include "CrispTime.h"


namespace JSP {

//=============================================================================
//
//	Class TaskJSP
//
//=============================================================================
/**
* Objects of this class contain the constant values for operations
* of an interval job shop scheduling instance. That is, it represents operation
* such that processing times are Intervals (Crisp in this versions)
*
* @author hdiaz
*
*/

class TaskJSP {

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
	FuzzyFW::Crisp p;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	TaskJSP();

	/**
	* Full constructor
	*/
	TaskJSP(int id, int job, int machine, int jobPred = -1, int jobSuc = -1);

	/**
	* Duration constructor
	*/
	TaskJSP(int id, int job, int machine, int jobPred, int jobSuc, 
		FuzzyFW::Crisp pt);

	/**
	* Copy constructor
	*/
	TaskJSP(const TaskJSP &source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	TaskJSP & operator=(const TaskJSP &src);
};

}
