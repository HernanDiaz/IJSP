/*
* Solution.h
*
*  Created on: September 14, 2017
*      Author: jjpalacios
*/
#pragma once

#include "heading.h"

namespace FuzzyFW {


//=============================================================================
//
//	Abstract class Solution
//
//=============================================================================
/**
* This class is a simple super class under which solutions to any problem can
* be defined bi inheritance.
*
* The only target of this class is to provide a wrapper, so methods of
* different algorithms can receive a solution to a problem under the name
* of this class.
*
* @author jjpalacios
*
*/
class Solution {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	Solution() { }

	/**
	* Copy constructor
	*/
	Solution(const Solution &source) { }

	/**
	* Destructor
	*/
	virtual ~Solution() { }

	/**
	* Clone operator for the case of inheritance
	*/
	virtual Solution* clone() const = 0;

	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/**
	* Converts the solution to a string
	*/
	virtual std::string toString() const = 0;

};

}

