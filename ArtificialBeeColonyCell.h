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
	class ArtificialBeeColonyCell : public GeneticAlgorithm {
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
		explicit ArtificialBeeColonyCell(ParameterDB *params = NULL);

		/**
		* Destructor
		*/
		virtual ~ArtificialBeeColonyCell();


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
