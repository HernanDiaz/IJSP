/*
 * DecoderIJSP.cpp
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */

#include "DecoderIJSP.h"

namespace IJSP {

//=============================================================================
//
//	Abstract class DecoderIJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
DecoderIJSP::DecoderIJSP(const DecoderIJSP & source)
	: sgsLabel(source.sgsLabel), Decoder(source) {
	if (source.sgs)
		this->sgs.reset(source.sgs->clone());
}



//-----  Setup method  --------------------------------------------------------
std::vector<std::string> DecoderIJSP::buildDecoderName(const std::string &name) const {
	std::vector<std::string> result;
	std::vector<std::string> sgsName = this->sgs->getName();
	result.push_back(name);
	result.push_back(";SGS:;" + sgsName[0]);
	for (size_t i = 1; i < sgsName.size(); i++)
		result.push_back(";" + sgsName[i]);
	return result;
}

void DecoderIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Decoder::setup(parameters);

	// Loads the SGS type to create
	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs.reset(IJSPClassRegister::getSGSObject(sgsType));
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}





//=============================================================================
//
//	Class DecoderIJSP_Order
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
FuzzyFW::Solution * DecoderIJSP_Order::decode(FuzzyFW::Individual * indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) {

	FuzzyFW::IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderIJSP_Order *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with a ";
		errorMsg += "incompatible method";
		throw IJSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw IJSPException("Decoding", errorMsg);
	}
	
	this->sgs->reset();
	return sgs->buildSchedule(svars, ind->getGenotype());
}





//=============================================================================
//
//	Class DecoderIJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
FuzzyFW::Solution * DecoderIJSP_JobOrder::decode(FuzzyFW::Individual * indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) {

	int job;
	std::vector<int> solution, count;
	FuzzyFW::IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderIJSP_JobOrder *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with an ";
		errorMsg += "incompatible method";
		throw IJSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw IJSPException("Decoding", errorMsg);
	}

	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw IJSPException("Encoding", errorMsg);
	}

	solution = ind->getGenotype();
	count.resize(fuzzyProb->getNumberJobs(), 0);
	for (size_t i = 0; i < solution.size(); i++) {
		job = solution[i];
		solution[i] = fuzzyProb->getTaskId(job, count[job]);
		count[job]++;
	}

	this->sgs->reset();
	return sgs->buildSchedule(svars, solution);
}

}
