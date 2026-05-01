/*
 * DecoderIJSP_Order.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "DecoderIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class DecoderIJSP_Order
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype in the form
* of a task permutation and generates a solution using a specific SGS
*
* @author hdiaz
*
*/
class DecoderIJSP_Order : public DecoderIJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderIJSP_Order(FuzzyFW::ParameterDB *parameters = NULL)
		: DecoderIJSP(parameters) { }


	/**
	* Copy constructor
	*/
	DecoderIJSP_Order(const DecoderIJSP_Order & source)
		: DecoderIJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderIJSP_Order(*this);
	}


	/**
	* Destructor
	*/
	virtual ~DecoderIJSP_Order() { }



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
		std::vector<std::string> name;
		std::vector<std::string> sgsName = this->sgs->getName();
		name.push_back("Task Permutation");
		name.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			name.push_back(";" + sgsName[i]);
		return name;
	}
};

}
