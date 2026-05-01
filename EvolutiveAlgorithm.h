/*
* EvolutiveAlgorithm.h
*
*  Created on : Sep 15, 2017
*/
#pragma once


#include "EvaluationClassRegister.h"
#include "StatisticsClassRegister.h"


namespace FuzzyFW {

#define OBJECTIVE_FUNCTION "objective"
#define STATISTICS_ROOT "statistics"
#define STATISTICS_VALUE "value"
#define STATISTICS_METRIC "metric"

#define STATISTICS_BEST "best"
#define STATISTICS_WORST "worst"
#define STATISTICS_AVG "average"
#define STATISTICS_SDEV "deviation"


//=============================================================================
//
//	Abstract class EvolutiveAlgorithm
//
//=============================================================================
/**
* This class provides the framework to implement any kind of evolutionary
* algorithm with the following features:
*		- Single objective
*		- Return 1 solution
*
*
*/
class EvolutiveAlgorithm {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/**
	* Indicates if the algorithm is ready to run
	*/
	bool ready;

public:
	/**
	* Shared variables for the algorithm
	*/
	SharedVarsEvolutionary * sharedVariables;

	/**
	* Flag indicating the status of the algorithm
	*/
	bool finished;

	/**
	* Evaluation function
	*/
	Evaluation *evaluator;

	/**
	* Statistics to show
	*/
	std::vector<Statistics *> statistics;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file.
	* Reads the parameters file, but does not initialize objects
	*/
	explicit EvolutiveAlgorithm(ParameterDB *params = NULL);

	/**
	* Destructor
	*/
	virtual ~EvolutiveAlgorithm() {
		delete sharedVariables;
	}

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
	virtual void printSetupTree(std::ofstream & output) const = 0;


	/**
	* Returns the basic statistics provided by the algorithm
	*
	* @return An array of pairs with the name of the statistic and its value
	*/
	virtual std::vector< std::pair<std::string, double> > getStatistics() const = 0;

	/**
	* Returns the time employed by each component of the algorithm
	*
	* @return An array of pairs with the name of the component and the time
	* consumed. The first component is always the total runtime, the others
	* are relative to the total amount of time
	*/
	virtual std::vector< std::pair<std::string, double> > getRuntime() const = 0;

	/**
	* Returns the evolution of the algorithm during the run
	*
	* @param labels Output variable to store the name of each evolution
	* component
	* @return A matrix with the evolution of different elements
	*/
	virtual std::vector< std::vector<double> > getEvolution(
		std::vector< std::string > &labels) const = 0;



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
	* @return The best found solution
	*/
	virtual std::pair<Solution *, Objective *> run(Problem *problem,
		std::string signature, std::string logFolder, int rngSeed) = 0;

};


}
