/*
 * RobustnessClassRegister.h
 *
 *  Created on: Jul 04, 2020
 *      Author: Hernan Diaz Rodriguez
 */
#pragma once


/*=============================================================================
 *
 * IF A NEW CLASS IS CREATED, IT MUST BE REGISTERED AT THE END OF THIS FILE,
 * INSIDE THE registerClasses METHOD.
 *
 * THE FILE IN WHICH THE NEW CLASS HAS BEEN IMPLEMENTED MUST BE INCLUDED
 * IN THE HEADING OF THIS FILE
 *
 * C++ does not allow to specify classes at runtime by means of strings.
 * However, this can be done by creating a map between strings and classes.
 *
 * This file provides that map and has to be updated every time a new class
 * is implemented in this framework.
 *
 * The function registerClassTypes must be called before using any of the
 * problem components
=============================================================================*/

#include "IJSPRobustnessAnalyzerMakespan.h"
#include "IJSPRobustnessAnalyzerTardiness.h"


// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************







namespace PostExecution {

/**
 * This static class allows to create the different kinds of problem elements.
 * This is made so the user can implement a new element, just register it
 * here and use it immediately by specifying it in the configuration file.
 *
 * It may seem that we are complicating things here, but actually having
 * this class simplifies a lot the use of this framework we are implementing
 * It is much easier to load the parameters from the configuration file and
 * also is much easier for the incoming programmers because they can abstract
 * from the structure of the framework. They just implement their new operators
 * by inheriting from the respective abstract class and than come here to
 * register the name of the new operator.
 *
 * @author hdiaz
 *
 */
struct RobustnessClassRegister {
protected:
	// Methods to create an instance of different operators.
	//=========================================================================
	template<typename T>
	static RobustnessAnalyzer* createRobustnessInstance() { return new T; }


	// Mapping for each operator. These is the register of all classes and is
	// used to create objects of those classes from a string.
	//=========================================================================
	static std::map<std::string, RobustnessAnalyzer*(*)()> SGSMap;



public:
	/**
	* Method to create a SGS from the name given in the
	* configuration file
	*
	* @param name Name of the SGS to use
	* @return An object of the specified type of SGS_FJSP. Null if the
	* type is not registered
	*/

	static RobustnessAnalyzer * getRobustnessObject(std::string name) {
		std::map<std::string, RobustnessAnalyzer*(*)()>::iterator iter;
		iter = RobustnessClassRegister::SGSMap.find(toUpper(name));
		if (iter == SGSMap.end())
			return NULL;
		return iter->second();
	}
		
	// ************************************************************************
	//
	// ADD HERE ALL YOUR NEW CLASSES WITH THE NAME YOU WANT TO USE IN THE
	// CONFIGURATION FILE.
	// PLEASE, PLACE YOUR ELEMENTS IN THE CORRECT SECTION
	//
	// ************************************************************************
	/**
	 * Method to register all the classes and assign them a name to be used
	 * in the configuration files
	 */
	static void registerClasses() {

		SGSMap[toUpper("ijsp.tardiness")] = &createRobustnessInstance<IJSPRobustnessAnalyzerTardiness>;
		SGSMap[toUpper("ijsp.makespan")] = &createRobustnessInstance<IJSPRobustnessAnalyzerMakespan>;
	}

};

}
