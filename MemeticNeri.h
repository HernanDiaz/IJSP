/*
* MemeticNeri.h
*
*  Created on: Jun 13, 2018
*/
#pragma once


#include "MemeticAlgorithm.h"



namespace FuzzyFW {

/* Variant of a Memetic Algorithm proposed in the following paper:
*
* An Adaptive Multimeme Algorithm for Designing HIV Multidrug Therapies.
* F.Neri, J.Toivanen, G.L.Cascella, Y-S. Ong. IEEE/ACM TRANSACTIONS ON
* COMPUTATIONAL BIOLOGY AND BIOINFORMATICS. Vol 4(2), 264-278. 2007
*
*	This algorithm uses a diversity measure to modify the exploitation 
* capabilities of the algorithm, increasing or decreasing the crossover
* probability, mutation probability and population size.
* Therefore, it requries additional parameters:
*/
#define NERI_POP_MINSIZE "population.minsize"
#define NERI_POP_MAXSIZE "population.minsize"

#define NERI_CROSS_MIN "crossover.min"
#define NERI_CROSS_MAX "crossover.max"

#define NERI_MUTATION_MIN "mutation.minprob"
#define NERI_MUTATION_MAX "mutation.maxprob"

#define NERI_LS_MIN "localSearch.minTarget"
#define NERI_LS_MAX "localSearch.maxTarget"

#define NERI_DIVERSITY_METRIC "diversity.metric"



	//=============================================================================
	//
	//	Class MemeticNeri
	//
	//=============================================================================
	/**
	* This class implements a generic memetic algorithm. It is completely based
	* on the structure of a genetic algorithm. However, it includes a local search
	* strategy that can be applied on different points of the run and with
	* different probability values
	*
	*
	*/
	class MemeticNeri : public MemeticAlgorithm {
		//=========================================================================
		//		FIELDS
		//=========================================================================
	protected:
		//-----  CONFIGURATION FIELDS  ------------------------
		/**
		* Minimum/Maximum population size
		*/
		unsigned int minPopSize;
		unsigned int maxPopSize;

		/**
		* Minimum/Maximum crossovers pre iteration
		*/
		unsigned int minCrossover;
		unsigned int maxCrossover;

		/**
		* Minimum/Maximum mutation probabilities
		*/
		double minMutation;
		double maxMutation;

		/**
		* Minimum/Maximum probabilities of applying Local Search
		*/
		double minLS;
		double maxLS;

		/**
		* Diversity metric
		*/
		Statistics *diversityMetric;

			

		//-----  DYNAMIC FIELDS  ------------------------------
		/**
		* Runtimes
		*/
		clock_t diversityTime;





		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		* Constructor using the parameters file.
		* Reads the parameters file, but does not initialize objects
		*/
		explicit MemeticNeri(ParameterDB *params = NULL);

		/**
		* Destructor
		*/
		virtual ~MemeticNeri() { }


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
