/*
* ArtificialBeeColony.h
*
*  Created on: May 22, 2021
*      Author: hdiaz
*/
#pragma once


#include "GeneticAlgorithm.h"
#include "LocalSearchClassRegister.h"
#include "NonMonSimulatedCoolingClassRegister.h"



namespace FuzzyFW {

	/* Parameters for Memetic Algorithms:
	*
	*	-localsearch : Indicates the local search algorithm to use
	*	-localsearch.neighbourhood : Indiciates the neighbourhood to use
	* 	-frequency : Indicates when the hill climbing must be applied.This
	* 			parameter can take three values : initial, final and period.
	* 			If set to initial, hill climbing will be applied only to the
	* 			initial population.If it is final, only to the last population.
	* 			If it is period, it will be applied every "period-freq" iterations
	* 	-period - freq : If the previous option is "period", we must specify the
	* 			number of iterations between different hill - climbing
	* 	-target : best, worst, percentage value.Indicates to which solutions in
	* 			the population apply the hill climbing
	*/
#define MA_LOCAL_SEARCH "localsearch"
#define MA_LOCAL_SEARCH_LAMARCKISM "localsearch.lamarckism"

#define MA_LOCAL_SEARCH_NEIGHBOURHOOD "localsearch.neighbourhood"

#define MA_LOCAL_SEARCH_FREQ "localsearch.frequency"
#define MA_LOCAL_SEARCH_PERIOD "localsearch.period"
#define MA_LOCAL_SEARCH_TARGET "localsearch.target"

#define MA_LS_FREQ_NONE "none"
#define MA_LS_FREQ_INITIAL "initial"
#define MA_LS_FREQ_FINAL "final"
#define MA_LS_FREQ_PERIOD "period"
#define MA_LS_FREQ_STUCK "stuck"

#define MA_LS_TARGET_BEST "best"
#define MA_LS_TARGET_WORST "worst"
#define MA_LS_TARGET_ALL "all"

#define MAX_NUM_TRIALS "maxnumtrials" //maximum number of trials for a food source
#define ELITE_SELECTION "elite.selection"  //Selection mechanism to select the elite for crossing.
#define	ELITE_SIZE	"elite.size" //Selection mechanism to select number of elements in elite
// I-003: what replaces an exhausted food source in the scout step.
//   "random" (default, the classical ABC): a freshly created solution
//   "kick"  : a clone of a random elite with abc.scout.kicks mutations applied
#define SCOUT_MODE  "abc.scout"        //random | kick
#define SCOUT_KICKS "abc.scout.kicks"  //mutations applied to the cloned elite
#define SCOUT_MODE_KICK "kick"
// I-009: one deep tabu call on the incumbent when the run stalls.
//   abc.deepls = N  fires the first time N generations pass without improving
//   the global best, once per run, with the depth below. 0 or absent is off.
#define DEEP_LS_TRIGGER "abc.deepls"
// Depth of a deep call, in consecutive non-improving moves, against the 15 of
// the frozen setup. Sized by measurement, not taste: a tabu iteration costs
// about 1.7 us here because the pruning evaluates one or two neighbours, so a
// run's shallow calls add up to roughly 20 million iterations. A thousand-move
// trajectory is 0.005 % of that and cannot matter; thirty thousand moves at
// about 50 ms a call, repeated until the quota below is spent, is a fifth of
// the search. This is TSAB territory, which crosses worsening regions for
// hundreds or thousands of moves.
#define DEEP_LS_DEPTH 30000         // fixed in advance, never tuned
// I-010: the deep call gets a SHARE of the local-search effort instead of one
// shot per run. I-009 fired once and that was 0.06 % to 2.3 % of the tabu
// work (measured 2026-09-22), far too little to move anything. This fires on
// every stagnation episode while the time spent in deep calls stays under
// abc.deepls.share percent of all local-search time, so the mechanism gets a
// real share and the cost is self-limiting.
#define DEEP_LS_SHARE "abc.deepls.share" 


	//=============================================================================
	//
	//	Class ArtificialBeeColony
	//
	//=============================================================================
	/**
	*
	* @author hdiaz
	*
	*/
	class ArtificialBeeColonyPSO : public GeneticAlgorithm {
	protected:
		/** Inert counters, reported as statistics (2026-09-21). */
		unsigned int plateauVetoesCross;
		unsigned int plateauVetoesLS;

		/** I-003: kick an elite instead of injecting a random scout. */
		bool scoutKick;
		unsigned int scoutKicks;

