/*
 * DecoderJSP_Order.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "DecoderJSP.h"


namespace JSP {

//=============================================================================
//
//	Class DecoderJSP_Order
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype in the form
* of a task permutation and generates a solution using a specific SGS
*
* @author hdiaz
*
*/
class DecoderJSP_Order : public DecoderJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderJSP_Order(FuzzyFW::ParameterDB *parameters = NULL)
		: DecoderJSP(parameters) { }


	/**
	* Copy constructor
	*/
	DecoderJSP_Order(const DecoderJSP_Order & source)
		: DecoderJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderJSP_Order(*this);
	}


	/**
	* Destructor
	*/
	virtual ~DecoderJSP_Order() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Decodes an individual to create a full schedule
	*/
	virtual FuzzyFW::Solution * decode(FuzzyFW::Individual * indiv,
		const FuzzyFW::SharedVarsEvolutionary * const svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		return buildDecoderName("Task Permutation");
	}
};

}
