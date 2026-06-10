/*
 * IPRTS.h
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */
#pragma once

#include "EvolutiveAlgorithm.h"
#include "GeneticClassRegister.h"
#include "LocalSearchClassRegister.h"
#include "ElitePool.h"
#include "PathRelinkIJSP.h"

namespace FuzzyFW {

/* Parameters for IPRTS (names shared with GA/MA setups where they exist):
*
*	- codification / decodification : encoder and decoder
*	- creation : strategy used to seed (and re-seed) the elite pool
*	- localsearch / localsearch.* : tabu search applied to every seed and to
*			every path relinking product (all LS_Tabu parameters apply)
*	- localsearch.lamarckism : write the improved schedule back to the genotype
*	- pool.size : number of elite solutions
*	- pool.min-distance : minimum pairwise distance between elites, as a
*			fraction of the total disjunctive pairs of the instance
*	- pool.restart-patience : after this many consecutive cycles without any
*			pool insertion, the worst half of the pool is re-seeded (scout
*			phase analogue; -1 disables it)
*	- pool.restart-perturbation : refill restarted slots by perturbing
*			surviving elites (extract+reinsert genotype moves, rebuilt
*			through the decoder) instead of creating fresh seeds; perturbed
*			material starts at elite level, so it can actually re-enter the
*			pool and re-diversify a saturated search
*	- pool.perturbation-strength : perturbation moves as a fraction of the
*			genotype length
*	- pool.seed-retries : creation attempts per pool slot before relaxing the
*			diversity rule during (re-)seeding
*	- pr.neighbourhood / pr.max-steps : see PathRelinkIJSP
*	- timelimit / noimprovement / generations / evaluations : stopping criteria
*			(noimprovement and generations count PR cycles)
*	- evolution.unit / evolution.span : evolution reporting, as in GA
*/
#define IPRTS_ENCODING "codification"
#define IPRTS_DECODING "decodification"
#define IPRTS_CREATION "creation"

#define IPRTS_LOCAL_SEARCH "localsearch"
#define IPRTS_LS_NEIGHBOURHOOD "localsearch.neighbourhood"
#define IPRTS_LS_LAMARCKISM "localsearch.lamarckism"

#define IPRTS_SEED_RETRIES "pool.seed-retries"
#define IPRTS_RESTART_PATIENCE "pool.restart-patience"
#define IPRTS_RESTART_PERTURB "pool.restart-perturbation"
#define IPRTS_PERTURB_STRENGTH "pool.perturbation-strength"
#define IPRTS_LS_ROUNDS "localsearch.max-rounds"

#define IPRTS_CYCLES "generations"
#define IPRTS_TIME "timelimit"
#define IPRTS_EVALUATIONS "evaluations"
#define IPRTS_NOIMPROVE "noimprovement"

#define IPRTS_EVOL_METRIC "evolution.unit"
#define IPRTS_EVOL_UNIT_GEN "iteration"
#define IPRTS_EVOL_UNIT_TIME "time"
#define IPRTS_EVOL_SPAN "evolution.span"


//=============================================================================
//
//	Class IPRTS
//
//=============================================================================
/**
* Interval-aware Path Relinking Tabu Search.
*
* Elite-pool algorithm for the IJSP: the pool is seeded with creation
* heuristics improved by tabu search; each cycle relinks a random elite
* towards a better one along extreme critical arcs (PathRelinkIJSP), improves
* the best intermediate with tabu search and inserts the result back into the
* pool under a quality + diversity rule. When the pool converges, the worst
* half is re-seeded.
*
* @author hdiaz
*/
class IPRTS : public EvolutiveAlgorithm {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	//-----  CONFIGURATION FIELDS  ------------------------
	Creation *creation;

	LocalSearch *localSearch;
	bool lsLamarckism;

	/**
	* Maximum number of consecutive tabu chains per individual: each chain
	* restarts from the previous result with a fresh tabu list and runs while
	* the solution keeps improving (LS_Tabu exits its chain on a dead end,
	* which is frequent with small filtered neighbourhoods)
	*/
	int lsMaxRounds;

	IJSP::PathRelinkIJSP *pathRelinking;

	IJSP::ElitePool pool;
	unsigned int poolSize;
	double poolMinDistance;
	int restartPatience;
	bool restartPerturbation;
	double perturbationStrength;
	int seedRetries;

	/**
	* Stopping criteria
	*/
	int maxCycles;
	int maxPlateau;
	long int maxEvaluations;
	double maxRuntime;

	/**
	* Evolution reporting
	*/
	bool showEvolutionTime;
	double evolutionSpan;


	//-----  DYNAMIC FIELDS  ------------------------------
	unsigned int cycles;
	unsigned int cyclesNI;
	unsigned long evaluations;
	unsigned long prSteps;
	unsigned long prCalls;
	unsigned long lsCalls;
	unsigned long poolInsertions;
	unsigned long poolRestarts;
	unsigned int cyclesSinceInsertion;
	Individual *bestSoFar;

	/**
	* Runtimes
	*/
	clock_t totalRuntime;
	clock_t creationTime;
	clock_t localSearchTime;
	clock_t pathRelinkingTime;

	/**
	* Evolution data for statistics
	*/
	std::vector< std::vector<double> > evolutionStats;
	double nextSplit;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit IPRTS(ParameterDB *params = NULL);

	virtual ~IPRTS();

protected:
	virtual void clearAll();


	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	virtual void printSetupTree(std::ofstream &output) const;

	virtual std::vector< std::pair<std::string, double> > getStatistics() const;

	virtual std::vector< std::pair<std::string, double> > getRuntime() const;

	virtual std::vector< std::vector<double> > getEvolution(
		std::vector< std::string > &labels) const;


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual void prepareToRun(ParameterDB *params = NULL);

	virtual std::pair<Solution *, Objective *> run(Problem *problem,
		std::string signature, std::string logFolder, int rngSeed);

protected:
	virtual bool checkSetup();

	/**
	* Indicates if the stopping criteria has been met
	*/
	virtual bool stop();

	/**
	* Applies the tabu search to one individual (lamarckism aware)
	*/
	void applyLocalSearch(Individual *individual);

	/**
	* Creates and evaluates one fresh individual from the creation strategy
	*/
	Individual * newSeedIndividual();

	/**
	* Fills the pool up to poolSize with TS-improved seeds (diversity rule
	* first, relaxed after seedRetries failures per free slot)
	*/
	void fillPool();

	/**
	* Refills restarted slots with TS-improved perturbations of surviving
	* elites; falls back to fillPool for any slot left
	*/
	void refillByPerturbation();

	/**
	* Clone of a random elite with extract+reinsert genotype moves applied,
	* re-decoded and re-evaluated (NULL if the genotype is not an int array)
	*/
	Individual * perturbedElite();

	/**
	* Stores the statistical values in each time/cycle span
	*/
	void computeStatistics();
};

}
