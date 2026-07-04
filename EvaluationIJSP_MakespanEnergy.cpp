/*
 * EvaluationIJSP_MakespanEnergy.cpp
 *
 *  Created on: Jul 4, 2026
 *      Author: hdiaz
 */

#include "EvaluationIJSP_MakespanEnergy.h"
#include "EvaluationIJSP_Energy.h"

namespace IJSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
EvaluationIJSP_MakespanEnergy::EvaluationIJSP_MakespanEnergy()
	: maximumLabel(IJSP_EVALUATION_MAXIMUM),
	intervalMaximum(FuzzyFW::Interval::Maximum::M_COMPONENT) {
}


EvaluationIJSP_MakespanEnergy::EvaluationIJSP_MakespanEnergy(
	const EvaluationIJSP_MakespanEnergy &source)
	: FuzzyFW::Evaluation(source), maximumLabel(source.maximumLabel),
	intervalMaximum(source.intervalMaximum) {
}


void EvaluationIJSP_MakespanEnergy::setup(FuzzyFW::ParameterDB *parameters) {
	std::string maxName;

	FuzzyFW::Evaluation::setup(parameters);

	// Load maximum type parameter
	maxName = parameters->getString(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->intervalMaximum = FuzzyFW::Interval::getMaximum(maxName);
	if (this->intervalMaximum == FuzzyFW::Interval::Maximum::M_Err) {
		std::string errorMsg = "Unknown maximum strategy for ";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw IJSPException("Evaluation", errorMsg);
	}
}


//=============================================================================
//		METHODS
//=============================================================================
//----- Lexicographic fitness of an already-built schedule  -------------------
FuzzyFW::Fitness * EvaluationIJSP_MakespanEnergy::evaluateSchedule(
	const ScheduleIJSP *schedule, const ProblemIJSP *problem) const {

	FuzzyFW::Interval makespan(0, 0);
	for (unsigned int i = 0; i < problem->getNumberJobs(); i++)
		makespan = maximum(makespan, schedule->getCTJob(i),
			this->intervalMaximum);

	FuzzyFW::FitnessLexicographic *fitness =
		new FuzzyFW::FitnessLexicographic();
	fitness->addFitness(new FuzzyFW::FitnessInterval(makespan, false));
	fitness->addFitness(new FuzzyFW::FitnessInterval(
		EvaluationIJSP_Energy::computeNPE(schedule, problem), false));
	return fitness;
}


//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationIJSP_MakespanEnergy::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;

	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	return this->evaluateSchedule(schedule, fuzzyProb);
}


//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationIJSP_MakespanEnergy::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;

	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());

	return this->evaluateSchedule(schedule, fuzzyProb);
}

} // namespace IJSP
