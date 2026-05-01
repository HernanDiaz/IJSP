/*
 * GeneticClassRegister.h
 *
 *  Created on: Aug 1, 2017
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


#include "Mutation.h"
#include "Selection.h"
#include "Replacement.h"
// ****************************************************************************
//
// ADD HERE ALL THE HEADER FILES OF THE OPERATORS YOU HAVE CREATED
//
// ****************************************************************************
#include "CreationFJSP.h"
#include "CrossoverFJSP.h"
#include "CrossoverIJSP.h"
#include "DecoderFJSP.h"
#include "DecoderFVRP.h"
#include "DecoderIJSP.h"
#include "CreationIJSP.h"




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
 *
 */
struct GeneticClassRegister {
protected:
	// Methods to create an instance of different operators.
	//=========================================================================
	template<typename T>
	static Encoder * createEncoderInstance() { return new T; }

	template<typename T>
	static Decoder * createDecoderInstance() { return new T; }

	template<typename T>
	static Creation * createCreationInstance() { return new T; }

	template<typename T>
	static Crossover * createCrossoverInstance() { return new T; }

	template<typename T>
	static Mutation * createMutationInstance() { return new T; }

	template<typename T>
	static Selection * createSelectionInstance() { return new T; }

	template<typename T>
	static Replacement * createReplacementInstance() { return new T; }

