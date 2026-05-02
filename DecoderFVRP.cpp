/*
 * DecoderFVRP.cpp
 *
 *  Created on: Nov 23, 2017
 */

#include "DecoderFVRP.h"

namespace FVRP {

//=============================================================================
//
//	Abstract class DecoderFVRP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
DecoderFVRP::DecoderFVRP(const DecoderFVRP & source)
	: sgsLabel(source.sgsLabel), Decoder(source) {
	if (source.sgs)
		this->sgs.reset(source.sgs->clone());
}



//-----  Setup method  --------------------------------------------------------
void DecoderFVRP::setup(FuzzyFW::ParameterDB *parameters) {
	Decoder::setup(parameters);

	// Loads the SGS type to create
	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw FVRPException("Evaluation", errorMsg);
	}

	this->sgs.reset(FVRPClassRegister::getSGSObject(sgsType));
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw FVRPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}





//=============================================================================
//
//	Class DecoderFRVP_Split
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
FuzzyFW::Solution * DecoderFRVP_Split::decode(FuzzyFW::Individual * indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) {

	FuzzyFW::IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderFVRP_Order *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with a ";
		errorMsg += "incompatible method";
		throw FVRPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw FVRPException("Decoding", errorMsg);
	}
	
	this->sgs->reset();
	return sgs->buildPlan(svars, ind->getGenotype());
}


}
