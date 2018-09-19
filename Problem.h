/*
* Problem.h
*
*  Created on: Sep 22, 2017
*      Author: jjpalacios
*/
#ifndef ECOBJECTS_PROBLEM_H_
#define ECOBJECTS_PROBLEM_H_

#include "ParameterDB.h"

namespace FJSP {


//=============================================================================
//
//	Abstract class Problem
//
//=============================================================================
/**
* This class is a simple super class under which problems can be defined
* by inheritance.
*
* The only target of this class is to provide a wrapper, so methods of
* different algorithms can receive the problem under the name
* of this class.
*
* @author jjpalacios
*
*/
class Problem {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/**
	* Name of the problem
	*/
	std::string problemName;

	/**
	* Path to the problem instance
	*/
	char * problemPath;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	Problem(const ParameterDB *params = NULL) { }



	/**
	* Main constructor
	* Requires the file of parameters to check if there are bounds and/or
	* due-dates and therefore make a better reading of the input file.
	*/
	Problem(const ParameterDB *params, const char *inputFile) {

		char auxChar;

		// Initialize problem name and path
		size_t len = strlen(inputFile);
		this->problemPath = (char *)malloc(len + 1);
		memset(this->problemPath, '\0', len + 1);
		memcpy(this->problemPath, inputFile, len);

		this->problemName = "";
		for (int i = 0; this->problemPath[i] != '\0'; i++) {
			auxChar = this->problemPath[i];
			if (auxChar == '/' || auxChar == '\\')
				this->problemName = "";
			else
				this->problemName += auxChar;
		}
		this->problemName = this->problemName.substr(0,
			this->problemName.length() - 4);
	}



	/**
	* Copy constructor
	*/
	Problem(const Problem &source) {
		this->problemPath = (char *)malloc(strlen(source.problemPath) + 1);
		memset(this->problemPath, '\0', sizeof(this->problemPath));
		memcpy(this->problemPath, source.problemPath, strlen(source.problemPath));

		this->problemName = source.problemName;
	}



	/**
	* Destructor
	*/
	virtual ~Problem() {
		free(this->problemPath);
	}



	/**
	* Clone operator for the case of inheritance
	*/
	virtual Problem* clone() const = 0;





	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/**
	* Get the name of the problem
	*/
	std::string getName() const {
		return this->problemName;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Load all data from the stored file
	*/
	virtual void loadFile(const char *inputFile) = 0;


protected:
	/**
	* Clears all the structures in the class. Used mainly for reset
	* purposes or re-read a file.
	*/
	virtual void clear() = 0;

};

}


#endif /* ECOBJECTS_SOLUTION_H_ */
