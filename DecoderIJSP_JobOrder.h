/*
 * DecoderIJSP_JobOrder.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "DecoderIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class DecoderIJSP_JobOrder
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author hdiaz
*
*/
class DecoderIJSP_JobOrder : public DecoderIJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderIJSP_JobOrder(FuzzyFW::ParameterDB *parameters = NULL)
		: DecoderIJSP(parameters) { }

	/**
	* Copy constructor
	*/
	DecoderIJSP_JobOrder(const DecoderIJSP_JobOrder & source)
		: DecoderIJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderIJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~DecoderIJSP_JobOrder() { }



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
