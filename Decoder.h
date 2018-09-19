/*
 * Decoder.h
 *
 *  Created on: September 12, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_ECOPERATORS_DECODER_H_
#define SRC_ECOPERATORS_DECODER_H_

#include "SchedulingClassRegister.h"
#include "Encoder.h"


namespace FJSP {

#define DECODING_SGS "decode.sgs"

//=============================================================================
//
//	Abstract class Decoder
//
//=============================================================================
/**
 * This class creates a method apply that receives a genotype and generates
 * a solution from it
 *
 * @author jjpalacios
 *
 */
class Decoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Decoder(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Decoder(const Decoder &source) { }

	/**
	 * Loads the needed parameters.
	 * Loads the crossover probability
	 */
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const = 0;

	/**
	 * Destructor
	 */
	virtual ~Decoder() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the decodification to the genotype to get a full solution to the
	 * problem
	 */
	virtual Solution * decode(Individual * indiv,
		const SharedVars * const svars) = 0;

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
//	Abstract class DecoderFJSP
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author jjpalacios
*
*/
class DecoderFJSP : public Decoder {
protected:
	//=============================================================================
	//		COMMON FIELDS
	//=============================================================================
	/*
	* Label to identify the SGS type
	*/
	const std::string sgsLabel;

	/*
	* SGS to create schedules from task orderings
	*/
	FuzzySGS * sgs;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	DecoderFJSP(ParameterDB *parameters = NULL)
		: Decoder(parameters), sgsLabel(DECODING_SGS) { }

	/**
	* Copy constructor
	*/
	DecoderFJSP(const DecoderFJSP & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~DecoderFJSP() {
		delete this->sgs;
	}
};





//=============================================================================
//
//	Class DecoderFJSP_Order
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype in the form
* of a task permutation and generates a solution using a specific SGS
*
* @author jjpalacios
*
*/
class DecoderFJSP_Order : public DecoderFJSP {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	DecoderFJSP_Order(ParameterDB *parameters = NULL)
		: DecoderFJSP(parameters) { } 


	/**
	* Copy constructor
	*/
	DecoderFJSP_Order(const DecoderFJSP_Order & source)
		: DecoderFJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderFJSP_Order(*this);
	}


	/**
	* Destructor
	*/
	virtual ~DecoderFJSP_Order() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Decodes an individual to create a full schedule
	*/
	virtual Solution * decode(Individual * indiv,
		const SharedVars * const svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		std::vector<std::string> sgsName = this->sgs->getName();
		name.push_back("Task Permutation");
		name.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			name.push_back(";" + sgsName[i]);
		return name;
	}
};





//=============================================================================
//
//	Class DecoderFJSP_JobOrder
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author jjpalacios
*
*/
class DecoderFJSP_JobOrder : public DecoderFJSP {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	DecoderFJSP_JobOrder(ParameterDB *parameters = NULL)
		: DecoderFJSP(parameters) { }

	/**
	* Copy constructor
	*/
	DecoderFJSP_JobOrder(const DecoderFJSP_JobOrder & source)
		: DecoderFJSP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderFJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~DecoderFJSP_JobOrder() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Decodes an individual to create a full schedule
	*/
	Solution * decode(Individual * indiv,
		const SharedVars * const svars);

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		std::vector<std::string> sgsName = this->sgs->getName();
		name.push_back("Job Permutation");
		name.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			name.push_back(";" + sgsName[i]);
		return name;
	}
};


}

#endif /* SRC_ECOPERATORS_CROSSOVER_H_ */
