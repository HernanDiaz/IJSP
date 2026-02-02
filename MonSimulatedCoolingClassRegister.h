/*
 * SimulatedCoolingClassRegister.h
 *
 *  Created on: May 06, 2022
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


#include "SGS_IJSP_Insertion.h"
#include "SGS_IJSP_Append.h"
#include "MonotonicAdaptativeCooling.h"


// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************







namespace FuzzyFW {

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
struct MonSimulatedCoolingClassRegister {
protected:
	// Methods to create an instance 
	//=========================================================================
	template<typename T>
	static Monotonic_Adaptative_Cooling * createMACInstance() { return new T; }

	// Mapping. These is the register of all classes and is
	// used to create objects of those classes from a string.
	//=========================================================================
	static std::map<std::string, Monotonic_Adaptative_Cooling*(*)()> MACMap;
	
public:
	/**
	* Method to create a MAC from the name given in the
	* configuration file
	*
	* @param name Name of the MAC to use
	* @return An object of the specified type of MAC. Null if the
	* type is not registered
	*/
	   
	static Monotonic_Adaptative_Cooling * getMACObject(std::string name) {
		std::map<std::string, Monotonic_Adaptative_Cooling*(*)()>::iterator iter;
		iter = MonSimulatedCoolingClassRegister::MACMap.find(toUpper(name));
		if (iter == MACMap.end())
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
		MACMap[toUpper("Exponential_Multiplicative_Cooling")] = &createMACInstance<Exponential_Multiplicative_Cooling>;
		MACMap[toUpper("Logaritmic_Multiplicative_Cooling")] = &createMACInstance<Logaritmic_Multiplicative_Cooling>;
		MACMap[toUpper("Linear_Cooling")] = &createMACInstance<Linear_Cooling>;
		MACMap[toUpper("Linear_Multiplicative_Cooling")] = &createMACInstance<Linear_Multiplicative_Cooling>;
		MACMap[toUpper("Quadratic_Multiplicative_Cooling")] = &createMACInstance<Quadratic_Multiplicative_Cooling>;
		MACMap[toUpper("Linear_Additive_Cooling")] = &createMACInstance<Linear_Additive_Cooling>; 
		MACMap[toUpper("Quadratic_Additive_Cooling")] = &createMACInstance<Quadratic_Additive_Cooling>;
		MACMap[toUpper("Trigonometric_Additive_Cooling")] = &createMACInstance<Trigonometric_Additive_Cooling>;
	}

};

}

