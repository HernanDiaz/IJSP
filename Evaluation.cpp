/*
* Evaluation.cpp
*
*  Created on: July 10, 2017
*      Author: Juan Jose Palacios
*/

#include "Evaluation.h"

namespace FJSP {

//=============================================================================
//
//	Abstract Class Evaluation
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
Evaluation::Evaluation(ParameterDB *parameters)
	: lamarckLabel(EVALUATION_LAMARCK), lamarckism(false) {
	if (parameters != NULL)
		this->setup(parameters);
}

//-----  Copy constructor  ----------------------------------------------------
Evaluation::Evaluation(const Evaluation & source)
	: lamarckLabel(source.lamarckLabel), lamarckism(source.lamarckism) { }


//-----  Setup method  --------------------------------------------------------
void Evaluation::setup(ParameterDB *parameters) {
	// Checks is lamarckism is desired
	this->lamarckism = parameters->getBoolean(this->lamarckLabel);
}


	
//=============================================================================
//		METHODS
//=============================================================================
//-----  Evaluate Population  -------------------------------------------------
void Evaluation::evaluatePopulation(const SharedVars * const svars,
	Population *population, bool reEvaluate) const {
	Fitness *fitness;
	for (unsigned int i = 0; i < population->size(); i++) {
		if (!population->getIndividual(i)->isEvaluated()
			|| reEvaluate) {
			fitness = this->evaluate(svars, population->getIndividual(i));
			population->getIndividual(i)->updateFitness(fitness);
		}
	}
}





//=============================================================================
//
//	Class EvaluationFJSP_Makespan
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationFJSP_Makespan::EvaluationFJSP_Makespan(ParameterDB *parameters)
	: maximumLabel(EVALUATION_MAXIMUM), tfnMaximum(TFN::M_COMPONENT),
	compareLabel(EVALUATION_COMPARE), tfnCompare(TFN::C_EV),
	Evaluation(parameters) {
	SchedulingClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationFJSP_Makespan::EvaluationFJSP_Makespan(
	const EvaluationFJSP_Makespan & source)
	: maximumLabel(source.maximumLabel), tfnMaximum(source.tfnMaximum),
	compareLabel(source.compareLabel), tfnCompare(source.tfnCompare),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationFJSP_Makespan::setup(ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string compareName, maxName;

	// Load maximum type parameter
	maxName = parameters->getString(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw new FJSPException("Evaluation", errorMsg);
	}
	this->tfnMaximum = TFN::getMaximum(maxName);
	if (this->tfnMaximum == TFN::M_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->maximumLabel + "\': \'";
		errorMsg += maxName + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Load comparison strategy parameter
	compareName = parameters->getString(this->compareLabel);
	if (compareName.length() == 0) {
		std::string errorMsg = this->compareLabel + " parameter not found.";
		throw new FJSPException("Evaluation", errorMsg);
	}
	this->tfnCompare = TFN::getComparison(compareName);
	if (this->tfnCompare == TFN::C_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += compareName + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}
	FitnessTFN::FitnessCompareStrategy = this->tfnCompare;
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Evaluate  -------------------------------------------------------------
Fitness * EvaluationFJSP_Makespan::evaluate(const SharedVars * const svars,
	Individual *individual) const {

	Solution * solution;
	FuzzySchedule * schedule;
	FuzzyProblem *fuzzyProb;
	TFN makespan = TFN(0, 0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);
	
	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation",errorMsg);
	}

	fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Ealuation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = maximum(makespan, schedule->getCTJob(i), this->tfnMaximum);
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FitnessTFN(makespan, false);
}





//=============================================================================
//
//	Class EvaluationFJSP_AImin
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationFJSP_AImin::EvaluationFJSP_AImin(ParameterDB *parameters)
	: exactLabel(EVALUATION_AI), isExact(true),
	Evaluation(parameters) {
	SchedulingClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationFJSP_AImin::EvaluationFJSP_AImin(const EvaluationFJSP_AImin & source)
	: exactLabel(source.exactLabel), isExact(source.isExact),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationFJSP_AImin::setup(ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string exactName;

	// Load maximum type parameter
	exactName = parameters->getStringUpper(this->exactLabel);
	if (exactName.length() == 0) {
		std::string errorMsg = this->exactLabel + " parameter not found.";
		throw new FJSPException("Evaluation", errorMsg);
	}
	if (exactName.compare(toUpper(EVAL_AI_EXACT)) == 0)
		this->isExact = true;
	else if (exactName.compare(toUpper(EVAL_AI_APROX)) == 0)
		this->isExact = false;
	else {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->exactLabel + "\': \'";
		errorMsg += exactName + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Evaluate  -------------------------------------------------------------
Fitness * EvaluationFJSP_AImin::evaluate(const SharedVars * const svars,
	Individual *individual) const {

	double ai, minAI = 1.0;
	Solution * solution;
	FuzzySchedule * schedule;
	FuzzyProblem * fuzzyProb;

	fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Ealuation", errorMsg);
	}

	if (!fuzzyProb->hasDueDates()) {
		std::string errorMsg = "Agreement index cannot be computed over a ";
		errorMsg += "problem without due-dates";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		ai = this->agreementIndex(fuzzyProb->getDueDate(i), schedule->getCTJob(i));
		if (compareDouble(ai, minAI) < 0)
			minAI = ai;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FitnessDouble(minAI, true);
}


//----- Agreement Index  ------------------------------------------------------
double EvaluationFJSP_AImin::agreementIndex(const DueDate * const dd,
	const TFN &completionTime) const {

	if (this->isExact)
		return dd->agreementIndex(completionTime);

	double c1 = completionTime.a;
	double c2 = completionTime.b;
	double c3 = completionTime.c;
	double d1, d2;

	if (dd->getType() == DueDate::Type::CRISP) {
		const DueDateCrisp * ddl = dynamic_cast<const DueDateCrisp *>(dd);
		d1 = ddl->duedate;

		// c3 <= d
		if (compareDouble(c3, d1) < 1)
			return 1.0;
		// c1 >= d (and c2 >= d and c3 > d)
		if (compareDouble(c1, d1) > -1)
			return 0.0;
		// c2 >= d (and c3 > d and c1 < d)
		if (compareDouble(c2, d1) > -1)
			return ((d1 - c1)*(d1 - c1)) / ((c2 - c1)*(c3 - c1));

		// (c3 > d and c1 < d and c2 < d)
		else {
			return (d1 - c1) / (c3 - c1);
		}
	}
	if (dd->getType() == DueDate::Type::LINEAR) {
		const DueDateLinear * ddl = dynamic_cast<const DueDateLinear *>(dd);
		d1 = ddl->d1;
		d2 = ddl->d2;

		double ip1;	// Intersection points

		// c2 <= d1 and c3 <= d2
		if (compareDouble(c2, d1) <= 0 && compareDouble(c3, d2) <= 0)
			return 1.0;
		// c1 >= d2
		if (compareDouble(c1, d2) >= 0)
			return 0.0;
		// c2 >= d1 and c3 >= d2
		if (compareDouble(c2, d1) >= 0 && compareDouble(c3, d2) >= 0) {
			ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);

			if (compareDouble(c1, c2) == 0)
				return ((ip1 - d2)*(d2 - c1)) / ((d1 - d2)*(c3 - c1));
			else
				return ((ip1 - c1)*(d2 - c1)) / ((c2 - c1)*(c3 - c1));
		}
		// c2 < d1 and c3 > d2
		if (compareDouble(c2, d1) < 0 && compareDouble(c3, d2) > 0) {
			return (d2 - c1) / (c3 - c1);
		}
		else {
			ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);

			if (compareDouble(c1, c2) == 0)
				return (ip1 - d2) / (d1 - d2);
			else {
				return (ip1 - c1) / (c2 - c1);
			}
		}
	}

	// Case not controlled
	return -1.0;
}





//=============================================================================
//
//	Class Evaluation_AIavg
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//----- Evaluate  -------------------------------------------------------------
Fitness * EvaluationFJSP_AIavg::evaluate(const SharedVars * const svars,
	Individual *individual) const {

	double ai, sum = 0.0;
	unsigned int nJobs;
	Solution * solution;
	FuzzySchedule * schedule;
	FuzzyProblem * fuzzyProb;

	fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Ealuation", errorMsg);
	}

	if (!fuzzyProb->hasDueDates()) {
		std::string errorMsg = "Agreement index cannot be computed over a ";
		errorMsg += "problem without due-dates";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Evaluate the individual to find the phenotype
	nJobs = fuzzyProb->getNumberJobs();
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < nJobs; i++) {
		ai = this->agreementIndex(fuzzyProb->getDueDate(i), schedule->getCTJob(i));
		sum += ai;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FitnessDouble(sum / nJobs, true);
}

}
