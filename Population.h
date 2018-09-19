/*
 * Population.h
 *
 *  Created on: Jun 23, 2017
 *      Author: jjpalacios
 */

#ifndef SRC_ECOBJECTS_POPULATION_H_
#define SRC_ECOBJECTS_POPULATION_H_

#include "Individual.h"

namespace FJSP {

/**
 * This class keeps an array of individuals to simulate a population for
 * an evolutionary algorithm
 *
 * It is based on the STL vector class for a quicker access to individuals
 *
 * The reason to keep an independent vector for the ordering of the individuals
 * instead of just sorting the original array is that some algorithm may
 * need to get the best K individuals of the population, but keeping
 *  a specific order in the individuals array
 *
 * @author jjpalacios
 *
 */
class Population {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/**
	 * Array of individuals
	*/
	std::vector<Individual *> individual;


	/**
	* Indicates whether the population is sorted or not
	*/
	char sorted;


	/**
	 * Order of individuals in the population based on their fitness
	*/
	std::vector<int> order;



	//=========================================================================
	//		CONTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	Population() 
		: sorted(true), order(), individual() { }

	/**
	 * Copy constructor
	 */
	Population(const Population &source);

	/**
	 * Destructor
	 */
	virtual ~Population();

	/**
	 * Clone operator (as in Java)
	 */
	virtual Population* clone() const {
		return new Population(*this);
	}

	/**
	 * Clears the objects of the population. This will delete forever all
	 * the individuals in the population
	 */
	void clear();



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	 * Controlled access to the individual that is at a given position
	 *
	 * @param position The position of the individual
	 * @return The individual at the given position
	 */
	Individual * getIndividual(const unsigned int index) const;


	/**
	* Replaces an individual by a new one.
	*
	* @param position The position of the individual to replace
	* @param individual The new individual to add to the population
	* @return The replaced individual
	*/
	Individual * replaceIndividual(const unsigned int index,
		Individual *newInd);


	/**
	 * Get the fitness of a given individual
	 *
	 * @param position The position of the individual
	 * @return the fitness of the individual
	 */
	Fitness * getFitness(const unsigned int index) const;


	/**
	 * Set the fitness of a given individual
	 *
	 * @param position The position of the individual
	 * @param fitness The fitness value to assign to the individual
	 */
	void setFitness(const unsigned int index, Fitness * fitness);


	/**
	 * Gets the Kth best individual in the population
	 * If the population is not sorted, this method will sort it
	 *
	 * @param k Ranking of the individual to get
	 * @return The Kth best individual
	 */
	Individual * getBest(const SharedVars * svars, const unsigned int k=0);


	/**
	 * Gets the position where the Kth best individual is placed
	 * If the population is not sorted, this method will sort it
	 *
	 * @param k Ranking of the individual whose position is wanted
	 * @return The position of the Kth best individual
	 */
	int whoIsBest(const SharedVars * svars, const unsigned int k=0);


	/**
	 * Gets the average quality of the population if it can convert the
	 * fitness type to double
	 *
	 * @return The average fitness in the population or -1 if there
	 * are not evaluated algorithms
	 */
	double getAverageFitness() const;

	/**
	 * Gets the size of the population
	 *
	 * @return The size of the population
	 */
	unsigned int size() const {
		return (int)this->individual.size();
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Add an individual to the population
	 *
	 * @param individual The individual to add to the population
	 */
	void addIndividual(Individual *individual);


	/**
	 * Sorts the population by the fitness of the individuals
	 *
	 * @param force Force the sorting of the population
	 */
	void sort(Random *rng, const bool force=false);


	/**
	* Updates the identifier of the individuals by setting the to the current
	* position in the population
	*/
	void rename();


	/**
	 * Print the current population in CSV format
	 *
	 * @param output Output file to print the population
	 */
	void printCSV(std::ofstream &output) const {
		for(int i=0;i < (int)this->individual.size(); i++) {
			output << this->individual[i]->toString();
			output << ";" << (this->individual[i])->getFitness()->toString() << std::endl;
		}
	}


protected:
	/**
	 * Quick-sort method applied to the population
	 */
	void quickSort(const int left, const int right, Random *rng);
};


}

#endif /* SRC_ECOBJECTS_POPULATION_H_ */
