/*
* EvaluationJSP.cpp
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/

#include "EvaluationJSP.h"

namespace JSP {


//=============================================================================
//
//	Class EvaluationJSP_Makespan
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationJSP_Makespan::EvaluationJSP_Makespan(
	FuzzyFW::ParameterDB *parameters)
	: Evaluation(parameters) {
	JSP::JSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationJSP_Makespan::EvaluationJSP_Makespan(
	const EvaluationJSP_Makespan & source)
	: Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationJSP_Makespan::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationJSP_Makespan::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleJSP * schedule;
	ProblemJSP *fuzzyProb;
	FuzzyFW::Crisp makespan = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw JSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw JSPException("Evaluation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = std::max(makespan, schedule->getCTJob(i));
	}

	return new FuzzyFW::FitnessCrisp(makespan, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationJSP_Makespan::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleJSP * schedule;
	ProblemJSP *fuzzyProb;
	FuzzyFW::Crisp makespan = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw JSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw JSPException("Evaluation", errorMsg);
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
//	Class EvaluationJSP_Tardiness
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationJSP_Tardiness::EvaluationJSP_Tardiness(
	FuzzyFW::ParameterDB *parameters)
	: Evaluation(parameters) {
	JSP::JSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationJSP_Tardiness::EvaluationJSP_Tardiness(
	const EvaluationJSP_Tardiness & source)
	: Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationJSP_Tardiness::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationJSP_Tardiness::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleJSP * schedule;
	ProblemJSP *fuzzyProb;
	const FuzzyFW::TimeWindowLinear* timeWindow;
	FuzzyFW::Crisp tardiness = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw JSPException("EvaluationJSP", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw JSPException("EvaluationJSP", errorMsg);
	}

	// Compute the tardiness
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		timeWindow = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(fuzzyProb->getTimeWindow(i));
		if (timeWindow == NULL) {
			std::string errorMsg = "This evaluation function works only with ";
			errorMsg += "Linear Timewidows.";
			throw JSPException("EvaluationJSP", errorMsg);
		}

		// Tardiness against a linear time window is [max(0, C - d2),
		// max(0, C - d1)], which is a proper interval whenever d1 != d2 even
		// though C is a point. It is therefore not a crisp quantity, and
		// collapsing it onto either endpoint would be a modelling decision
		// dressed up as an implementation detail. This branch targets the
		// classic makespan; the objective is refused rather than guessed.
		std::string errorMsg = "Tardiness against a linear time window is not ";
		errorMsg += "a crisp quantity. Use jsp.makespan on this branch, or ";
		errorMsg += "the interval solver on feature/JSP.";
		throw JSPException("EvaluationJSP", errorMsg);
	}

	return new FuzzyFW::FitnessCrisp(tardiness, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationJSP_Tardiness::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleJSP * schedule;
	ProblemJSP *fuzzyProb;
	const FuzzyFW::TimeWindowLinear* timeWindow;
	FuzzyFW::Crisp tardiness = FuzzyFW::Crisp(0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Crisp Job Shop Problems.";
		throw JSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Crisp problems.";
		throw JSPException("Evaluation", errorMsg);
	}

	// Compute the tardiness
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		timeWindow = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(fuzzyProb->getTimeWindow(i));

		if (timeWindow == NULL) {
			std::string errorMsg = "This evaluation function works only with ";
			errorMsg += "Linear Timewidows.";
			throw JSPException("EvaluationJSP", errorMsg);
		}

		// See the note in the evaluation above: tardiness against a linear
		// time window is an interval even when the completion time is a point.
		std::string errorMsg = "Tardiness against a linear time window is not ";
		errorMsg += "a crisp quantity. Use jsp.makespan on this branch, or ";
		errorMsg += "the interval solver on feature/JSP.";
		throw JSPException("EvaluationJSP", errorMsg);
	}


	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());

	return new FuzzyFW::FitnessCrisp(tardiness, false);
}
}
