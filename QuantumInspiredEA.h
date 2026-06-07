/*
* QuantumInspiredEA.h
*
*  Quantum-inspired Evolutionary Algorithm (QEA) for the Interval Job Shop
*  Scheduling Problem.
*
*  Instead of evolving a population of concrete solutions, this algorithm
*  evolves a probability distribution (a matrix of "q-bit" amplitudes) over
*  the job-order genotype. Each generation it OBSERVES K samples from that
*  distribution, EVALUATES them reusing the framework's decoder/evaluator
*  (so interval ranking EV/LEX2 comes for free), and ROTATES the amplitudes
*  towards the best solution found so far.
*
*  This file only ADDS a new algorithm; it does not modify any existing
*  operator or algorithm.
*
*  Created on: June 6, 2026
*/
#pragma once


#include "EvolutiveAlgorithm.h"
#include "GeneticClassRegister.h"


namespace FuzzyFW {


// Parameters read from the configuration file
#define QEA_ENCODING       "codification"      // shared with the GA on purpose
#define QEA_DECODING       "decodification"
#define QEA_SAMPLES        "qea.samples"       // observations per generation (K)
#define QEA_ROTATION       "qea.rotation"      // rotation step (delta theta)
#define QEA_FLOOR          "qea.floor"         // minimum probability per symbol
#define QEA_SCHEME         "qea.scheme"        // "positional" (default) | "precedence"
#define QEA_ROT_START      "qea.rot_start"     // rotation at generation 0 (default = qea.rotation)
#define QEA_ROT_END        "qea.rot_end"       // rotation at last generation (default = qea.rotation)
#define QEA_FLOOR_START    "qea.floor_start"   // floor at generation 0 (default = qea.floor)
#define QEA_FLOOR_END      "qea.floor_end"     // floor at last generation (default = qea.floor)
#define QEA_SCHED_TYPE     "qea.schedule_type" // "linear" (default) | "cosine"

#define QEA_GENERATIONS    "generations"       // stopping criteria (reused)
#define QEA_TIME           "timelimit"
#define QEA_EVALUATIONS    "evaluations"
#define QEA_NOIMPROVE      "noimprovement"

#define QEA_EVOL_METRIC    "evolution.unit"
#define QEA_EVOL_UNIT_GEN  "iteration"
#define QEA_EVOL_UNIT_TIME "time"
#define QEA_EVOL_SPAN      "evolution.span"



//=============================================================================
//
//	Class QuantumInspiredEA
//
//=============================================================================
/**
* Quantum-inspired evolutionary algorithm working on the IJSP job-order
* genotype. See file header for the high-level description.
*/
class QuantumInspiredEA : public EvolutiveAlgorithm {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	//-----  CONFIGURATION FIELDS  ----------------------------------------
	/** Number of solutions observed (sampled) each generation */
	unsigned int samples;

	/** Rotation step applied to the amplitudes (radians) */
	double deltaTheta;

	/** Rotation schedule: linear ramp between rotStart and rotEnd along the
	 *  generation budget. If both equal deltaTheta -> constant (baseline). */
	double rotStart;
	double rotEnd;

	/** Minimum probability kept for every symbol to preserve exploration */
	double floorProb;

	/** Floor schedule: ramp between floorStart and floorEnd along the
	 *  generation budget. If both equal floorProb -> constant (baseline). */
	double floorStart;
	double floorEnd;

	/** Schedule shape: false = linear ramp (baseline); true = cosine
	 *  annealing (smooth ease-in/ease-out). Affects both rotation and floor. */
	bool cosineSchedule;

	/** Search-space model: false = positional amplitude[pos][job];
	 *  true = precedence model pref[a][b] (invariant to absolute position) */
	bool usePrecedence;

	/** Stopping criteria */
	int maxGenerations;
	int maxPlateau;
	long int maxEvaluations;
	double maxRuntime;

	/** Show the evolution by generations or by time */
	bool showEvolutionTime;
	double evolutionSpan;


	//-----  DYNAMIC FIELDS  ----------------------------------------------
	unsigned int generation;
	unsigned int iterationsNI;
	long int evaluations;
	Individual *bestSoFar;

	/** Positional model: amplitude[position][job], norm 1 per position.
	 *  Probability of placing job j at position p is amplitude[p][j]^2. */
	std::vector< std::vector<double> > amplitude;

	/** Precedence model: pref[a][b] = probability that job a precedes job b.
	 *  Full matrix kept consistent (pref[b][a] = 1 - pref[a][b]). */
	std::vector< std::vector<double> > pref;

	/** Problem dimensions cached at the beginning of run() */
	int nJobs;
	int genotypeLength;
	std::vector<int> tasksPerJob;

	/** Runtimes */
	clock_t totalRuntime;
	clock_t observationTime;
	clock_t evaluationTime;
	clock_t rotationTime;

	/** Evolution data for statistics */
	std::vector< std::vector<double> > evolutionStats;
	double nextSplit;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/** Constructor using the parameters file (does not initialise objects) */
	explicit QuantumInspiredEA(ParameterDB *params = NULL);

	/** Destructor */
	virtual ~QuantumInspiredEA();

protected:
	/** Destroys everything specific to this algorithm */
	virtual void clearAll();



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	virtual void printSetupTree(std::ofstream & output) const;

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
	/** Validates that the algorithm is correctly configured */
	virtual bool checkSetup();

	/** Indicates if any stopping criterion has been met */
	virtual bool stop();

	/** Stores the statistical values for the current generation */
	virtual void computeStatistics(Population *currentPopulation);


	//-----  QEA-specific operators  --------------------------------------
	/** Samples one valid job-order genotype from the current model, repairing
	 *  on the fly so every job appears its required number of times.
	 *  Dispatches to the positional or precedence variant. */
	std::vector<int> observe();

	/** Rotates the model towards the best solution with the default step.
	 *  Dispatches by scheme. */
	void rotateTowards(const std::vector<int> &bestGenotype);

	/** Returns the rotation step for the current generation (schedule-aware). */
	double currentRotation() const;

	/** Returns the floor probability for the current generation (schedule-aware). */
	double currentFloor() const;

	/** Keeps a minimum probability to avoid premature collapse. Dispatches. */
	void applyFloor();

	//-----  Positional scheme: amplitude[position][job]  -----------------
	std::vector<int> observePositional();
	void rotatePositional(const std::vector<int> &bestGenotype, double step);
	void applyFloorPositional();

	//-----  Precedence scheme: pref[a][b] = P(job a precedes job b)  -----
	std::vector<int> observePrecedence();
	void rotatePrecedence(const std::vector<int> &bestGenotype, double step);
	void applyFloorPrecedence();
};


}
