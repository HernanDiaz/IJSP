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
	: maximumLabel(IJSP_EVALUATION_MAXIMUM),
	intervalMaximum(FuzzyFW::Interval::M_COMPONENT),
	compareLabel(IJSP_EVALUATION_COMPARE), intervalCompare(FuzzyFW::Interval::C_EV),
	Evaluation(parameters) {
	//TODO IJSPClassRegister
	FJSP::FJSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationIJSP_Makespan::EvaluationIJSP_Makespan(
	const EvaluationIJSP_Makespan & source)
	: maximumLabel(source.maximumLabel), intervalMaximum(source.intervalMaximum),
	compareLabel(source.compareLabel), intervalCompare(source.intervalCompare),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationIJSP_Makespan::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string compareName, maxName;

	// Load maximum type parameter
	maxName = parameters->getString(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw new IJSPException("Evaluation", errorMsg);
	}
	this->intervalMaximum = FuzzyFW::Interval::getMaximum(maxName);
	if (this->intervalMaximum == FuzzyFW::Interval::M_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw new IJSPException("Evaluation", errorMsg);
	}

	// Load comparison strategy parameter
	compareName = parameters->getString(this->compareLabel);
	if (compareName.length() == 0) {
		std::string errorMsg = this->compareLabel + " parameter not found.";
		throw new IJSPException("Evaluation", errorMsg);
	}
	this->intervalCompare = FuzzyFW::Interval::getComparison(compareName);
	if (this->intervalCompare == FuzzyFW::Interval::C_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += compareName + "\'";
		throw new IJSPException("Evaluation", errorMsg);
	}
	FuzzyFW::FitnessInterval::FitnessCompareStrategy = this->intervalCompare;
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
	FuzzyFW::Interval makespan = FuzzyFW::Interval(0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw new IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw new IJSPException("Evaluation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = maximum(makespan, schedule->getCTJob(i), this->intervalMaximum);
	}

	return new FuzzyFW::FitnessInterval(makespan, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationIJSP_Makespan::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;
	FuzzyFW::Interval makespan = FuzzyFW::Interval(0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw new IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw new IJSPException("Evaluation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = maximum(makespan, schedule->getCTJob(i), this->intervalMaximum);
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FuzzyFW::FitnessInterval(makespan, false);
}
}
