/*
 * Crossover.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */

#include "EncoderIJSP.h"
#include "Crossover.h"


namespace IJSP {

//=============================================================================
//
//	Class Crossover_JOX
//
//=============================================================================
/**
 * Job Order Crossover operator. This poerator is especifically designed to
 * work on job shop scheduling problems.
 *
 * The operator chooses a random set of jobs from parent 1 and keeps all
 * operations belonging to those jobs in their original position. The rest
 * of the chromosome is then filled by scheduling the remaining tasks following
 * the ordering given by the second parent.
 *
 * @author hdiaz
 *
 */
class Crossover_JOX : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Constructor using the parameters file
	 */
	Crossover_JOX(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	 * Loads extra parameters.
	 */
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	 * Destructor
	 */
	virtual ~Crossover_JOX() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the crossover operator to a pair of individuals and produces two
	 * offspring. The offspring will automatically replace their parents
	 *
	 * @param ind1 First parent for the mating
	 * @param ind2 Second parent for the mating
	 * @param svars Shared elements of the algorithm
	 * @return The two offspring produced
	 */
	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("JOX");
		return setup;
	}

protected:
	/**
	 * Crossover operator depending on the type of individual we are
	 * receiving
	 *
	 * @param ind1 First parent for the mating
	 * @param ind2 Second parent for the mating
	 * @param svars Shared elements of the algorithm
	 * @return The two offspring produced
	 */
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

};





//=============================================================================
//
//	Class Crossover_GOXBierwirth
//
//=============================================================================
/**
* General Order Crossover operator proposed by Bierwtih in this paper:
*		Bierwirth, C. A Generalized Permutation Approach to Jobshop Scheduling
*		with Genetic Algorithms. OR Spectrum (17), 87-92. 1995
*
* The operator chooses a random sequence of genes from parent 1. Then, in
* order to generate the offspring, it copies the genes of the second parent
* that are not included in the previously selected sequence until the first
* gene of the sequecne is found. At that point, the sequence is included and
* then the genotype refill with the remaining genes of the second parent.
* Example:
*	Parent 1: A B B A C A B C B C
*	Parent 2: B A B B C A C C B A
*	Substring in parent 1: * * * A C A B * * *
*	Locate the key gene: B A B B* C* [A] C C B A*
*	Offspring: B A B [A C A B] C C B
*		
* Bierwith establishes that the length of the substring must be always in the
* interval [1/3, 1/2] times the length of the genotype
*
* @author jjpalacios
*
*/
class Crossover_GOXBierwirth : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_GOXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads extra parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~Crossover_GOXBierwirth() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the crossover operator to a pair of individuals and produces two
	* offspring. The offspring will automatically replace their parents
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("GOX-Bierwirth");
		return setup;
	}

protected:
	/**
	* Crossover operator depending on the type of individual we are
	* receiving
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;
};






//=============================================================================
//
//	Class Crossover_GPMXBierwirth
//
//=============================================================================
/**
* General PMX operator proposed by Bierwtih in this paper:
*		Bierwirth C., Mattfeld D.C., Kopfer H. (1996) On permutation
*		representations for scheduling problems. In: Parallel Problem Solving
*		from Nature	— PPSN IV. PPSN 1996. LNCS, vol 1141.
*
* The operator chooses a random sequence of genes from parent 1. Then, in
* order to generate the offspring, it copies the genes of the second parent
* that are not included in the previously selected sequence until the position
* where the substring is. At that point, the sequence is included and
* then the genotype refill with the remaining genes of the second parent.
* Example:
*	Parent 1: A B B A C A B C B C
*	Parent 2: B A B B C A C C B A
*	Substring in parent 1: * * * A C A B * * *
*	Locate the key gene: B A B B* C* A* C C B A*
*	Offspring: B A B [A C A B] C C B
*
* Bierwith establishes that the length of the substring must be always in the
* interval [1/3, 1/2] times the length of the genotype
*
* @author jjpalacios
*
*/
class Crossover_GPMXBierwirth : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_GPMXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads extra parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~Crossover_GPMXBierwirth() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the crossover operator to a pair of individuals and produces two
	* offspring. The offspring will automatically replace their parents
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("GPMX-Bierwirth");
		return setup;
	}

protected:
	/**
	* Crossover operator depending on the type of individual we are
	* receiving
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;
};







//=============================================================================
//
//	Class Crossover_PPXBierwirth
//
//=============================================================================
/**
* PPX operator proposed by Bierwtih in this paper:
*		Bierwirth C., Mattfeld D.C., Kopfer H. (1996) On permutation
*		representations for scheduling problems. In: Parallel Problem Solving
*		from Nature	— PPSN IV. PPSN 1996. LNCS, vol 1141.
*
* The operator creates a sequence of 1's and 2's as long as the genotype.
* This string is used to determine when to take genes from the first or
* the second parent.
* Example:
*	Parent 1: C B B B C A A A C
*	Parent 2: A A C B B A B C C
*	Sequence mask: 1 1 2 2 2 2 1 1 1
*	Offspring: C B A A B A B C C
*
* @author jjpalacios
*
*/
class Crossover_PPXBierwirth : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_PPXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads extra parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~Crossover_PPXBierwirth() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the crossover operator to a pair of individuals and produces two
	* offspring. The offspring will automatically replace their parents
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("PPX-Bierwirth");
		return setup;
	}

protected:
	/**
	* Crossover operator depending on the type of individual we are
	* receiving
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;
};

}


