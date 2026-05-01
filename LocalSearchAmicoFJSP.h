/*
* LocalSearchAmicoFJSP.h
*
*  Created on: Feb 15, 2019
*/
#pragma once

#include "LocalSearchAmico.h"
#include "ProblemFJSP.h"


namespace FJSP {

//=============================================================================
//
//	Class LS_Tabu_Amico_FJSP
//
//=============================================================================
/**
* This class defines the method to apply Tabu Search based on the dynamic
* tabu lists proposed in the paper:
*
* Applying tabu search to the job-shop scheduling problem. M. Dell'Amico and
* M. Trubian. Annals of Operations Research 41(1993) 231-252.
*
* It includes the particularity by which the parameters a, b, A and B are
* pre-established at the following values:
*	- a = 2
*	- b = 2 + (n+m)/3
*	- A = min+6
*	- B = A + (n+m)/3
*
*
*/

class LS_Tabu_Amico_FJSP : public FuzzyFW::LS_Tabu_Amico {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/**
	* Constant factor used for the calculation of min/max tabu list sizes
	*/
	double addend;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LS_Tabu_Amico_FJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::LS_Tabu_Amico(parameters), addend(-1.0) { }

	/**
	* Copy constructor
	*/
	LS_Tabu_Amico_FJSP(const LS_Tabu_Amico_FJSP &source)
		: LS_Tabu_Amico(source), addend(source.addend) { }

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Clone method for inheriting classes
	*/
	virtual LS_Tabu_Amico_FJSP * clone() const {
		return new LS_Tabu_Amico_FJSP(*this);
	}

	/**
	* Destructor. Nothing to destroy
	*/
	virtual ~LS_Tabu_Amico_FJSP() {	}



	//=========================================================================
	//		METHODS
	//=========================================================================
protected:
	/**
	* Updates the minimum and maximum tabu list sizes
	*/
	virtual void updateTabuListBounds(const FuzzyFW::SharedVars *svars);



	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup, tabu;
		std::string line;

		tabu = this->tabuList->getName();
		setup.push_back("Tabu Search (Dell'Amico FJSP)");
		for (size_t i = 0; i < tabu.size(); i++) {
			setup.push_back(";" + tabu[i]);
		}
		setup.push_back(";Tcycle:;"
			+ valueToString(this->Tcycle));
		setup.push_back(";Lambda:;"
			+ valueToString(this->Lambda));
		setup.push_back(";Min tabu list size:;"
			+ valueToString(this->minA));
		setup.push_back(";Addend for tabu list size:;"
			+ valueToString(this->addend));
		setup.push_back(";Max. Iterations without Improve:;"
			+ valueToString(this->maxBadIterations));
		if (this->maxIterations >= 0)
			setup.push_back(";Max. Iterations:;"
				+ valueToString(this->maxIterations));
		if (this->maxEvaluations >= 0)
			setup.push_back(";Max. Evaluations:;"
				+ valueToString(this->maxEvaluations));
		if (this->maxTime >= 0)
			setup.push_back(";Max. Time:;"
				+ valueToString(this->maxTime));
		if (this->estimationGuided)
			setup.push_back(";Guide:;Estimations");
		else
			setup.push_back(";Guide:;Real fitness");
		if (this->estimationFilter)
			setup.push_back(";Filter:;Yes");
		else
			setup.push_back(";Filter:;No");
		return setup;
	}
};

}

