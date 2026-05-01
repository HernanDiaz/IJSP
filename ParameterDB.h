/*
* ParameterDB.h
*
*  Created on: Jul 23, 2015
*/
#pragma once

#include <iostream>
#include <map>
#include "heading.h"

namespace FuzzyFW {

#define FUZZYFW_COMMENT_CHAR '#'
#define FUZZYFW_PARAM_SEPARATOR '='


/**
* Class to keep all the parameters read from a configuration file
* It can be useful to load any kind of file
*
* The input file must be a text file, and each parameter is defined in
* a line following the format "parameter_name = parameter_value"
*
*
*/
class ParameterDB {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/**
	* Maps of parameters.
	*/
	std::map<std::string, std::string> parameters;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*
	*/
	ParameterDB();

	/**
	* Constructor based on a configuration file path
	*
	* @param fileName Path to the configuration file
	*/
	ParameterDB(const char * fileName);



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Set a value for a given parameter. If the parameter exists in the
	* database, its value is override. If it does not exist, it is added.
	*
	* @param parameter Name of the parameter
	* @param value Value for the parameter
	*/
	void setParameter(std::string parameter, std::string value);


	/**
	* Get the value of a given parameter
	*
	* @param parameter Name of the parameter
	* @returns Value for the parameter (empty string if it does not exist)
	*/
	std::string getString(std::string parameter) const;
	std::string getStringUpper(std::string parameter) const;
	std::string getStringLower(std::string parameter) const;


	/**
	* Get the value of a given parameter as a double
	*
	* @param parameter Name of the parameter
	* @param defaultValue default value if the parameter is not found
	* @returns Value for the parameter
	*/
	double getDouble(std::string parameter, double defaultValue = -1.0) const;


	/**
	* Get the value of a given parameter as a int
	*
	* @param parameter Name of the parameter
	* @param defaultValue default value if the parameter is not found
	* @returns Value for the parameter
	*/
	int getInteger(std::string parameter, int defaultValue = -1) const;


	/**
	* Get the value of a given parameter as a bool
	*
	* @param parameter Name of the parameter
	* @param defaultValue default value if the parameter is not found
	* @returns Value for the parameter
	*/
	bool getBoolean(std::string parameter, bool defaultValue = false) const;


protected:
	/**
	* Reads the next configuration line of a file
	*
	* @param inputFile Configuration file with the parameters
	* @param label The function will store here the name of the parameter
	* @param value The function will store here the value of the parameter
	* @return false if there are not more parameters to read
	*/
	bool nextParameter(std::ifstream & inputFile, std::string & label,
		std::string & value);
};


}
