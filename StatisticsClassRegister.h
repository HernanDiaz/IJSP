/*
 * StatisticsClassRegister.h
 *
 *  Created on: Oct 4, 2017
 *      Author: Juan Jose Palacios
 */
#ifndef STATISTICS_CLASSREGISTER_H_
#define STATISTICS_CLASSREGISTER_H_


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


#include "Statistics.h"
// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************







namespace FJSP {

/**
 * This static class allows to create the different kinds of statistics elements.
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
 * @author Juan Jose Palacios
 *
 */
struct StatisticsClassRegister {
protected:
	// Methods to create an instance of different operators.
	//=========================================================================
	template<typename T>
	static Statistics * createStatsInstance() { return new T; }


	// Mapping for each operator. These is the register of all classes and is
	// used to create objects of those classes from a string.
	//=========================================================================
	static std::map<std::string, Statistics*(*)()> StatsMap;



public:
	/**
	* Method to set Statistics from the name given in the
	* configuration file
	*
	* @param name Name of the SGS to use
	* @return An object of the specified type of FuzzySGS. Null if the
	* type is not registered
	*/
	static Statistics * getStatsObject(std::string name) {
		std::map<std::string, Statistics*(*)()>::iterator iter;
		iter = StatisticsClassRegister::StatsMap.find(toUpper(name));
		if (iter == StatsMap.end())
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

		// -----  STATISTICS TYPE  --------------------------------------------
		StatsMap[toUpper("makespan")] = &createStatsInstance<StatisticsMakespan>;
		StatsMap[toUpper("cmax")] = &createStatsInstance<StatisticsMakespan>;
		StatsMap[toUpper("aimin")] = &createStatsInstance<StatisticsAImin>;
		StatsMap[toUpper("ai_min")] = &createStatsInstance<StatisticsAImin>;
		StatsMap[toUpper("aiavg")] = &createStatsInstance<StatisticsAIavg>;
		StatsMap[toUpper("ai_avg")] = &createStatsInstance<StatisticsAIavg>;
	}

};

}


#endif /* SCHEDULING_CLASSREGISTER_H_ */