	// Mapping for each operator. These is the register of all classes and is
	// used to create objects of those classes from a string.
	//=========================================================================
	static std::map<std::string, Encoder*(*)()>	EncoderMap;
	static std::map<std::string, Decoder*(*)()>	DecoderMap;
	static std::map<std::string, Creation*(*)()> CreationMap;
	static std::map<std::string, Crossover*(*)()> CrossoverMap;
	static std::map<std::string, Mutation*(*)()> MutationMap;
	static std::map<std::string, Selection*(*)()> SelectionMap;
	static std::map<std::string, Replacement*(*)()> ReplacementMap;



public:
	/**
	 * Method to select and Encoding strategy from the name given in the
	 * configuration file
	 *
	 * @param name Name of the encoding strategy to use
	 * @return An object of the specified encoding method. Null if the
	 * type is not registered
	 */
	static Encoder * getEncoderObject(std::string name) {
		std::map<std::string, Encoder*(*)()>::iterator iter;
		iter = GeneticClassRegister::EncoderMap.find(toUpper(name));
		if (iter == EncoderMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to select and Encoding strategy from the name given in the
	* configuration file
	*
	* @param name Name of the encoding strategy to use
	* @return An object of the specified encoding method. Null if the
	* type is not registered
	*/
	static Decoder * getDecoderObject(std::string name) {
		std::map<std::string, Decoder*(*)()>::iterator iter;
		iter = GeneticClassRegister::DecoderMap.find(toUpper(name));
		if (iter == DecoderMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to select a Creation operator from the name given in the
	* configuration file
	*
	* @param name Name of the encoding strategy to use
	* @return An object of the specified encoding method. Null if the
	* type is not registered
	*/
	static Creation * getCreationObject(std::string name) {
		std::map<std::string, Creation*(*)()>::iterator iter;
		iter = GeneticClassRegister::CreationMap.find(toUpper(name));
		if (iter == CreationMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to set a Crossover operator from the name given in the
	* configuration file
	*
	* @param name Name of the operator to use
	* @return An object of the specified type of Crossover operator
	* Null if the type is not registered
	*/
	static Crossover * getCrossoverObject(std::string name) {
		std::map<std::string, Crossover*(*)()>::iterator iter;
		iter = GeneticClassRegister::CrossoverMap.find(toUpper(name));
		if (iter == CrossoverMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to set a Mutation operator from the name given in the
	* configuration file
	*
	* @param name Name of the operator to use
	* @return An object of the specified type of Mutation operator.
	* Null if the type is not registered
	*/
	static Mutation * getMutationObject(std::string name) {
		std::map<std::string, Mutation*(*)()>::iterator iter;
		iter = GeneticClassRegister::MutationMap.find(toUpper(name));
		if (iter == MutationMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to set a Selection process from the name given in the
	* configuration file
	*
	* @param name Name of the strategy function to use
	* @return An object of the specified type of Selection strategy
	* Null if the type is not registered
	*/
	static Selection * getSelectionObject(std::string name) {
		std::map<std::string, Selection*(*)()>::iterator iter;
		iter = GeneticClassRegister::SelectionMap.find(toUpper(name));
		if (iter == SelectionMap.end())
			return NULL;
		return iter->second();
	}

	/**
	* Method to set an a Replacement strategy from the name given in the
	* configuration file
	*
	* @param name Name of the strategy to use
	* @return An object of the specified type of Replacement strategy.
	* Null if the type is not registered
	*/
	static Replacement * getReplacementObject(std::string name) {
		std::map<std::string, Replacement*(*)()>::iterator iter;
		iter = GeneticClassRegister::ReplacementMap.find(toUpper(name));
		if (iter == ReplacementMap.end())
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

		// -----  ENCODING FUNCTION  ------------------------------------------
		
		// -----  DECODING FUNCTION  ------------------------------------------

		// -----  CREATION OPERATORS  -----------------------------------------

		// -----  CROSSOVER OPERATORS  ----------------------------------------
		CrossoverMap[toUpper("obc")] = &createCrossoverInstance<Crossover_OBC>;
		CrossoverMap[toUpper("cbc")] = &createCrossoverInstance<Crossover_CBC>;
		CrossoverMap[toUpper("pmx")] = &createCrossoverInstance<Crossover_PMX>;

		// -----  MUTATION OPERATORS  -----------------------------------------
		MutationMap[toUpper("insertion")] = &createMutationInstance<MutationInsertion>;
		MutationMap[toUpper("inversion")] = &createMutationInstance<MutationInversion>;
		MutationMap[toUpper("swap")] = &createMutationInstance<MutationSwap>;

		// -----  SELECTION OPERATORS  ----------------------------------------
		SelectionMap[toUpper("random")] = &createSelectionInstance<SelectionRandom>;
		SelectionMap[toUpper("shuffle")] = &createSelectionInstance<SelectionShuffle>;
		SelectionMap[toUpper("sus")] = &createSelectionInstance<SelectionSUS>;
		SelectionMap[toUpper("roulette")] = &createSelectionInstance<SelectionRoulette>;
		SelectionMap[toUpper("tournament")] = &createSelectionInstance<SelectionTournament>;
		SelectionMap[toUpper("elite")] = &createSelectionInstance <SelectionElite>;
		SelectionMap[toUpper("L5")] = &createSelectionInstance <SelectionCellL5>;
		SelectionMap[toUpper("L9")] = &createSelectionInstance <SelectionCellL9>;
		SelectionMap[toUpper("C9")] = &createSelectionInstance <SelectionCellC9>;
		SelectionMap[toUpper("C13")] = &createSelectionInstance <SelectionCellC13>;
		SelectionMap[toUpper("R5")] = &createSelectionInstance <SelectionCellR5>;
		SelectionMap[toUpper("R13")] = &createSelectionInstance <SelectionCellR13>;
		
		// -----  REPLACEMENT OPERATORS  --------------------------------------
		ReplacementMap[toUpper("generational")] = &createReplacementInstance<ReplacementElitist>;
		ReplacementMap[toUpper("simple")] = &createReplacementInstance<ReplacementElitist>;
		ReplacementMap[toUpper("elitism")] = &createReplacementInstance<ReplacementElitist>;
		ReplacementMap[toUpper("parents")] = &createReplacementInstance<ReplacementParents>;
		ReplacementMap[toUpper("tournament")] = &createReplacementInstance<ReplacementParents>;



		// --------------------------------------------------------------------
		//			FJSP
		// --------------------------------------------------------------------
		// -----  ENCODING FUNCTION  ------------------------------------------
		EncoderMap[toUpper("permutation")] = &createEncoderInstance<FJSP::EncoderFJSP_Order>;
		EncoderMap[toUpper("task-order")] = &createEncoderInstance<FJSP::EncoderFJSP_Order>;
		EncoderMap[toUpper("taskorder")] = &createEncoderInstance<FJSP::EncoderFJSP_Order>;
		EncoderMap[toUpper("permutation-reap")] = &createEncoderInstance<FJSP::EncoderFJSP_JobOrder>;
		EncoderMap[toUpper("permutationreap")] = &createEncoderInstance<FJSP::EncoderFJSP_JobOrder>;
		//EncoderMap[toUpper("job-order")] = &createEncoderInstance<FJSP::EncoderFJSP_JobOrder>;
		//EncoderMap[toUpper("joborder")] = &createEncoderInstance<FJSP::EncoderFJSP_JobOrder>;
	
		// -----  DECODING FUNCTION  ------------------------------------------
		DecoderMap[toUpper("permutation")] = &createDecoderInstance<FJSP::DecoderFJSP_Order>;
		DecoderMap[toUpper("task-order")] = &createDecoderInstance<FJSP::DecoderFJSP_Order>;
		DecoderMap[toUpper("taskorder")] = &createDecoderInstance<FJSP::DecoderFJSP_Order>;
		DecoderMap[toUpper("permutation-reap")] = &createDecoderInstance<FJSP::DecoderFJSP_JobOrder>;
		DecoderMap[toUpper("permutationreap")] = &createDecoderInstance<FJSP::DecoderFJSP_JobOrder>;
		DecoderMap[toUpper("job-order")] = &createDecoderInstance<FJSP::DecoderFJSP_JobOrder>;
		DecoderMap[toUpper("joborder")] = &createDecoderInstance<FJSP::DecoderFJSP_JobOrder>;
		

		// -----  CREATION OPERATORS  -----------------------------------------
		CreationMap[toUpper("fjsp-random")] = &createCreationInstance<FJSP::CreationRandomSchedule>;
		CreationMap[toUpper("fjsprandom")] = &createCreationInstance<FJSP::CreationRandomSchedule>;
	

		// -----  CROSSOVER OPERATORS  ----------------------------------------
		CrossoverMap[toUpper("jox")] = &createCrossoverInstance<FJSP::Crossover_JOX>;
		CrossoverMap[toUpper("gox")] = &createCrossoverInstance<FJSP::Crossover_GOXBierwirth>;
		CrossoverMap[toUpper("biertwirth")] = &createCrossoverInstance<FJSP::Crossover_GOXBierwirth>;
		CrossoverMap[toUpper("gpmx")] = &createCrossoverInstance<FJSP::Crossover_GPMXBierwirth>;
		CrossoverMap[toUpper("ppx")] = &createCrossoverInstance<FJSP::Crossover_PPXBierwirth>;

		// -----  MUTATION OPERATORS  -----------------------------------------

		// -----  SELECTION OPERATORS  ----------------------------------------

		// -----  REPLACEMENT OPERATORS  --------------------------------------


		// --------------------------------------------------------------------
		//			IJSP
		// --------------------------------------------------------------------
		// -----  ENCODING FUNCTION  ------------------------------------------
		EncoderMap[toUpper("ijsp.permutation")] = &createEncoderInstance<IJSP::EncoderIJSP_Order>;
		EncoderMap[toUpper("ijsp.task-order")] = &createEncoderInstance<IJSP::EncoderIJSP_Order>;
		EncoderMap[toUpper("ijsp.taskorder")] = &createEncoderInstance<IJSP::EncoderIJSP_Order>;
		EncoderMap[toUpper("ijsp.permutation-reap")] = &createEncoderInstance<IJSP::EncoderIJSP_JobOrder>;
		EncoderMap[toUpper("ijsp.permutationreap")] = &createEncoderInstance<IJSP::EncoderIJSP_JobOrder>;
		EncoderMap[toUpper("ijsp.job-order")] = &createEncoderInstance<IJSP::EncoderIJSP_JobOrder>;
		EncoderMap[toUpper("ijsp.joborder")] = &createEncoderInstance<IJSP::EncoderIJSP_JobOrder>;

		// -----  DECODING FUNCTION  ------------------------------------------
		DecoderMap[toUpper("ijsp.permutation")] = &createDecoderInstance<IJSP::DecoderIJSP_Order>;
		DecoderMap[toUpper("ijsp.task-order")] = &createDecoderInstance<IJSP::DecoderIJSP_Order>;
		DecoderMap[toUpper("ijsp.taskorder")] = &createDecoderInstance<IJSP::DecoderIJSP_Order>;
		DecoderMap[toUpper("ijsp.permutation-reap")] = &createDecoderInstance<IJSP::DecoderIJSP_JobOrder>;
		DecoderMap[toUpper("ijsp.permutationreap")] = &createDecoderInstance<IJSP::DecoderIJSP_JobOrder>;
		DecoderMap[toUpper("ijsp.job-order")] = &createDecoderInstance<IJSP::DecoderIJSP_JobOrder>;
		DecoderMap[toUpper("ijsp.joborder")] = &createDecoderInstance<IJSP::DecoderIJSP_JobOrder>;

		// -----  CREATION OPERATORS  -----------------------------------------
    	CreationMap[toUpper("ijsp-random")] = &createCreationInstance<IJSP::CreationRandomSchedule>;
		CreationMap[toUpper("ijsp.random")] = &createCreationInstance<IJSP::CreationRandomSchedule>;
		CreationMap[toUpper("ijsp.SRT.makespan")] = &createCreationInstance<IJSP::CreationSRTIntervalMkSchedule>;
		CreationMap[toUpper("ijsp.SNTF.makespan")] = &createCreationInstance<IJSP::CreationSNTFIntervalMkSchedule>;
		
		CreationMap[toUpper("ijsp.SPJF.makespan")] = &createCreationInstance<IJSP::CreationSPJFIntervalMkSchedule>;		   //Shortest processed time job first
		CreationMap[toUpper("ijsp.LPJF.makespan")] = &createCreationInstance<IJSP::CreationSPJFInverseIntervalMkSchedule>; //Longest processed time job first
		CreationMap[toUpper("ijsp.SRTF.makespan")] = &createCreationInstance<IJSP::CreationLRTFInverseIntervalMkSchedule>; //Shortest remaining time first
		CreationMap[toUpper("ijsp.LRTF.makespan")] = &createCreationInstance<IJSP::CreationLRTFIntervalMkSchedule>;        //Longest remaining time first
		CreationMap[toUpper("ijsp.STPT.makespan")] = &createCreationInstance<IJSP::CreationSCTFIntervalMkSchedule>;        //Shortest total processing time
		CreationMap[toUpper("ijsp.LTPT.makespan")] = &createCreationInstance<IJSP::CreationLCTFIntervalMkSchedule>;		   //Longest total processing time
		CreationMap[toUpper("ijsp.Manager.makespan")] = &createCreationInstance<IJSP::CreationManagerIntervalMkSchedule>;
		
		// -----  CROSSOVER OPERATORS  ----------------------------------------
		CrossoverMap[toUpper("ijsp.jox")] = &createCrossoverInstance<IJSP::Crossover_JOX>;
		CrossoverMap[toUpper("ijsp.gox")] = &createCrossoverInstance<IJSP::Crossover_GOXBierwirth>;
		CrossoverMap[toUpper("ijsp.biertwirth")] = &createCrossoverInstance<IJSP::Crossover_GOXBierwirth>;
		CrossoverMap[toUpper("ijsp.gpmx")] = &createCrossoverInstance<IJSP::Crossover_GPMXBierwirth>;
		CrossoverMap[toUpper("ijsp.ppx")] = &createCrossoverInstance<IJSP::Crossover_PPXBierwirth>;


		// -----  MUTATION OPERATORS  -----------------------------------------

		// -----  SELECTION OPERATORS  ----------------------------------------

		// -----  REPLACEMENT OPERATORS  --------------------------------------


		// --------------------------------------------------------------------
		//			FVRP
		// --------------------------------------------------------------------
		// -----  ENCODING FUNCTION  ------------------------------------------
		EncoderMap[toUpper("permutation")] = &createEncoderInstance<FVRP::EncoderFVRP_Order>;

		// -----  DECODING FUNCTION  ------------------------------------------
		DecoderMap[toUpper("split")] = &createDecoderInstance<FVRP::DecoderFRVP_Split>;

		// -----  CREATION OPERATORS  -----------------------------------------

		// -----  CROSSOVER OPERATORS  ----------------------------------------

		// -----  MUTATION OPERATORS  -----------------------------------------

		// -----  SELECTION OPERATORS  ----------------------------------------

		// -----  REPLACEMENT OPERATORS  --------------------------------------



	}

};

}

