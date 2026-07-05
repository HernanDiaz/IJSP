/*
 * LocalSearchClassRegister.h
 *
 *  Created on: Oct 13, 2017
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


#include "LocalSearchAmico.h"

// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************
#include "NeighbourhoodFJSP_Cmax.h"
#include "NeighbourhoodIJSP_Cmax.h"
#include "NeighbourhoodIJSP_N8.h"
#include "NeighbourhoodIJSP_N2ME.h"
#include "NeighbourhoodFJSP_AI.h"
#include "NeighbourhoodFJSP_ESD.h"
#include "LocalSearchAmicoFJSP.h"








namespace FuzzyFW {

/**
 * This static class allows to create the different kinds of loca search
 * algorithms and neighbourhood structures
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
 *
 */
struct LocalSearchClassRegister {
protected:
	// Methods to create an instance of different operators.
	//=========================================================================
	template<typename T>
	static LocalSearch * createLocalSearchInstance() { return new T; }

	template<typename T>
	static Neighbourhood * createNeighbourhoodInstance() { return new T; }


	// Mapping for each operator. These is the register of all classes and is
	// used to create objects of those classes from a string.
	//=========================================================================
	static std::map<std::string, LocalSearch*(*)()> LocalSearchMap;

	static std::map<std::string, Neighbourhood*(*)()> NeighbourhoodMap;



public:
	/**
	* Method to create the Local Search algorithm
	*
	* @param name Name of the algorithm to use
	* @return An object of the specified type
	*/
	static LocalSearch * getLocalSearchObject(std::string name) {
		std::map<std::string, LocalSearch*(*)()>::iterator iter;
		iter = LocalSearchClassRegister::LocalSearchMap.find(toUpper(name));
		if (iter == LocalSearchMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to get a Niehgbourhood structure
	*
	* @param name Name of the Neighbourhood to use
	* @return An object of the specified type
	*/
	static Neighbourhood * getNeighbourhoodObject(std::string name) {
		std::map<std::string, Neighbourhood*(*)()>::iterator iter;
		iter = LocalSearchClassRegister::NeighbourhoodMap.find(toUpper(name));
		if (iter == NeighbourhoodMap.end())
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

		// -----  LOCAL SEARCH METHOD  ----------------------------------------
		LocalSearchMap[toUpper("hillclimbing")] =
			&createLocalSearchInstance<LS_HillClimbing>;
		LocalSearchMap[toUpper("hill-climbing")] =
			&createLocalSearchInstance<LS_HillClimbing>;
		LocalSearchMap[toUpper("hc")] =
			&createLocalSearchInstance<LS_HillClimbing>;
		LocalSearchMap[toUpper("gradientdescent")] =
			&createLocalSearchInstance<LS_GradientDescent>;
		LocalSearchMap[toUpper("gradient-descent")] =
			&createLocalSearchInstance<LS_GradientDescent>;
		LocalSearchMap[toUpper("tabu-search")] =
			&createLocalSearchInstance<LS_Tabu>;
		LocalSearchMap[toUpper("tabu")] =
			&createLocalSearchInstance<LS_Tabu>;
		LocalSearchMap[toUpper("amico")] =
			&createLocalSearchInstance<LS_Tabu_Amico>;
		LocalSearchMap[toUpper("amicofjsp")] =
			&createLocalSearchInstance<FJSP::LS_Tabu_Amico_FJSP>;


		// -----  NEIGHBOURHOOD STRUCTURES  FJSP---------------------------------------
		NeighbourhoodMap[toUpper("fjsp.makespan.n1")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_MakespanFJSP>;
		NeighbourhoodMap[toUpper("fjsp.aiavg.n1")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_AIavgFJSP>;
		NeighbourhoodMap[toUpper("fjsp.aimin.n1")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_AIminFJSP>;
		NeighbourhoodMap[toUpper("fjsp.aimin.ICAE")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_AIminICAE>;
		NeighbourhoodMap[toUpper("fjsp.esdavg.n1")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_ESDavgFJSP>;
		NeighbourhoodMap[toUpper("fjsp.esdmin.n1")] =
			&createNeighbourhoodInstance<FJSP::NB_ParallelN1_ESDminFJSP>;

		// -----  NEIGHBOURHOOD STRUCTURES  IJSP---------------------------------------
		NeighbourhoodMap[toUpper("ijsp.makespan.n1")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN1_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n2")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN2_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n3")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN3_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.nh")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelNH_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.next")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelNext_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n2inter")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN2Inter_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n2minus")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN2Minus_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n2plus")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN2Plus_MakespanIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan-energy.n2me")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN2ME_MakespanEnergyIJSP>;
		NeighbourhoodMap[toUpper("ijsp.makespan.n8")] =
			&createNeighbourhoodInstance<IJSP::NB_ParallelN8_MakespanIJSP>;
	}

};

}
