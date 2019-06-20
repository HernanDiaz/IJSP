/*
 * EncoderFJSP.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: jjpalacios
 */

#include "EncoderIJSP.h"

namespace IJSP {

//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderIJSP_Order::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleIJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	
	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw new IJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new IJSPException("Encoding", errorMsg);
	}

	intIndiv->updateGenotype(schedule->getTaskOrder(svars->rng));
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderIJSP_Order::encode(FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleIJSP *schedule;
	std::vector<unsigned int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw new IJSPException("Encoding", errorMsg);
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
void EncoderIJSP_JobOrder::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleIJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	ProblemIJSP * fuzzyProb;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw new IJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new IJSPException("Encoding", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new IJSPException("Encoding", errorMsg);
	}

	genotype = schedule->getTaskOrder(svars->rng);
	for (size_t i = 0; i < genotype.size(); i++)
		genotype[i] = (*fuzzyProb)[genotype[i]]->job;
	
	intIndiv->updateGenotype(genotype);
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderIJSP_JobOrder::encode(
	FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {


	ScheduleIJSP *schedule;
	std::vector<int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw new IJSPException("Encoding", errorMsg);
	}

	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw new IJSPException("Encoding", errorMsg);
	}

	taskOrder = schedule->getTaskOrder(svars->rng);
	genotype.resize(taskOrder.size());
	for (size_t i = 0; i < taskOrder.size(); i++)
		genotype[i] = (*fuzzyProb)[taskOrder[i]]->job;

	return new FuzzyFW::IndividualArrayInt(genotype);
}

}
