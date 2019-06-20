/*
 * DecoderFJSP.h
 *
 *  Created on: September 12, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "FJSPClassRegister.h"
#include "Decoder.h"
#include "EncoderIJSP.h"


namespace IJSP {

#define DECODING_SGS "decode.sgs"

//=============================================================================
//
//	Abstract class DecoderFJSP
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author jjpalacios
*
*/
class DecoderIJSP : public FuzzyFW::Decoder {
protected:
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
	/*
	* Label to identify the SGS type
	*/
	const std::string sgsLabel;

	/*
	* SGS to create schedules from task orderings
	*/
	SGS_IJSP * sgs;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Decoder(parameters), sgsLabel(DECODING_SGS) { }

	/**
	* Copy constructor
	*/
	DecoderIJSP(const DecoderIJSP & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~DecoderIJSP() {
		delete this->sgs;
	}
};





//=============================================================================
//
//	Class DecoderFJSP_Order
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype in the form
* of a task permutation and generates a solution using a specific SGS
*
* @author jjpalacios
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





//=============================================================================
//
//	Class DecoderFJSP_JobOrder
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author jjpalacios
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
		std::vector<std::string> name;
		std::vector<std::string> sgsName = this->sgs->getName();
		name.push_back("Job Permutation");
		name.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			name.push_back(";" + sgsName[i]);
		return name;
	}
};

}
