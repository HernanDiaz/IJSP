/*
 * CrossoverJSP_JOX.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverJSP_JOX.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace JSP {

//=============================================================================
//
//	Class Crossover_JOX
//
//=============================================================================
//-----  I-039 mask counters, printed to stderr at exit (mechanism check)  ---
namespace {
	unsigned long joxCrossings = 0;
	double joxKeepSum = 0.0;
	double joxDeviationSum = 0.0;

	void printJoxCounters() {
		std::fprintf(stderr, "JOX crossings %lu, mean keep %.3f, "
			"mean deviation %.3f%s", joxCrossings,
			joxCrossings ? joxKeepSum / joxCrossings : 0.0,
			joxCrossings ? joxDeviationSum / joxCrossings : 0.0, "\n");
	}
}

//=============================================================================
//		METHODS
//=============================================================================
//=====  Setup  ===============================================================
void Crossover_JOX::setup(FuzzyFW::ParameterDB *parameters) {
	CrossoverJSP_Base::setup(parameters);
	this->uniformMask =
		(parameters->getStringLower(JOX_MASK).compare("uniform") == 0);
	if (this->uniformMask)
		std::atexit(printJoxCounters);
}

//=====  Apply (Permutation)  =================================================
void Crossover_JOX::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	unsigned int count1, count2;
	int gene, job;
	bool different1, different2;
	std::vector<int> mask;
	std::vector<int> offs1, offs2;

	// Convert the problem type
	ProblemJSP * fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw JSPException("Creation", errorMsg);
	}

	// Choose the jobs to keep in position. I-039: with the uniform mask the
	// keep probability is drawn at every crossing; the default keeps 1/2 and
	// draws nothing extra, exactly as before.
	double keep = 0.5;
	if (this->uniformMask) {
		keep = svars->rng->getProbability();
		joxCrossings++;
		joxKeepSum += keep;
		joxDeviationSum += std::fabs(keep - 0.5);
	}
	mask.resize(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		if (svars->rng->getProbability() < 1.0 - keep)
			mask[i] = 0;
		else mask[i] = 1;
	}

	// Build the first offspring
	offs1.resize(ind1->size());

	different1 = false;
	count1 = count2 = 0;
	while (count1 < ind1->size()) {
		gene = ind1->getGene(count1);
		job = fuzzyProb->getTask(gene)->job;

		if (mask[job] == 1)
			offs1[count1] = gene;
		else {
			gene = ind2->getGene(count2);
			job = fuzzyProb->getTask(gene)->job;

			while (count2 < ind2->size() - 1 && mask[job] == 1) {
				count2++;
				gene = ind2->getGene(count2);
				job = fuzzyProb->getTask(gene)->job;
			}
			offs1[count1] = gene;
			count2++;
		}
		if (offs1[count1] != ind1->getGene(count1))
			different1 = true;
		count1++;
	}


	// Build the second offspring
	offs2.resize(ind2->size());

	different2 = false;
	count1 = count2 = 0;
	while (count1 < ind2->size()) {
		gene = ind2->getGene(count1);
		job = fuzzyProb->getTask(gene)->job;

		if (mask[job] == 1)
			offs2[count1] = gene;
		else {
			gene = ind1->getGene(count2);
			job = fuzzyProb->getTask(gene)->job;

			while (count2 < ind1->size() - 1 && mask[job] == 1) {
				count2++;
				gene = ind1->getGene(count2);
				job = fuzzyProb->getTask(gene)->job;
			}
			offs2[count1] = gene;
			count2++;
		}
		if (offs2[count1] != ind2->getGene(count1))
			different2 = true;
		count1++;
	}

	if (different1)
		ind1->setGenotype(offs1);
	if (different2)
		ind2->setGenotype(offs2);
}



//=====  Apply (Permutation with Repetitions)  ================================
void Crossover_JOX::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	unsigned int count1, count2;
	int gene;
	bool different1, different2;
	std::vector<int> mask;
	std::vector<int> offs1, offs2;


	// Convert the problem type
	ProblemJSP * fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw JSPException("Creation", errorMsg);
	}

	// Choose the jobs to keep in position. I-039: with the uniform mask the
	// keep probability is drawn at every crossing; the default keeps 1/2 and
	// draws nothing extra, exactly as before.
	double keep = 0.5;
	if (this->uniformMask) {
		keep = svars->rng->getProbability();
		joxCrossings++;
		joxKeepSum += keep;
		joxDeviationSum += std::fabs(keep - 0.5);
	}
	mask.resize(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		if (svars->rng->getProbability() < 1.0 - keep)
			mask[i] = 0;
		else mask[i] = 1;
	}

	// Build the first offspring
	offs1.resize(ind1->size());

	different1 = false;
	count1 = count2 = 0;
	while (count1 < ind1->size()) {
		gene = ind1->getGene(count1);

		if (mask[gene] == 1)
			offs1[count1] = gene;
		else {
			gene = ind2->getGene(count2);

			while (count2 < ind2->size() - 1 && mask[gene] == 1) {
				count2++;
				gene = ind2->getGene(count2);
			}
			offs1[count1] = gene;
			count2++;
		}
		if (offs1[count1] != ind1->getGene(count1))
			different1 = true;
		count1++;
	}


	// Build the second offspring
	offs2.resize(ind2->size());

	different2 = false;
	count1 = count2 = 0;
	while (count1 < ind2->size()) {
		gene = ind2->getGene(count1);

		if (mask[gene] == 1)
			offs2[count1] = gene;
		else {
			gene = ind1->getGene(count2);

			while (count2 < ind1->size() - 1 && mask[gene] == 1) {
				count2++;
				gene = ind1->getGene(count2);
			}
			offs2[count1] = gene;
			count2++;
		}
		if (offs2[count1] != ind2->getGene(count1))
			different2 = true;
		count1++;
	}

	if (different1)
		ind1->setGenotype(offs1);
	if (different2)
		ind2->setGenotype(offs2);
}

} // namespace JSP
