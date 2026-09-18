/*
* EvaluationIJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "EvaluationIJSP.h"

namespace IJSP {


//=============================================================================
//
//	Class EvaluationIJSP_Makespan
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationIJSP_Makespan::EvaluationIJSP_Makespan(
	FuzzyFW::ParameterDB *parameters)
	: Evaluation(parameters) {
	IJSP::IJSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationIJSP_Makespan::EvaluationIJSP_Makespan(
	const EvaluationIJSP_Makespan & source)
	: Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationIJSP_Makespan::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationIJSP_Makespan::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;
	FuzzyFW::Crisp makespan = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = std::max(makespan, schedule->getCTJob(i));
	}

	return new FuzzyFW::FitnessCrisp(makespan, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationIJSP_Makespan::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;
	FuzzyFW::Crisp makespan = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = std::max(makespan, schedule->getCTJob(i));
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
 	return new FuzzyFW::FitnessCrisp(makespan, false);
}


//=============================================================================
//
//	Class EvaluationIJSP_Tardiness
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationIJSP_Tardiness::EvaluationIJSP_Tardiness(
	FuzzyFW::ParameterDB *parameters)
	: Evaluation(parameters) {
	IJSP::IJSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationIJSP_Tardiness::EvaluationIJSP_Tardiness(
	const EvaluationIJSP_Tardiness & source)
	: Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationIJSP_Tardiness::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationIJSP_Tardiness::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;
	const FuzzyFW::TimeWindowLinear* timeWindow;
	FuzzyFW::Crisp tardiness = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw IJSPException("EvaluationIJSP", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw IJSPException("EvaluationIJSP", errorMsg);
	}

	// Compute the tardiness
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		timeWindow = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(fuzzyProb->getTimeWindow(i));
		if (timeWindow == NULL) {
			std::string errorMsg = "This evaluation function works only with ";
			errorMsg += "Linear Timewidows.";
			throw IJSPException("EvaluationIJSP", errorMsg);
		}

		// Tardiness against a linear time window is [max(0, C - d2),
		// max(0, C - d1)], which is a proper interval whenever d1 != d2 even
		// though C is a point. It is therefore not a crisp quantity, and
		// collapsing it onto either endpoint would be a modelling decision
		// dressed up as an implementation detail. This branch targets the
		// classic makespan; the objective is refused rather than guessed.
		std::string errorMsg = "Tardiness against a linear time window is not ";
		errorMsg += "a crisp quantity. Use ijsp.makespan on this branch, or ";
		errorMsg += "the interval solver on feature/IJSP.";
		throw IJSPException("EvaluationIJSP", errorMsg);
	}

	return new FuzzyFW::FitnessCrisp(tardiness, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationIJSP_Tardiness::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;
	const FuzzyFW::TimeWindowLinear* timeWindow;
	FuzzyFW::Crisp tardiness = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	// Compute the tardiness
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		timeWindow = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(fuzzyProb->getTimeWindow(i));

		if (timeWindow == NULL) {
			std::string errorMsg = "This evaluation function works only with ";
			errorMsg += "Linear Timewidows.";
			throw IJSPException("EvaluationIJSP", errorMsg);
		}

		// See the note in the evaluation above: tardiness against a linear
		// time window is an interval even when the completion time is a point.
		std::string errorMsg = "Tardiness against a linear time window is not ";
		errorMsg += "a crisp quantity. Use ijsp.makespan on this branch, or ";
		errorMsg += "the interval solver on feature/IJSP.";
		throw IJSPException("EvaluationIJSP", errorMsg);
	}


	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());

	return new FuzzyFW::FitnessCrisp(tardiness, false);
}
}
