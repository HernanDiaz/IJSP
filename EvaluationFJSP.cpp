/*
* EvaluationFJSP.cpp
*
*  Created on: July 10, 2017
*/

#include "EvaluationFJSP.h"

namespace FJSP {


//=============================================================================
//
//	Class EvaluationFJSP_Makespan
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationFJSP_Makespan::EvaluationFJSP_Makespan(
	FuzzyFW::ParameterDB *parameters)
	: maximumLabel(FJSP_EVALUATION_MAXIMUM),
	tfnMaximum(FuzzyFW::TFN::M_COMPONENT),
	compareLabel(FJSP_EVALUATION_COMPARE), tfnCompare(FuzzyFW::TFN::C_EV),
	Evaluation(parameters) {
	FJSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationFJSP_Makespan::EvaluationFJSP_Makespan(
	const EvaluationFJSP_Makespan & source)
	: maximumLabel(source.maximumLabel), tfnMaximum(source.tfnMaximum),
	compareLabel(source.compareLabel), tfnCompare(source.tfnCompare),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationFJSP_Makespan::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string compareName, maxName;

	// Load maximum type parameter
	maxName = parameters->getString(this->maximumLabel);
	if (maxName.length() == 0) {
		std::string errorMsg = this->maximumLabel + " parameter not found.";
		throw new FJSPException("Evaluation", errorMsg);
	}
	this->tfnMaximum = FuzzyFW::TFN::getMaximum(maxName);
	if (this->tfnMaximum == FuzzyFW::TFN::M_Err) {
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
	this->tfnCompare = FuzzyFW::TFN::getComparison(compareName);
	if (this->tfnCompare == FuzzyFW::TFN::C_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += compareName + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}
	FuzzyFW::FitnessTFN::FitnessCompareStrategy = this->tfnCompare;
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationFJSP_Makespan::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP *fuzzyProb;
	FuzzyFW::TFN makespan = FuzzyFW::TFN(0, 0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Ealuation", errorMsg);
	}

	// Compute the makespan
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		makespan = maximum(makespan, schedule->getCTJob(i), this->tfnMaximum);
	}

	return new FuzzyFW::FitnessTFN(makespan, false);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_Makespan::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP *fuzzyProb;
	FuzzyFW::TFN makespan = FuzzyFW::TFN(0, 0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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
	return new FuzzyFW::FitnessTFN(makespan, false);
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
EvaluationFJSP_AImin::EvaluationFJSP_AImin(FuzzyFW::ParameterDB *parameters)
	: exactLabel(FJSP_EVALUATION_AI), isExact(true),
	Evaluation(parameters) {
	FJSPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationFJSP_AImin::EvaluationFJSP_AImin(const EvaluationFJSP_AImin & source)
	: exactLabel(source.exactLabel), isExact(source.isExact),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationFJSP_AImin::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string exactName;

	// Load maximum type parameter
	exactName = parameters->getStringUpper(this->exactLabel);
	if (exactName.length() == 0) {
		std::string errorMsg = this->exactLabel + " parameter not found.";
		throw new FJSPException("Evaluation", errorMsg);
	}
	if (exactName.compare(toUpper(FJSP_EVAL_AI_EXACT)) == 0)
		this->isExact = true;
	else if (exactName.compare(toUpper(FJSP_EVAL_AI_APROX)) == 0)
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
//----- Get the Objective Function  -------------------------------------------
FuzzyFW::Objective * EvaluationFJSP_AImin::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double ai, minAI = 1.0;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		ai = this->agreementIndex(fuzzyProb->getTimeWindow(i),
			schedule->getCTJob(i), this->isExact);
		if (compareDouble(ai, minAI) < 0)
			minAI = ai;
	}

	return new FuzzyFW::FitnessDouble(minAI, true);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_AImin::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double ai, minAI = 1.0;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		ai = this->agreementIndex(fuzzyProb->getTimeWindow(i),
			schedule->getCTJob(i), this->isExact);
		if (compareDouble(ai, minAI) < 0)
			minAI = ai;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FuzzyFW::FitnessDouble(minAI, true);
}



//----- Agreement Index  ------------------------------------------------------
double EvaluationFJSP_AImin::agreementIndex(
	const FuzzyFW::TimeWindow * const tw,
	const FuzzyFW::TFN & completionTime, const bool exact) {

	if (exact)
		return tw->agreementIndex(completionTime);

	double c1 = completionTime.a;
	double c2 = completionTime.b;
	double c3 = completionTime.c;
	double d1, d2;

	if (tw->getType() == FuzzyFW::TimeWindow::Type::DEADLINE) {
		const FuzzyFW::TimeWindowDeadline * ddl =
			dynamic_cast<const FuzzyFW::TimeWindowDeadline *>(tw);
		d1 = ddl->timeLimit;

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
	if (tw->getType() == FuzzyFW::TimeWindow::Type::LINEAR) {
		const FuzzyFW::TimeWindowLinear * ddl =
			dynamic_cast<const FuzzyFW::TimeWindowLinear *>(tw);
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
//	Class EvaluationFJSP_AImin_ICAE
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_AImin_ICAE::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double ai;
	std::list<double> AIlist;
	std::list<double>::iterator listIter;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;
	FuzzyFW::FitnessLexicographic * fitness;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the list of AI values
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		ai = this->agreementIndex(fuzzyProb->getTimeWindow(i),
			schedule->getCTJob(i), this->isExact);
		// Find the insertion position inside the list
		listIter = AIlist.begin();
		while (listIter != AIlist.end() && ai > *listIter)
			listIter++;
		AIlist.insert(listIter, ai);
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());

	// Create the lexicographical fitness function
	fitness = new FuzzyFW::FitnessLexicographic();
	for (listIter = AIlist.begin(); listIter != AIlist.end(); listIter++)
		fitness->addFitness(new FuzzyFW::FitnessDouble(*listIter, true));
	return fitness;
}





//=============================================================================
//
//	Class Evaluation_AIavg
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective Function  -----------------------------------------------
FuzzyFW::Objective * EvaluationFJSP_AIavg::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double ai, sum = 0.0;
	unsigned int nJobs;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the average AI
	for (unsigned int i = 0; i < nJobs; i++) {
		ai = this->agreementIndex(fuzzyProb->getTimeWindow(i),
			schedule->getCTJob(i), this->isExact);
		sum += ai;
	}

	return new FuzzyFW::FitnessDouble(sum / nJobs, true);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_AIavg::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double ai, sum = 0.0;
	unsigned int nJobs;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the average AI
	for (unsigned int i = 0; i < nJobs; i++) {
		ai = this->agreementIndex(fuzzyProb->getTimeWindow(i),
			schedule->getCTJob(i), this->isExact);
		sum += ai;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FuzzyFW::FitnessDouble(sum / nJobs, true);
}





//=============================================================================
//
//	Class EvaluationFJSP_ESDmin
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationFJSP_ESDmin::EvaluationFJSP_ESDmin(FuzzyFW::ParameterDB *parameters)
	: Evaluation(parameters) {
	FJSPClassRegister::registerClasses();
}


//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective Function  -----------------------------------------------
FuzzyFW::Objective * EvaluationFJSP_ESDmin::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double esd, minESD = 1.0;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		esd = fuzzyProb->getTimeWindow(i)->agreementIndex(
			schedule->getCTJob(i).expectedValue());
		if (compareDouble(esd, minESD) < 0)
			minESD = esd;
	}

	return new FuzzyFW::FitnessDouble(minESD, true);
}



//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_ESDmin::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double esd, minESD = 1.0;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		esd = fuzzyProb->getTimeWindow(i)->agreementIndex(
			schedule->getCTJob(i).expectedValue());
		if (compareDouble(esd, minESD) < 0)
			minESD = esd;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FuzzyFW::FitnessDouble(minESD, true);
}





//=============================================================================
//
//	Class EvaluationFJSP_ESDavg
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective Function  -----------------------------------------------
FuzzyFW::Objective * EvaluationFJSP_ESDavg::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double esd, sum = 0.0;
	unsigned int nJobs;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < nJobs; i++) {
		esd = fuzzyProb->getTimeWindow(i)->agreementIndex(
			schedule->getCTJob(i).expectedValue());
		sum += esd;
	}

	return new FuzzyFW::FitnessDouble(sum / nJobs, true);
}


//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFJSP_ESDavg::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	double esd, sum = 0.0;
	unsigned int nJobs;
	FuzzyFW::Solution * solution;
	ScheduleFJSP * schedule;
	ProblemFJSP * fuzzyProb;

	fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
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

	schedule = dynamic_cast<ScheduleFJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy Job Shop Problems.";
		throw new FJSPException("Evaluation", errorMsg);
	}

	// Compute the minimum AI
	for (unsigned int i = 0; i < nJobs; i++) {
		esd = fuzzyProb->getTimeWindow(i)->agreementIndex(
			schedule->getCTJob(i).expectedValue());
		sum += esd;
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());
	return new FuzzyFW::FitnessDouble(sum / nJobs, true);
}


}
