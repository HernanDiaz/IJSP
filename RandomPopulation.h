/*
* RandomPopulation.h
*
*  Created on: Jun 12, 2018
*/
#pragma once

#include "GeneticAlgorithm.h"


namespace FuzzyFW {

// Parameters defined in this header file
#define RS_DISTANCE_METRIC "distance.metric"	// Used metric for distances



	//=============================================================================
	//
	//	Class RandomPopulation
	//
	//=============================================================================
	/**
	* This class implements a Random Search algorithm.
	* it creates a pool of random solutions and returns the best
	*
	*
	*/
	class RandomPopulation : public EvolutiveAlgorithm {
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
		* Different stopping criteria
		*/
		int maxPlateau;
		long int maxEvaluations;
		double maxRuntime;

		/**
		* Show the evolution by generations or by time
		*/
		bool showEvolutionTime;

		/**
		* Span of time/generations to show evolution
		*/
		double evolutionSpan;

		/**
		* Statistics object to measure diversity distances
		*/
		Statistics * diversityStats;



		//-----  DYNAMIC FIELDS  ------------------------------
		/**
		* Current status
		*/
		unsigned int iterationsNI;
		unsigned int evaluations;
		Individual *bestSoFar;

		/**
		* Runtimes
		*/
		clock_t totalRuntime;
		clock_t creationTime;
		clock_t evaluationTime;

		/**
		* Statistics on distances
		*/
		double shortestDistance;
		double longestDistance;
		double avgDistance;
		double stdevDistance;


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
		explicit RandomPopulation(ParameterDB *params = NULL);

		/**
		* Destructor
		*/
		virtual ~RandomPopulation();


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
		virtual void computeStatistics(Individual *newIndividual);
	};

}
