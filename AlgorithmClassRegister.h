/*
* AlgorithmClassRegister.h
*
*  Created on: Sep 20, 2017
*/
#pragma once

/*=============================================================================
 *
 * Maps algorithm name strings to factory functions for EvolutiveAlgorithm
 * subclasses. Add a new algorithm by including its header and registering it
 * inside registerClasses().
 *
 * This file is the single place that knows about concrete algorithm types,
 * keeping EvoLauncher decoupled from them.
 *
=============================================================================*/

#include "MemeticNeri.h"
#include "ArtificialBeeColony.h"
#include "ArtificialBeeColonyThread.h"
#include "ArtificialBeeColonyPSO.h"
#include "ABCPSOPareto.h"
#include "MemeticPareto.h"
#include "SweepPareto.h"
#include "ArtificialBeeColonyCell.h"
#include "RandomPopulation.h"

namespace FuzzyFW {

struct AlgorithmClassRegister {
protected:
	template<typename T>
	static EvolutiveAlgorithm * createInstance(ParameterDB *p) { return new T(p); }

	using AlgorithmFactory = EvolutiveAlgorithm*(*)(ParameterDB*);
	static std::map<std::string, AlgorithmFactory> AlgorithmMap;

public:
	static EvolutiveAlgorithm * getAlgorithm(const std::string &name, ParameterDB *params) {
		auto it = AlgorithmMap.find(toUpper(name));
		if (it == AlgorithmMap.end())
			return NULL;
		return it->second(params);
	}

	// ****************************************************************************
	//
	// ADD HERE ALL ALGORITHM CLASSES WITH THE NAME TO USE IN THE CONFIG FILE.
	//
	// ****************************************************************************
	static void registerClasses() {
		AlgorithmMap[toUpper("GENETIC")]  = &createInstance<GeneticAlgorithm>;
		AlgorithmMap[toUpper("GA")]       = &createInstance<GeneticAlgorithm>;
		AlgorithmMap[toUpper("MEMETIC")]  = &createInstance<MemeticAlgorithm>;
		AlgorithmMap[toUpper("MA")]       = &createInstance<MemeticAlgorithm>;
		AlgorithmMap[toUpper("NERI")]     = &createInstance<MemeticNeri>;
		AlgorithmMap[toUpper("NERI-MA")] = &createInstance<MemeticNeri>;
		AlgorithmMap[toUpper("GA-TOOL-RANDOMPOPULATION")] = &createInstance<RandomPopulation>;
		AlgorithmMap[toUpper("ABC")]      = &createInstance<ArtificialBeeColony>;
		AlgorithmMap[toUpper("ABCT")]     = &createInstance<ArtificialBeeColonyThread>;
		AlgorithmMap[toUpper("ABCPSO")]   = &createInstance<ArtificialBeeColonyPSO>;
		AlgorithmMap[toUpper("ABCPSO-PARETO")] = &createInstance<ABCPSOPareto>;
		AlgorithmMap[toUpper("MEMETIC-PARETO")] = &createInstance<MemeticPareto>;
		AlgorithmMap[toUpper("SWEEP-PARETO")] = &createInstance<SweepPareto>;
		AlgorithmMap[toUpper("ABCCELL")]  = &createInstance<ArtificialBeeColonyCell>;
	}
};

}
