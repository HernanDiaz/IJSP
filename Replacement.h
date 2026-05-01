/*
 * Replacement.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Population.h"


namespace FuzzyFW {

// Replacement parameters defined in this header file
#define REPLACE_REPEAT "replacement.allow-repeated"
#define REPLACE_ELITE "replacement.elitism-k"



//=============================================================================
//
//	Abstract class Raplecement
//
//=============================================================================
/**
 * This class provides the framework to implement replacement strategies
 *
 * Classes inheriting from this one have to implement the method "apply"
 *
 * It will receive two populations and both will be combined into a simple one.
 * The newest population serves as a container for the resulting one,
 * while the oldest population will disappear
 *
 *
 */
class Replacement {
protected:
	//=========================================================================
	//		FIELDS
	//=========================================================================
	/**
	* Label for the parameter for allowing repeated elements
	*/
	std::string labelRepeat;

	/**
	* Field indicating if repeated elements are allowed or not
	*/
	char allowRepeated;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Replacement(ParameterDB *parameters = NULL);

	/**
	 * Loads the needed parameters: No parameters needed by default
	 */
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~Replacement() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Merge both populations into one. The resulting population will be kept
	 * in the newPopulation pointer, while the oldPopulation will be removed
	 *
	 * @param oldPopulation Original population. It will disappear
	 * @param newPopulation New generated population to replace the previous
	 * one
	 */
	virtual void apply(Population *oldPopulation, Population *newPopulation,
			const SharedVars *svars) const=0;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;
};





//=============================================================================
//
//	Class ReplacementElitist
//
//=============================================================================
/**
 * This class implements the classical elitist replacement.
 *
 * It will require an additional parameter, which is the number of elite
 * individuals
 *
 *
 */
class ReplacementElitist : public Replacement {
public:
	/**
	 * Name of the parameter for the size of the tournament
	 */
	std::string labelElite;

	/**
	 * Number of elite individuals
	 */
	unsigned int elite;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit ReplacementElitist(ParameterDB *parameters = NULL)
		: Replacement(parameters), labelElite(REPLACE_ELITE), elite(0) { }

	/**
	 * Loads the needed parameters: Number of elite individuals
	 */
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~ReplacementElitist() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Merge both populations into one. The resulting population will be kept
	 * in the newPopulation pointer, while the oldPopulation will be removed
	 *
	 * @param oldPopulation Original population. It will disappear
	 * @param newPopulation New generated population to replace the previous.
	 * one
	 */
	virtual void apply(Population *oldPopulation, Population *newPopulation,
			const SharedVars *svars) const;


	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Elitist");
		if (this->allowRepeated)
			setup.push_back(";Allow repeat;Yes");
		else
			setup.push_back(";Allow repeat;No");
		setup.push_back(";Elite;" + valueToString(this->elite));
		return setup;
	}
};





//=============================================================================
//
//	Class ReplacementParents
//
//=============================================================================
/**
* This class implements a tournament between parents and offspring. The 2
* best ones go into the next generation
*
*
*/
class ReplacementParents : public Replacement {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit ReplacementParents(ParameterDB *parameters = NULL)
		: Replacement(parameters) { }


	/**
	* Loads the needed parameters: Number of elite individuals
	*/
	virtual void setup(ParameterDB *parameters) {
		Replacement::setup(parameters);
	}


	/**
	* Destructor
	*/
	virtual ~ReplacementParents() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Merge both populations into one. The resulting population will be kept
	* in the newPopulation pointer, while the oldPopulation will be removed
	*
	* @param oldPopulation Original population. It will disappear
	* @param newPopulation New generated population to replace the previous.
	* one
	*/
	virtual void apply(Population *oldPopulation, Population *newPopulation,
		const SharedVars *svars) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Parents-Offspring");
		if (this->allowRepeated)
			setup.push_back(";Allow repeat;Yes");
		else
			setup.push_back(";Allow repeat;No");
		return setup;
	}

protected:
	/**
	* Apply the replacement allowing repeated elements
	*/
	virtual void applyRepeat(Population *oldPopulation,
		Population *newPopulation) const;

	/**
	* Apply the replacement avoiding repeated elements
	*/
	virtual void applyNoRepeat(Population *oldPopulation,
		Population *newPopulation) const;
};


}

