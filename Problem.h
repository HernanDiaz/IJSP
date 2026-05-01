/*
* Problem.h
*
*  Created on: Sep 22, 2017
*/
#pragma once

#include "ParameterDB.h"
#include "TimeWindowClassRegister.h"

namespace FuzzyFW {


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

	/**
	* Indicates if the problem has been loaded or not
	*/
	char isSetup;




	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	Problem(const ParameterDB *params = NULL) {
		TimeWindowClassRegister::registerClasses();
		if (params != NULL)
			this->setup(params);
	}



	/**
	* Main constructor
	* Requires the file of parameters to check if there are bounds and/or
	* due-dates and therefore make a better reading of the input file.
	*/
	Problem(const ParameterDB *params, const char *inputFile) {
		TimeWindowClassRegister::registerClasses();
		this->updatePath(inputFile);
		if (params != NULL)
			this->setup(params);
	}


	/**
	* Main constructor
	* Takes the path to the problem instance, but does not load.
	*/
	Problem(const char *inputFile) {
		TimeWindowClassRegister::registerClasses();
		this->updatePath(inputFile);
	}


	/**
	* Loads the parameters describing the problem
	*/
	virtual void setup(const ParameterDB *params) {
		this->isSetup = true;
	}


	/**
	* Copy constructor
	*/
	Problem(const Problem &source) {
		this->problemPath = (char *)malloc(strlen(source.problemPath) + 1);
		memset(this->problemPath, '\0', sizeof(this->problemPath));
		memcpy(this->problemPath, source.problemPath,
			strlen(source.problemPath));

		this->problemName = source.problemName;
		this->isSetup = source.isSetup;
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

protected:
	/**
	* Auxiliar method. Sets the path of the problem from the complete name
	*/
	void updatePath(const char*newPath) {
		char auxChar;

		// Initialize problem name and path
		size_t len = strlen(newPath);
		this->problemPath = (char *)malloc(len + 1);
		memset(this->problemPath, '\0', len + 1);
		memcpy(this->problemPath, newPath, len);

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





	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Load all data from the stored file
	*/
	virtual void loadFile(const char *inputFile = NULL) = 0;


protected:
	/**
	* Clears all the structures in the class. Used mainly for reset
	* purposes or re-read a file.
	*/
	virtual void clear() = 0;

};

}
