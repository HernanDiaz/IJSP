/*
* ParameterDB.cpp
*
*  Created on: Jul 23, 2015
*/

#include "ParameterDB.h"

namespace FuzzyFW {


//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
ParameterDB::ParameterDB() { }


//-----  Main constructor  ----------------------------------------------------
ParameterDB::ParameterDB(const char * fileName) {
	std::ifstream fileReader;

	fileReader.open(fileName);

#ifdef _DEBUG
	char cwd[1024];
	if (getWorkDir(cwd, sizeof(cwd)) != NULL)
		std::cout << cwd;
#endif

	if (!fileReader.is_open()) {
		std::string error = "Configuration file ";
		error += fileName;
		error += " not found.";
		throw FuzzyFWException("Loading Parameters", error);
	}

	std::string label, value;
	while (nextParameter(fileReader, label, value)) {
		this->setParameter(label, value);
	}
	fileReader.close();
}




//=============================================================================
//		METHODS
//=============================================================================
//-----  setParamneter  -------------------------------------------------------
void ParameterDB::setParameter(std::string parameter, std::string value) {
	// Insert the new parameter in the database
	std::pair<std::map<std::string, std::string>::iterator, bool> iter =
		parameters.insert(std::pair<std::string, std::string>
		(toUpper(parameter), value));

	// If it already exists, overwrite the value
	if (!iter.second) {
		iter.first->second = value;
	}
}


//-----  getString  -----------------------------------------------------------
std::string ParameterDB::getString(std::string parameter) const {
	std::map<std::string, std::string>::const_iterator entry;
	entry = this->parameters.find(toUpper(parameter));

	// If the parameter does not exist
	if (entry == parameters.end())
		return "";
	return entry->second;
}


//-----  getStringUpper  ------------------------------------------------------
std::string ParameterDB::getStringUpper(std::string parameter) const {
	std::map<std::string, std::string>::const_iterator entry;
	entry = this->parameters.find(toUpper(parameter));

	// If the parameter does not exist
	if (entry == parameters.end())
		return "";
	return toUpper(entry->second);
}


//-----  getStringLower  ------------------------------------------------------
std::string ParameterDB::getStringLower(std::string parameter) const {
	std::map<std::string, std::string>::const_iterator entry;
	entry = this->parameters.find(toUpper(parameter));

	// If the parameter does not exist
	if (entry == parameters.end())
		return "";
	return toLower(entry->second);
}


//-----  getDouble  -----------------------------------------------------------
double ParameterDB::getDouble(std::string parameter, double defaultValue)
	const {
	std::string value = this->getString(parameter);
	if (value == "")
		return defaultValue;
	return atof(value.c_str());
}


//-----  getInteger  ----------------------------------------------------------
int ParameterDB::getInteger(std::string parameter, int defaultValue) const {
	std::string value = this->getString(parameter);
	if (value == "")
		return defaultValue;
	return atoi(value.c_str());
}


//-----  getBoolean  -----------------------------------------------------------
bool ParameterDB::getBoolean(std::string parameter, bool defaultValue) const {
	std::string value = this->getString(parameter);
	if (value == "")
		return defaultValue;
	else if (toupper(value[0]) == 'T' || toupper(value[0]) == 'Y')
		return true;
	else if (toupper(value[0]) == 'F' || toupper(value[0]) == 'N')
		return false;
	return defaultValue;
}


//-----  readConfigLine  ------------------------------------------------------
bool ParameterDB::nextParameter(std::ifstream & inputFile,
	std::string & label, std::string & value) {

	label.clear();
	value.clear();

	std::string line;
	bool lineFound = false;
	while (!lineFound) {
		if (!inputFile.eof())
			getline(inputFile, line);
		else
			return false;

		int pos = 0;
		while (pos < (int)line.size() &&
			(line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\r'))
			pos++;

		if (pos < (int)line.size() && line[pos] != FUZZYFW_COMMENT_CHAR) {
			lineFound = true;
		}
	}


	bool val = false;
	for (int i = 0; i < (int)line.size(); i++) {
		if (line[i] == FUZZYFW_PARAM_SEPARATOR) {
			val = true;
			continue;
		}
		if (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')
			continue;
		if (line[i] == FUZZYFW_COMMENT_CHAR)
			break;
		if (!val)
			label.push_back(line[i]);
		else
			value.push_back(line[i]);
	}

	return true;
}

}
