/*
 * CreationJSP_Seeded.h
 *
 *  Created on: Sep 21, 2026
 *      Author: hdiaz
 */
#pragma once

#include "CreationJSP_Base.h"
#include <string>
#include <vector>

namespace JSP {

	// Creation parameters defined in this header file
#define CREATION_SEED_POOL     "creation.seed.pool"
#define CREATION_SEED_COUNT    "creation.seed.count"
#define CREATION_SEED_OFFSET   "creation.seed.offset"
#define CREATION_SEED_POP_SIZE "population.size"


//=============================================================================
//
//	Class CreationSeededSchedule
//
//=============================================================================
/**
 * Seeds the initial population with job permutations read from a pool file
 * -- one solution per line, "j1 j2 ... jN;anything", jobs numbered from 1 --
 * and fills the rest at random. Ported from the seeding study on the
 * interval problem (branch seeding-study, CreationSeededSchedule): a job
 * permutation is a valid solution of any instance with the same machine
 * routing, so the pools built for the interval instances serve the crisp
 * ones unchanged.
 *
 * Only a full-population creation is seeded, that is, a request for exactly
 * population.size individuals; any smaller request -- the ABC's scout
 * replacements -- is served at random, so the run counter is not advanced
 * during the evolution.
 *
 * Run r with k seeds takes lines [(r*k) mod L, (r*k+k) mod L) of the pool,
 * wrapping around, L being the pool size. creation.seed.offset shifts r so
 * that a set of runs split across processes draws the same lines as the
 * same runs in one process.
 *
 * @author hdiaz
 *
 */
	class CreationSeededSchedule : public CreationRandomSchedule {
	protected:
		//=========================================================================
		//		COMMON FIELDS
		//=========================================================================
		unsigned int seedCount;                    // seeds per full population
		unsigned int popSize;                      // size of a full population
		unsigned int seedOffset;                   // global index of this process' first run
		mutable unsigned int runCounter;           // full populations created so far
		std::vector< std::vector<int> > seedJobs;  // job sequences of the pool, 1-based


		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		explicit CreationSeededSchedule(FuzzyFW::ParameterDB *parameters = NULL)
			: CreationRandomSchedule(parameters), seedCount(0), popSize(0),
			  seedOffset(0), runCounter(0) { }

		CreationSeededSchedule(const CreationSeededSchedule &source)
			: CreationRandomSchedule(source), seedCount(source.seedCount),
			  popSize(source.popSize), seedOffset(source.seedOffset),
			  runCounter(0), seedJobs(source.seedJobs) { }

		/**
		* Loads the SGS as the random creation does, then the pool and the
		* seeding parameters
		*/
		virtual void setup(FuzzyFW::ParameterDB *parameters);

		virtual Creation * clone() const {
			return new CreationSeededSchedule(*this);
		}

		virtual ~CreationSeededSchedule() { }


		//=========================================================================
		//		METHODS
		//=========================================================================
	protected:
		/**
		* Builds an individual from a job sequence (jobs numbered from 1)
		*/
		FuzzyFW::Individual * buildFromJobPerm(
			const std::vector<int> &jobs,
			const FuzzyFW::SharedVarsEvolutionary *svars) const;

	public:
		/**
		* Creates a population: the first seed.count individuals from the pool
		* when a full population is requested, the rest at random
		*
		* @param popSize Number of individuals requested
		* @param svars Shared variables for the algorithm
		* @return The new population
		*/
		virtual FuzzyFW::Population * createPopulation(
			const unsigned int popSize,
			const FuzzyFW::SharedVarsEvolutionary *svars) const;

		/**
		* Get the name and setup of the operator
		*
		* @return A string of parameter values. The first string is the name of
		* the operator
		*/
		virtual std::vector<std::string> getName() const {
			std::vector<std::string> setup = this->buildStrategyName("Seeded");
			setup.push_back(";SeedCount:;" + valueToString(this->seedCount));
			setup.push_back(";PoolLines:;" + valueToString((int) this->seedJobs.size()));
			return setup;
		}
	};

}
