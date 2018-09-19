/*
 * Individual.h
 *
 *  Created on: June 22, 2017
 *      Author: jjpalacios
 */
#ifndef ECOBJECTS_INDIVIDUAL_H_
#define ECOBJECTS_INDIVIDUAL_H_

#include "SharedVars.h"
#include "Fitness.h"
#include "Solution.h"


namespace FJSP {


//=============================================================================
//
//	Abstract class Individual
//
//=============================================================================
/**
 * This class provides the framework to implement individuals to be used by
 * evolutionary algorithms.
 *
 * By default, all individuals are codified as an aray of genes, which can
 * be of any type. There is then a phenotype, which in this framework is a
 * solutions to scheduling problem, or a Schedule.
 *
 * Classes inheriting from this one may redefine the way of updating all
 * the components of the Individual
 *
 * The classes will be based in the std::vector collection for a quick
 * access to the genes.
 *
 * @author jjpalacios
 *
 */
class Individual {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
public:
	// Different types of Individuals implemented
	enum IndividualType { ARRAY_INTEGER };

	/*
	* Identifier of the individual
	*/
	int id;


protected:
	/*
	* Phenotype of the Individual (problem solution)
	*/
	Solution * phenotype;

	/**
	 * Fitness
	 */
	Fitness *fitness;

	/**
	 * Evaluated flag
	 */
	char evaluated;

	/**
	* Updated phenotype flag
	*/
	char phenotypeUpdated;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	Individual(const int idx = -1);
	
	/**
	 * Copy constructor
	 */
	Individual(const Individual &source);

	/**
	 * Destructor
	 */
	virtual ~Individual();

	/**
	 * Clone operator for the case of inheritance
	 */
	virtual Individual* clone() const = 0;



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Set the fitness of the individual
	*/
	virtual void updateFitness(Fitness * fitness);

	/**
	* Get the fitness of the individual
	*/
	Fitness * getFitness() const;

	/**
	* Gets the length of the genotype
	*/
	virtual unsigned int size() const = 0;

	/**
	* Set a new phenotype for the individual
	*/
	virtual void updatePhenotype(Solution *solution);

	/**
	* Get the genotype of the individual as an array of real values
	*/
	Solution * getPhenotype() const {
		return this->phenotype;
	}

	/**
	* Check if the phenotype of the individual is updated to the genotype
	*/
	char isPhenotypeUpdated() const {
		return this->phenotypeUpdated;
	}

	/**
	* Check if the individual has been already evaluated
	*/
	char isEvaluated() const {
		return this->evaluated;
	}
	

	/**
	 * Print the genotype into a string. Each gene is separated by a blank
	 * space
	 *
	 * @return The genotype of the individual
	 */
	virtual std::string toString() const = 0;


	/**
	* Specify the type of Individual
	*
	* @return The type of individual
	*/
	virtual IndividualType getType() const = 0;
};





//=============================================================================
//
//	Class IndividualArrayInteger
//
//=============================================================================
/**
* This class codifies individuals using an array of integer values
*
* @author jjpalacios
*
*/
class IndividualArrayInt : public Individual {
	//=========================================================================
	//		FIELDS
	//=========================================================================
	/*
	* Genotype of the Individual
	*/
	std::vector<int> genotype;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	IndividualArrayInt()
		: Individual(), genotype() { }

	/**
	* Genotype constructor
	*/
	IndividualArrayInt(const std::vector<int> &genes);

	/**
	* Copy constructor
	*/
	IndividualArrayInt(const IndividualArrayInt &source);

	/**
	* Destructor
	*/
	virtual ~IndividualArrayInt() { } // Nothing to destroy (sadly)


	/**
	* Clone operator for the case of inheritance
	*/
	virtual Individual * clone() const {
		return new IndividualArrayInt(*this);
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Set a new genotype for the individual. The individual will be
	* considered as not evaluated from that moment
	*/
	virtual void setGenotype(std::vector<int> & genes);

	/**
	* Updates de genotype of the individual, keeping the evaluation
	* flags untouched.
	*/
	virtual void updateGenotype(std::vector<int> & genes);

	/**
	* Get the genotype of the individual as an array of real values
	*/
	virtual std::vector<int> & getGenotype() {
		return (std::vector<int> &)this->genotype;
	}

	/**
	* Set a specifica value for a gene
	*/
	virtual void setGene(const unsigned int index, int value);

	/**
	* Gets a specific gene
	*/
	virtual int getGene(const unsigned int index) const;

	/**
	* Gets the length of the genotype
	*/
	virtual unsigned int size() const {
		return (unsigned int)this->genotype.size();
	}
	

	/**
	* Print the genotype into a string. Each gene is separated by a blank
	* space
	*
	* @return The genotype of the individual
	*/
	virtual std::string toString() const;


	/**
	* Specify the type of Individual
	*
	* @return The type of individual
	*/
	virtual IndividualType getType() const {
		return Individual::ARRAY_INTEGER;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
	double euclideanDistance(const Individual *ind) const;

	
};

// Future versions
/*
double hammingDistance(const SharedVars const *sharedVars,
const IndividualArrayInt *ind1, const IndividualArrayInt *ind2);
*/


}

#endif /* ECOBJECTS_INDIVIDUAL_H_  */

