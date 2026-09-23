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
// I-014. Restart the population when the run has stalled long enough that a
// further improvement has stopped being likely.
//   "no"    (default, unchanged)
//   "stall" : when STALL_RESTART_SHARE of the budget has passed with no new
//             global best, rebuild the population AROUND THE INCUMBENT -- the
//             global best itself plus perturbed copies of it, 1 to 10 mutations
//             each -- and go on to the same budget.
// The cold form of this, rebuilding with the creation operator, was WITHDRAWN
// on 2026-09-23 before any machine time was spent, because the traces already
// on disk say it cannot pay (scripts/cold_catchup.py, 120 control runs). A cold
// population needs about 80 % of the budget to come within a few units of what
// the run finally reaches: at 40 % of the budget it is still +4.5 to +22 away,
// at 20 % it is +27 to +55. A stall restart fires late by construction, since
// it needs 0.2 of the budget of stall AFTER the last improvement and that
// improvement lands at a median 0.56 to 0.82 of the budget, so it is handed the
// stretch where a cold start is furthest behind. The check bore it out: the
// mechanism fired 1 and 3 times and the makespans came out IDENTICAL to the
// control's. Rebuilding around the incumbent costs nothing to catch up, so what
// is being tested is diversity rather than a handicap.
// The spread of 1 to 10 mutations is fixed and not tuned: it sits below the
// reach of one tabu call, measured at 26 to 41 moves, so the perturbation is
// not simply undone by the local search that follows it.
// The share is FIXED BY A STATED RULE over the measured stall hazard
// (scripts/stall_hazard.py, 120 control runs, 2026-09-23) and is not a
// tunable: the FIRST bin edge at which the hazard has fallen below a tenth of
// its value in the first second. That gives 8 s of 40 on ta23 and ta30, 4 s of
// 40 on ta29 and 16 s of 150 on ta45, i.e. 0.10 to 0.20 of the budget, and the
// most conservative of them is taken.
// The first attempt used 0.4, the age at which the hazard reaches ZERO, and
// that was a calibration mistake caught before any machine time was spent: on
// a 150 s budget a 60 s stall can only be reached if the last improvement came
// before 90 s, while its median is 123 s. The mechanism fired once in four
// runs and the makespans came out identical to the control's. A threshold
// where nothing ever happens measures nothing, which is what I-009 cost four
// redesigns to learn.
// Restarting INSIDE the run keeps the per-class budget, the number of runs and
// the endpoint exactly as they are, which ending the run early would not.
// I-015. What happens to an offspring that improves its individual up to
// EXACTLY the incumbent's makespan, in the crossover replacement and in the
// Lamarckian write-back of the tabu search.
//   "veto"  (default, unchanged): it is discarded and a trial failure counted
//   "allow" : it is kept like any other improvement
// The veto is a duplicate filter built on the fitness rather than on the
// genotype: it rejects DIFFERENT solutions for tying and admits clones whose
// makespan differs. In JSP the plateaus at the incumbent's value are large and
// moving sideways along them is how a search leaves a basin, which is what
// I-014 found this one cannot do. Measured 2026-09-21: 1,324 improvements per
// run discarded in the write-back on ta29 and 3,415 on ta41, 2.4 % of the
// useful tabu work, with a spread of 71 to 8,073 between runs.
#define  PLATEAU_MODE "abc.plateau"
// I-017. Which individual the second local-search call of MALS_SOME reaches.
//   "index"  (default, unchanged): individual i, the loop counter
//   "chosen" : the individual just drawn at random, as the code intends
//   "best"   : I-018, always the best of the group again, the one the first
//              call already searched: a second tabu search from its local
//              optimum with an empty tabu list, which is what the index bug
//              does by accident a little over half the time. I-017 measured
//              that removing it entirely costs +4.87.
//   "none"   : I-020, no second call at all. Cheaper still than "best", so
//              even more generations, but without the extra depth on the
//              better child: it separates the two things "best" does at once.
// The loop draws a random individual without replacement, skips it if it is
// the best, and then applies the tabu search to individual i instead of to the
// one it drew, so the draw is computed and thrown away. The only call site the
// frozen configuration reaches is the pair of children of each food source,
// where the quota trunc(0.4645 x 2) - 1 = -1 wraps round to about four billion
// in an unsigned int, so the loop walks both children and the second call
// always lands on child 0. When the better child is child 0 it is searched
// TWICE, the second time from a local optimum, and the other child never is.
#define  LS_PICK "abc.ls.pick"

#define  STALL_RESTART "abc.restart"
#define  STALL_RESTART_SHARE 0.2

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

		// I-014: restart on a stalled run. stallRestart is the switch;
		// lastImprovementSec is when the global best last moved, in the same
		// seconds the stopping criterion counts; stallRestarts is the
		// mechanism check, because an idea that never fires was never tested.
		bool stallRestart;
		double lastImprovementSec;
		unsigned long stallRestarts;

		// I-015: the switch, and how many plateau moves it admitted, which is
		// the mechanism check.
		bool plateauAllow;
		unsigned long plateauAdmittedCross;
		unsigned long plateauAdmittedLS;

		// I-017: the switch, and what the second call reaches.
		bool lsPickChosen;
		bool lsPickBest;       // I-018
		bool lsPickNone;       // I-020
		unsigned long lsSecondSkipped;   // I-020: second calls not made
		unsigned long lsPairCalls;        // group-of-two invocations
		unsigned long lsOtherCalls;       // invocations on any other group size
		unsigned long lsSecondOnBest;     // second call on the already searched best
		unsigned long lsSecondOnOther;    // second call on the other child


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