		/** I-009: generations of stagnation that trigger the deep call, and
		    whether it has already fired in this run. */
		unsigned int deepLsTrigger;
		unsigned int deepLsShare;      // percent of local-search time, 0 = one shot
		bool deepLsDone;               // fired in the current stagnation episode
		unsigned int deepLsCalls;
		clock_t deepLsTime;

		//=========================================================================
		//		FIELDS
		//=========================================================================
	protected:
		enum LS_Frequency { MALS_NONE, MALS_INITIAL, MALS_FINAL, MALS_PERIOD, MALS_STUCK };
		enum LS_Target { MALS_BEST, MALS_WORST, MALS_ALL, MALS_SOME };

		//-----  CONFIGURATION FIELDS  ------------------------
		/**
		* Local Search algorithm
		*/
		LocalSearch * localSearch;

		/*
		* Simulated cooling algorithm
		*/
		Non_Monotonic_Adaptative_Cooling* simulatedCooling;
		
		/**
		* Neighbourhood structure for the Local Search
		*/
		Neighbourhood * neighbourhood;

		/**
		* Frequency for applying the local search algorithm
		*/
		LS_Frequency lsFrequency;

		/**
		* Period between applying the Local search
		* (if lsFrequency is PERIOD or STUCK)
		*/
		unsigned int lsPeriod;


		/**
		* Individuals targetted by the local search
		*/
		LS_Target lsTarget;

		/**
		* Percentage of individuals to which LS should be applied
		*/
		double lsPercentage;

		/**
		* Flag indicating if the chromosome must be updated after
		* applying local search
		*/
		char lsLamarckism;
	
		//-----  DYNAMIC FIELDS  ------------------------------
		/**
		* Current status
		*/
		unsigned int evaluationsLS;
		unsigned int neighboursLS;
		unsigned int iterationsLS;
		unsigned int callsLS;
		unsigned int abc_replacements;
		unsigned int improvementsLS;
		unsigned int enworstmentsLS;
		unsigned int neutralLS;
		unsigned int maxNumTrials;

		/**
		* Runtimes
		*/
		clock_t localSearchTime;





		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		* Constructor using the parameters file.
		* Reads the parameters file, but does not initialize objects
		*/
		explicit ArtificialBeeColonyPSO(ParameterDB *params = NULL);

		/**
		* Destructor
		*/
		virtual ~ArtificialBeeColonyPSO();


	protected:
		/**
		* Destroys everything
		*/
		virtual void clearAll();


		//=========================================================================
		//		GET/SET METHODS
		//=========================================================================
	public:
		/**
		* This method prints the configuration of the algorithm in CSV format
		* and using a tree structure for a better understanding
		*
		* @return A string of parameter values. The first string is the name of
		* the operator
		*/
		virtual void printSetupTree(std::ofstream & output) const;


		/**
		* Returns the basic statistics provided by the algorithm
		*
		* @return An array of pairs with the name of the statistic and its value
		*/
		virtual std::vector< std::pair<std::string, double> > getStatistics() const;

		/**
		* Returns the time employed by each component of the algorithm
		*
		* @return An array of pairs with the name of the component and the time
		* consumed. The first component is always the total runtime
		*/
		virtual std::vector< std::pair<std::string, double> > getRuntime() const;





		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		/**
		* Creates all the objects that are needed by the algorithm from the
		* configuration stored in the ParameterDB object. This must be called
		* the first time we want to run the algorithm. If a parameter is changed
		* between the runs, it must be called again
		*/
		virtual void prepareToRun(ParameterDB *params = NULL);


		/**
		* Run the algorithm
		*
		* @return A set of solutions
		*/
		virtual std::pair<Solution *, Objective *> run(Problem *problem,
			std::string signature, std::string logFolder, int rngSeed);

	protected:
		/**
		* Check that the algorithm is setup and ready to run.
		* If any object is not correctly initialized, this produces an
		* exception so the algorithm stops before going any further
		*
		* @return true if all components are correctly initialized
		*/
		virtual bool checkSetup();


		/**
		* Method for the evaluation of the population, deciding if
		* Local Search should be applied or not
		*/
		virtual void evaluatePopulation(Population *current);


		/**
		* Apply the Local Search to all the chosen elements
		*/
		virtual void applyLocalSearch(Population *population);

		/**
		* Apply the Local Search to a concrete element
		*/
		virtual void applyLocalSearch(Population *population,
			const unsigned int individualIdx);
	};

}
