/*
* TFN.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/
#ifndef UTILS_TFN_H_
#define UTILS_TFN_H_

#include "heading.h"

namespace FJSP {

//=========================================================================
//
//	Class TFN
//
//=========================================================================
/**
* Objects of this class represent the TFNs as they are introduced in
* the paper from Sakawa&Kubota in 2000 for Fuzzy Scheduling Problems
*
* There are many different ways to compare TFNs in the literature. To avoid
* mistakes, comparison operators are not introduced. Instead, the user
* must specify the type of comparison that wants to make.
* 
* Similar thing happens with the maximum, which cannot be computed exactly
* in a reasonable amount of time. Because of that, different maximum
* oprations are included
* 
* @author Juan Jose Palacios
*
*/

class TFN
{
public:
	/*
	* Different comparisons implemented for comparing TFN. 
	*	- Component: The comparison is done at component level and all
	*			components must fulfil the condition
	*	- EV: Comparison in terms of Expected Value
	*	- Sakawa: Comparison made using the Sakawa ranking method
	* Detailed explanations on each method are given inside the class
	*/
	enum Compare {C_COMPONENT, C_EV, C_SAKAWA, C_Err};

	
	/*
	* Different ways to calculate the maximum of two TFN
	*	- Component: Maximum made component by component
	*	- EV: Takes the TFN with higher Expected Value
	*	- Sakawa: Maximum made using the Sakawa ranking method
	*/
	enum Maximum { M_COMPONENT, M_EV, M_SAKAWA, M_Err};
	
	/**
	* Methods to create a TFN Comparison strategy from a string
	* or convert it to a string
	*/
	static std::string getComparison(TFN::Compare cmp);
	static TFN::Compare getComparison(std::string str);

	/**
	* Methods to create a TFN Maximum operator from a string
	* or convert it to a string
	*/
	static std::string getMaximum(TFN::Maximum mxm);
	static TFN::Maximum getMaximum(std::string str);


		
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* The three defining points
	*/
	double a;
	double b;
	double c;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor.
	* Sets all values to 0 by default
	*/
	TFN();

	/**
	* Main constructor
	*/
	explicit TFN(double a1, double a2, double a3);

	/**
	* Copy constructor
	*/
	TFN(const TFN & source);



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	/**
	* Addition component by component
	*/
	TFN operator+(const TFN &t) const;

	/**
	* Substraction component by component
	*/
	TFN operator-(const TFN &t) const;

	/**
	* Assignment overload
	*/
	TFN & operator=(const TFN &t);

	/**
	* Addition and assignment
	*/
	TFN & operator+=(const TFN &t);

	/**
	* Substraction and assignment
	*/
	TFN & operator-=(const TFN &t);



	//=========================================================================
	//		COMPARATORS
	//=========================================================================
public:
	/**
	* Check if a given TFN is equal to the current one using a specific
	* comparison method:
	* - COMPONENT: Equality component by component.
	*		More restrictive than comparing by EV
	* - EV: Equality in terms of Expected Value
	* - SAKAWA: Equality based on Sakawa Ranking Method
	*		It checks first the EV, then the modal value and finally the support
	*/
	bool isEqualTo(const TFN t, const Compare cp) const;

	/**
	* Check if the current TFN is greater than a given one using a specific
	* comparison method
	* - COMPONENT:  Greater component by component.
	*		More restrictive than comparing by EV
	* - EV: Greater in terms of Expected Value
	* - SAKAWA: Greater using the Sakawa Ranking Method
	*		It checks first the EV, then the modal value and finally the support
	*/
	bool isGreaterThan(const TFN t, const Compare cp) const;

	/**
	* Check if the current TFN is greater or equal to a given one using a
	* specific comparison method
	*/
	bool isGreaterEqualTo(const TFN t, const Compare cp) const;

	/**
	* Check if the current TFN is lesser than a given one using a
	* specific comparison method
	*/
	bool isLesserThan(const TFN t, const Compare cp) const;

	/**
	* Check if the current TFN is lesser or equal to a given one using a
	* specific comparison method
	*/
	bool isLesserEqualTo(const TFN t, const Compare cp) const;


	/**
	* Equality in one component
	*/
	bool EqualComponent(const TFN t, const unsigned int component) const;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Calculates the approximated maximum of two TFNs given a specific
	* strategy:
	* - COMPONENT: Computed the maximum component by component
	* - EV: Returns the TFN with higher Expected Value
	* - SAKAWA: Returns the greatest TFN given by tha Sakawa ranking method
	*/
	friend TFN maximum(const TFN & x, const TFN & y, const Maximum mt);


	/**
	* Calculates the approximated minimum extrapolating the definitions
	* given for the maximum.
	*/
	friend TFN minimum(const TFN & x, const TFN & y, const Maximum mt);


	/**
	* Expected value of the TFN
	*/
	double expectedValue() const;


	/**
	* Converts the TFN into a string
	*/
	std::string toString() const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Reads a stream of data in the format "(a,b,c)"
	*/
	friend std::ifstream & operator >> (std::ifstream & is, TFN & t);

	/**
	* Writes a stream of data in the format "(a,b,c)"
	*/
	friend std::ostream & operator << (std::ostream & os, const TFN & t);
};

}

#endif /* UTILS_TFN_H_ */
