/*
 * Encoder.h
 *
 *  Created on: September 13, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_ECOPERATORS_ENCODER_H_
#define SRC_ECOPERATORS_ENCODER_H_

#include "FuzzySchedule.h"
#include "Individual.h"


namespace FJSP {


//=============================================================================
//
//	Abstract class Encoder
//
//=============================================================================
/**
 * This class is responsible for encoding a solution into the chromosome
 * of an individual. It's the complementary of the decoder
 *
 * @author jjpalacios
 *
 */
class Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Encoder(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Encoder(const Encoder &source) { }

	/**
	 * Loads the needed parameters. None in this case
	 */
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const = 0;

	/**
	 * Destructor
	 */
	virtual ~Encoder() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Encodes the solution into the individual's genotpye
	 */
	virtual void encode(Solution *solution, Individual *indiv,
		const SharedVars * const svars) const = 0;

	/**
	* Encodes the solution into a new individual
	*/
	virtual Individual * encode(Solution *solution,
		const SharedVars * const svars) const = 0;

	/**
	 * Get the name and setup of the encoding method
	 */
	virtual std::vector<std::string> getName() const=0;
};





//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it. 
* Tasks are numbered from 0 to n-1, begin n the number of tasks or operations.
*
* @author jjpalacios
*
*/
class EncoderFJSP_Order : public Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderFJSP_Order(ParameterDB *parameters = NULL)
		: Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderFJSP_Order(const EncoderFJSP_Order &source)
		: Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const {
		return new EncoderFJSP_Order(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderFJSP_Order() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Encodes the solution into the individual's genotpye
	*/
	virtual void encode(Solution *solution, Individual *indiv,
		const SharedVars * const svars) const;


	/**
	* Encodes the solution into a new individual
	*/
	virtual Individual * encode(Solution *solution,
		const SharedVars * const svars) const;


	/**
	* Get the name and setup of the encoding method
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Task Permutation");
		return name;
	}
};





//=============================================================================
//
//	Class EncoderFJSP_JobOrder
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it.
* Each task is codified with the number of job it belongs to
*
* @author jjpalacios
*
*/
class EncoderFJSP_JobOrder : public Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderFJSP_JobOrder(ParameterDB *parameters = NULL)
		: Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderFJSP_JobOrder(const EncoderFJSP_Order &source)
		: Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const {
		return new EncoderFJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderFJSP_JobOrder() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Encodes the solution into the individual's genotpye
	*/
	virtual void encode(Solution *solution, Individual *indiv,
		const SharedVars * const svars) const;

	/**
	* Encodes the solution into a new individual
	*/
	virtual Individual * encode(Solution *solution,
		const SharedVars * const svars) const;

	/**
	* Get the name and setup of the encoding method
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Job Permutation");
		return name;
	}
};

}

#endif /* SRC_ECOPERATORS_ENCODER_H_ */
