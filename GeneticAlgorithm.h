/*
* GeneticAlgorithm.h
*
*  Created on: Sep 15, 2017
*      Author: jjpalacios
*/
#ifndef SRC_GENETICALGORITHM_H_
#define SRC_GENETICALGORITHM_H_


#include "EvolutiveAlgorithm.h"
#include "DataPrinter.h"
#include "GeneticClassRegister.h"



namespace FJSP {


// Parameters defined in this header file
#define GA_ENCODING "codification"	// Encoding funnction
#define GA_DECODING "decodification"	// Encoding funnction

#define GA_CREATION "creation"	// Creation strategy
#define GA_CROSSOVER "crossover"	// Crossover operator
#define GA_MUTATION "mutation"		// Mutation operator
#define GA_SELECTION "selection"	// Selection operator
#define GA_REPLACE "replacement"	// Replacement operator

#define GA_EVOL_METRIC "evolution.unit"	// Wthat metric use for evolution
#define GA_EVOL_UNIT_GEN "iteration"	
#define GA_EVOL_UNIT_TIME "time"	
#define GA_EVOL_SPAN "evolution.span"	// Span of units for showing evolution

#define GA_GENERATIONS "generations"	// Maximum number of iterations
#define GA_TIME "timelimit"			// Maximum runtime
#define GA_EVALUATIONS "evaluations"	// Maximum number of evaluations
#define GA_NOIMPROVE "noimprovement"	// Maximum number of evaluations

#define GA_POP_SIZE "population.size"			// Population size
#define GA_INDIVIDUAL_TYPE "gene.type"	// Type of individual (integer or real)

#define GA_PRINT_POPULATION "printpopulation"	// Shall we print the population?
#define GA_POP_INTERVAL "printpopulation.generations"	// Number of generations between printing populations



	//=============================================================================
	//
	//	Class GeneticAlgorithm
	//
	//=============================================================================
	/**
	* This class implements niche genetic algorithms.
	*
	* It allows both clearing and sharing strategies.
	*
	* @author jjpalacios
	*
	*/
	class GeneticAlgorithm : public EvolutiveAlgorithm {
		//=========================================================================
		//		FIELDS
		//=========================================================================
	protected:
		//-----  CONFIGURATION FIELDS  ------------------------
		/**
		* Initial popuations
		*/
		Creation * creation;

		/**
		* Selection operator
		*/
		Selection * selection;

		/**
		* Genetic operators
		*/
		Crossover * crossover;
		Mutation * mutation;

		/**
		* Replacement strategy
		*/
		Replacement * replacement;

		/**
		* Different stopping criteria
		*/
		int maxGenerations;
		int maxPlateau;
		long int maxEvaluations;
		double maxRuntime;

		/**
		* Population size
		*/
		unsigned int populationSize;

		/**
		* Printing population parameters
		*/
		bool printPopulation;
		int printPopGenerations;

		/**
		* Show the evolution by generations or by time
		*/
		bool showEvolutionTime;

		/**
		* Span of time/generations to show evolution
		*/
		double evolutionSpan;





		//-----  DYNAMIC FIELDS  ------------------------------
		/**
		* Current status
		*/
		unsigned int generation;
		unsigned int iterationsNI;
		unsigned int evaluations;
		Individual *bestSoFar;

		/**
		* Runtimes
		*/
		clock_t totalRuntime;
		clock_t creationTime;
		clock_t selectionTime;
		clock_t crossoverTime;
		clock_t mutationTime;
		clock_t replacementTime;
		clock_t evaluationTime;

		/**
		* Evolution data for statistics
		*/
		std::vector< std::vector<double> > evolutionStats;

		/**
		* Next moment to show evolution
		*/
		double nextSplit;





		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		* Constructor using the parameters file.
		* Reads the parameters file, but does not initialize objects
		*/
		explicit GeneticAlgorithm(ParameterDB *params = NULL);

		/**
		* Destructor
		*/
		virtual ~GeneticAlgorithm();


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

		/**
		* Returns the evolution of the algorithm during the run
		*
		* @param labels Output variable to store the name of each evolution
		* component
		* @return A matrix with the evolution of different elements
		*/
		virtual std::vector< std::vector<double> > getEvolution(
			std::vector< std::string > &labels) const;



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
		bool checkSetup();
		

		/**
		* Indicates if the stopping criteria has been met
		*
		* @return true if the stopping criteria is met
		*/
		virtual bool stop();


		/**
		* Stores the statistical values in each time/generation span
		*/
		virtual void computeStatistics(Population *currentPopulation);
	};

}

#endif /* SRC_GENETICALGORITHM_H_ */

