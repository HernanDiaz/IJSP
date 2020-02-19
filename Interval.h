/*
* Interval.h
*
*  Created on: June 26, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include "heading.h"

namespace FuzzyFW {

//=========================================================================
//
//	Class Interval
//
//=========================================================================
/**
* Objects of this class represent Intervals Interval Scheduling Problems
*
* There are many different ways to compare Intervals in the literature. To avoid
* mistakes, comparison operators are not introduced. Instead, the user
* must specify the type of comparison that wants to make.
* 
* Similar thing happens with the maximum, which cannot be computed exactly
* in a reasonable amount of time. Because of that, different maximum
* oprations are included
* 
* @author hdiaz
*
*/

class Interval
{
public:
	/*
	* Different comparisons implemented for comparing Intervals. 
	*	- Component: The comparison is done at component level and all
	*			components must fulfil the condition
	*	- EV: Comparison in terms of Expected Value
	*	- Sakawa: Comparison made using the Sakawa ranking method
	* Detailed explanations on each method are given inside the class
	*/
	enum Compare {C_COMPONENT, C_EV, C_SAKAWA, C_JIANG, C_LEX1, C_LEX2, C_YX, C_Err};

	
	/*
	* Different ways to calculate the maximum of two TFN
	*	- Component: Maximum made component by component
	*	- EV: Takes the interval with higher Expected Value
	*	- Sakawa: Maximum made using the Sakawa ranking method
	*/
	enum Maximum { M_COMPONENT, M_EV, M_SAKAWA, M_JIANG, M_Err};
	
	/**
	* Methods to create a Interval Comparison strategy from a string
	* or convert it to a string
	*/
	static std::string getComparison(Interval::Compare cmp);
	static Interval::Compare getComparison(std::string str);

	/**
	* Methods to create a TFN Maximum operator from a string
	* or convert it to a string
	*/
	static std::string getMaximum(Interval::Maximum mxm);
	static Interval::Maximum getMaximum(std::string str);


		
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* The two defining points
	*/
	double a;
	double b;
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor.
	* Sets all values to 0 by default
	*/
	Interval();

	/**
	* Main constructor
	*/
	explicit Interval(double a1, double a2);

	/**
	* Copy constructor
	*/
	Interval(const Interval & source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	/**
	* Addition component by component
	*/
	Interval operator+(const Interval &t) const;

	/**
	* Substraction component by component
	*/
	Interval operator-(const Interval &t) const;

	/**
	* Assignment overload
	*/
	Interval & operator=(const Interval &t);

	/**
	* Addition and assignment
	*/
	Interval & operator += (const Interval &t);

	/**
	* Substraction and assignment
	*/
	Interval & operator-=(const Interval &t);



	//=========================================================================
	//		COMPARATORS
	//=========================================================================
public:
	/**
	* Check if a given Interval is equal to the current one using a specific
	* comparison method:
	* - COMPONENT: Equality component by component.
	*		More restrictive than comparing by EV
	* - EV: Equality in terms of Expected Value
	* - SAKAWA: Equality based on Sakawa Ranking Method
	*		It checks first the EV, then the modal value and finally the support
	*/
	bool isEqualTo(const Interval t, const Compare cp) const;

	/**
	* Check if the current Interval is greater than a given one using a specific
	* comparison method
	* - COMPONENT:  Greater component by component.
	*		More restrictive than comparing by EV
	* - EV: Greater in terms of Expected Value
	* - SAKAWA: Greater using the Sakawa Ranking Method
	*		It checks first the EV, then the modal value and finally the support
	*/
	bool isGreaterThan(const Interval t, const Compare cp) const;

	bool isGreaterEqualTo(const Interval t, const Compare cp) const;
	
	bool isLesserThan(const Interval t, const Compare cp) const;

	/**
	* Check if the current TFN is lesser or equal to a given one using a
	* specific comparison method
	*/
	bool isLesserEqualTo(const Interval t, const Compare cp) const;


	/**
	* Equality in one component
	*/
	bool EqualComponent(const Interval t, const unsigned int component) const;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Calculates the approximated maximum of two Intervals given a specific
	* strategy:
	* - COMPONENT: Computed the maximum component by component
	* - EV: Returns the Interval with higher Expected Value
	* - SAKAWA: Returns the greatest Interval given by tha Sakawa ranking method
	*/
	friend Interval maximum(const Interval & x, const Interval & y, const Maximum mt);


	/**
	* Calculates the approximated minimum extrapolating the definitions
	* given for the maximum.
	*/
	friend Interval minimum(const Interval & x, const Interval & y, const Maximum mt);


	/**
	* Expected value of the Interval
	*/
	double expectedValue() const;



	/**
	* Degree of membership for the real number r
	*/
	double membership(const double r) const;


	/**
	* Converts the Interval into a string
	*/
	std::string toString() const;


	
	double Lei2011Formula(const Interval & t) const;

	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Reads a stream of data in the format "(a,b)"
	*/
	friend std::ifstream & operator >> (std::ifstream & is, Interval & t);

	/**
	* Writes a stream of data in the format "(a,b)"
	*/
	friend std::ostream & operator << (std::ostream & os, const Interval & t);
};

}
