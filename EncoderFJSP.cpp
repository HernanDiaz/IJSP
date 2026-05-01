/*
 * EncoderFJSP.cpp
 *
 *  Created on: Sep 14, 2017
 */

#include "EncoderFJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderFJSP_Order::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleFJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	
	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv->updateGenotype(schedule->getTaskOrder(svars->rng));
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderFJSP_Order::encode(FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleFJSP *schedule;
	std::vector<unsigned int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	return new FuzzyFW::IndividualArrayInt(schedule->getTaskOrder(svars->rng));
}




//=============================================================================
//
//	Class EncoderFJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderFJSP_JobOrder::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleFJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	ProblemFJSP * fuzzyProb;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new FJSPException("Encoding", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	genotype = schedule->getTaskOrder(svars->rng);
	for (size_t i = 0; i < genotype.size(); i++)
		genotype[i] = (*fuzzyProb)[genotype[i]]->job;
	
	intIndiv->updateGenotype(genotype);
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderFJSP_JobOrder::encode(
	FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {


	ScheduleFJSP *schedule;
	std::vector<int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	taskOrder = schedule->getTaskOrder(svars->rng);
	genotype.resize(taskOrder.size());
	for (size_t i = 0; i < taskOrder.size(); i++)
		genotype[i] = (*fuzzyProb)[taskOrder[i]]->job;

	return new FuzzyFW::IndividualArrayInt(genotype);
}

}
