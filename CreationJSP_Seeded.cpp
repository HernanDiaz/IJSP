/*
 * CreationJSP_Seeded.cpp
 *
 *  Created on: Sep 21, 2026
 *      Author: hdiaz
 */

#include "CreationJSP_Seeded.h"
#include <fstream>
#include <sstream>

namespace JSP {

//=============================================================================
//
//	Class CreationSeededSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationSeededSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	// The SGS, exactly as the random creation loads it
	CreationRandomSchedule::setup(parameters);

	// Seeding parameters. The offset is the global index of this process'
	// first run: 0 unless a set of runs was split across processes.
	this->seedCount = (unsigned int) parameters->getInteger(CREATION_SEED_COUNT, 0);
	this->popSize = (unsigned int) parameters->getInteger(CREATION_SEED_POP_SIZE, 0);
	this->seedOffset = (unsigned int) parameters->getInteger(CREATION_SEED_OFFSET, 0);

	std::string poolPath = parameters->getString(CREATION_SEED_POOL);
	if (poolPath.length() == 0) {
		std::string errorMsg = "Parameter '" CREATION_SEED_POOL "' not found";
		errorMsg += " for creation = jsp.seeded";
		throw JSPException("Creation", errorMsg);
	}

	std::ifstream in(poolPath.c_str());
	if (!in.is_open()) {
		std::string errorMsg = "Cannot open the seed pool '" + poolPath + "'";
		throw JSPException("Creation", errorMsg);
	}
	this->seedJobs.clear();
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty())
			continue;
		// Everything after the first ';' is the pool's own annotation of the
		// solution (its makespan on the instance it was built for) and is not
		// read: the permutation is re-decoded here.
		size_t sc = line.find(';');
		std::stringstream ss(sc == std::string::npos ? line : line.substr(0, sc));
		std::vector<int> jobs;
		int v;
		while (ss >> v)
			jobs.push_back(v);
		if (!jobs.empty())
			this->seedJobs.push_back(jobs);
	}
	in.close();

	if (this->seedJobs.empty()) {
		std::string errorMsg = "The seed pool '" + poolPath + "' holds no permutations";
		throw JSPException("Creation", errorMsg);
	}
}


//=============================================================================
//		METHODS
//=============================================================================
//-----  buildFromJobPerm  ----------------------------------------------------
FuzzyFW::Individual * CreationSeededSchedule::buildFromJobPerm(
	const std::vector<int> &jobs,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	// Convert the problem type
	ProblemJSP * fuzzyProb = dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This creation works only with job shop problems.";
		throw JSPException("Creation", errorMsg);
	}

	// A permutation must name every task of the instance exactly once
	unsigned int nJobs = fuzzyProb->getNumberJobs();
	size_t nTasks = 0;
	for (unsigned int i = 0; i < nJobs; i++)
		nTasks += fuzzyProb->getNumberTasks(i);
	if (jobs.size() != nTasks) {
		std::string errorMsg = "A seed of " + valueToString((int) jobs.size());
		errorMsg += " jobs for an instance of " + valueToString((int) nTasks) + " tasks";
		throw JSPException("Creation", errorMsg);
	}

	// Job sequence, numbered from 1 -> permutation of internal task ids
	std::vector<int> occurrence(nJobs, 0), permutation;
	permutation.reserve(jobs.size());
	for (size_t i = 0; i < jobs.size(); i++) {
		int job = jobs[i] - 1;
		if (job < 0 || job >= (int) nJobs) {
			std::string errorMsg = "Job number out of range in the seed pool: ";
			errorMsg += valueToString(jobs[i]);
			throw JSPException("Creation", errorMsg);
		}
		int task = fuzzyProb->getTaskId(job, occurrence[job]);
		if (task < 0) {
			std::string errorMsg = "Job " + valueToString(jobs[i]);
			errorMsg += " appears more times in a seed than it has tasks";
			throw JSPException("Creation", errorMsg);
		}
		occurrence[job]++;
		permutation.push_back(task);
	}

	// Same genesis as the random creation, with the given permutation
	this->sgs->buildSchedule(svars, permutation);
	FuzzyFW::Individual * indiv = svars->encoder->encode(this->sgs->getSchedule(), svars);
	indiv->updatePhenotype(this->sgs->getSchedule()->clone());
	return indiv;
}


//-----  create Population  ---------------------------------------------------
FuzzyFW::Population * CreationSeededSchedule::createPopulation(
	const unsigned int reqSize,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	// A partial request (the scouts) or seeding switched off: all random
	if (reqSize != this->popSize || this->seedCount == 0 || this->seedJobs.empty())
		return CreationRandomSchedule::createPopulation(reqSize, svars);

	FuzzyFW::Population * population = new FuzzyFW::Population();
	unsigned int L = (unsigned int) this->seedJobs.size();
	unsigned int nSeed = (this->seedCount < reqSize) ? this->seedCount : reqSize;
	// The block depends on the global run index, not on this process' own
	// counter, so that runs split across processes draw the same lines
	unsigned int globalRun = this->seedOffset + this->runCounter;
	unsigned int blockStart = (globalRun * this->seedCount) % L;

	for (unsigned int i = 0; i < reqSize; i++) {
		FuzzyFW::Individual * indiv;
		if (i < nSeed)
			indiv = this->buildFromJobPerm(this->seedJobs[(blockStart + i) % L], svars);
		else
			indiv = this->createIndividual(svars);
		indiv->id = i;
		population->addIndividual(indiv);
	}

	this->runCounter++;
	return population;
}

}
