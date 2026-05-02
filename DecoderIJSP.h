/*
 * DecoderIJSP.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "IJSPClassRegister.h"
#include "Decoder.h"
#include <memory>
#include "EncoderIJSP.h"


namespace IJSP {

#define DECODING_SGS "decode.sgs"

//=============================================================================
//
//	Abstract class DecoderIJSP
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author hdiaz
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
	std::unique_ptr<SGS_IJSP> sgs;



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
	virtual ~DecoderIJSP() = default;
};

}

#include "DecoderIJSP_Order.h"
#include "DecoderIJSP_JobOrder.h"
