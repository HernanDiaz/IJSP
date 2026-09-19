/*
 * DecoderJSP_JobOrder.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "DecoderJSP.h"


namespace JSP {

//=============================================================================
//
//	Class DecoderJSP_JobOrder
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author hdiaz
*
*/
class DecoderJSP_JobOrder : public DecoderJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderJSP_JobOrder(FuzzyFW::ParameterDB *parameters = NULL)
		: DecoderJSP(parameters) { }

	/**
	* Copy constructor
	*/
	DecoderJSP_JobOrder(const DecoderJSP_JobOrder & source)
		: DecoderJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~DecoderJSP_JobOrder() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Decodes an individual to create a full schedule
	*/
	FuzzyFW::Solution * decode(FuzzyFW::Individual * indiv,
		const FuzzyFW::SharedVarsEvolutionary * const svars);

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		return buildDecoderName("Job Permutation");
	}
};

}
