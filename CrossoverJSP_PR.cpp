/*
 * CrossoverJSP_PR.cpp
 *
 *  Created on: Sep 24, 2026
 *      Author: hdiaz
 */

#include "CrossoverJSP_PR.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace JSP {

//-----  crossing counters, printed to stderr at exit (mechanism check)  ----
namespace {
	unsigned long prCrossings = 0;
	double prParentSum = 0.0;
	double prChildSum = 0.0;

	void printCounters() {
		std::fprintf(stderr, "PR crossings %lu, mean parent distance %.1f, "
			"mean child distance %.1f\n", prCrossings,
			prCrossings ? prParentSum / prCrossings : 0.0,
			prCrossings ? prChildSum / prCrossings : 0.0);
	}

	// The operations of a job-sequence genotype, in genotype order: the k-th
	// occurrence of job j is operation first[j] + k
	std::vector<unsigned int> operations(const FuzzyFW::IndividualArrayInt *ind,
		const std::vector<unsigned int> &first) {
		std::vector<unsigned int> seen(first.size(), 0), ops(ind->size());
		for (unsigned int p = 0; p < ind->size(); p++) {
			int job = ind->getGene(p);
			ops[p] = first[job] + seen[job]++;
		}
		return ops;
	}

	// Kendall distances between orders are pairwise; the interpolation keeps
	// every pair both parents agree on and orders the others by where they sit
	struct ByKey {
		const std::vector<double> *key;
		bool operator()(unsigned int a, unsigned int b) const {
			return (*key)[a] < (*key)[b];
		}
	};
}

//=============================================================================
//
//	Class Crossover_PR
//
//=============================================================================
//=====  Apply (Permutation)  =================================================
void Crossover_PR::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	throw JSPException("Crossover",
		"Path relinking is only implemented for the job-order encoding");
}


//=====  Apply (Permutation with Repetitions)  ================================
void Crossover_PR::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	const unsigned int n = ind1->size();

	// Operation numbering, from how many times each job appears
	int maxJob = 0;
	for (unsigned int p = 0; p < n; p++)
		maxJob = std::max(maxJob, ind1->getGene(p));
	std::vector<unsigned int> count(maxJob + 1, 0), first(maxJob + 1, 0);
	for (unsigned int p = 0; p < n; p++)
		count[ind1->getGene(p)]++;
	for (int j = 1; j <= maxJob; j++)
		first[j] = first[j - 1] + count[j - 1];

	const std::vector<unsigned int> opsA = operations(ind1, first);
	const std::vector<unsigned int> opsB = operations(ind2, first);
	std::vector<double> posA(n), posB(n);
	std::vector<int> job(n);
	unsigned int parentDistance = 0;
	for (unsigned int p = 0; p < n; p++) {
		posA[opsA[p]] = p;
		posB[opsB[p]] = p;
		job[opsA[p]] = ind1->getGene(p);
		if (ind1->getGene(p) != ind2->getGene(p))
			parentDistance++;
	}
	if (parentDistance == 0)
		return;

	static bool registered = false;
	if (!registered) {
		std::atexit(printCounters);
		registered = true;
	}

	// The two points of the path between the parents, one third and two
	// thirds of the way from the first to the second, fixed in advance.
	// Ties keep the first parent's order (stable sort over its sequence).
	const double alpha[2] = { 1.0 / 3.0, 2.0 / 3.0 };
	std::vector<int> offs[2];
	std::vector<double> key(n);
	for (int c = 0; c < 2; c++) {
		for (unsigned int o = 0; o < n; o++)
			key[o] = (1.0 - alpha[c]) * posA[o] + alpha[c] * posB[o];
		std::vector<unsigned int> order(opsA);
		ByKey byKey;
		byKey.key = &key;
		std::stable_sort(order.begin(), order.end(), byKey);
		offs[c].resize(n);
		for (unsigned int p = 0; p < n; p++)
			offs[c][p] = job[order[p]];
	}

	unsigned int childDistance = 0;
	for (unsigned int p = 0; p < n; p++)
		if (offs[0][p] != ind1->getGene(p))
			childDistance++;
	prCrossings++;
	prParentSum += parentDistance;
	prChildSum += childDistance;

	ind1->setGenotype(offs[0]);
	ind2->setGenotype(offs[1]);
}

} // namespace JSP
