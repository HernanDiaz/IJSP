/*
 * StatisticsClassRegister.h
 *
 *  Created on: Oct 4, 2017
 *      Author: Juan Jose Palacios
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



// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************
#include "StatisticsDiversity.h"
#include "StatisticsFJSP.h"
#include "StatisticsIJSP.h"
#include "StatisticsFVRP.h"






namespace FuzzyFW {

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

		// -----  Framework OBJECTS  ------------------------------------------
		StatsMap[toUpper("hamming")] = &createStatsInstance<StatisticsHamming>;
		StatsMap[toUpper("kendall")] = &createStatsInstance<StatisticsKendall>;
		StatsMap[toUpper("kendall-tau")] = &createStatsInstance<StatisticsKendall>;
		StatsMap[toUpper("neri")] = &createStatsInstance<StatisticsNeri>;
		StatsMap[toUpper("neri-best")] = &createStatsInstance<StatisticsNeriBest>;

		// -----  FJSP OBJECTS  -----------------------------------------------
		StatsMap[toUpper("makespan")] = &createStatsInstance<FJSP::StatisticsMakespan>;
		StatsMap[toUpper("cmax")] = &createStatsInstance<FJSP::StatisticsMakespan>;
		StatsMap[toUpper("aimin")] = &createStatsInstance<FJSP::StatisticsAImin>;
		StatsMap[toUpper("ai_min")] = &createStatsInstance<FJSP::StatisticsAImin>;
		StatsMap[toUpper("aiavg")] = &createStatsInstance<FJSP::StatisticsAIavg>;
		StatsMap[toUpper("ai_avg")] = &createStatsInstance<FJSP::StatisticsAIavg>;
		StatsMap[toUpper("esdmin")] = &createStatsInstance<FJSP::StatisticsESDmin>;
		StatsMap[toUpper("esd_min")] = &createStatsInstance<FJSP::StatisticsESDmin>;
		StatsMap[toUpper("esdavg")] = &createStatsInstance<FJSP::StatisticsESDavg>;
		StatsMap[toUpper("esd_avg")] = &createStatsInstance<FJSP::StatisticsESDavg>;

		// -----  IJSP OBJECTS  -----------------------------------------------
		StatsMap[toUpper("ijsp.makespan")] = &createStatsInstance<IJSP::StatisticsMakespan>;
		StatsMap[toUpper("ijsp.cmax")] = &createStatsInstance<IJSP::StatisticsMakespan>;
	
		// -----  FVRP OBJECTS  -----------------------------------------------
		StatsMap[toUpper("cost")] = &createStatsInstance<FVRP::StatisticsTimeCost>;
		StatsMap[toUpper("travelcost")] = &createStatsInstance<FVRP::StatisticsTimeCost>;
		StatsMap[toUpper("necessity")] = &createStatsInstance<FVRP::StatisticsDemandMinNec>;
		StatsMap[toUpper("minnec")] = &createStatsInstance<FVRP::StatisticsDemandMinNec>;
		StatsMap[toUpper("possibility")] = &createStatsInstance<FVRP::StatisticsDemandMinPos>;
		StatsMap[toUpper("minpos")] = &createStatsInstance<FVRP::StatisticsDemandMinNec>;
		StatsMap[toUpper("credibility")] = &createStatsInstance<FVRP::StatisticsDemandMinCred>;
		StatsMap[toUpper("mincred")] = &createStatsInstance<FVRP::StatisticsDemandMinNec>;
	}
};

}
