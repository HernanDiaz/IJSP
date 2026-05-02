/*
 * SimulatedCooling.h
 *
 *  Created on: May 25, 2022
 *      Author: Hernan Diaz
 */
#include "Individual.h"
#include "SharedVarsEvolutionary.h"
#include "MonotonicAdaptativeCooling.h"
#include "MonSimulatedCoolingClassRegister.h"
#include <cmath>
namespace FuzzyFW {
	#define SIM_COOLING_A "simCooling.alpha"
	#define SIM_COOLING_T0 "simCooling.t0"
	#define MONOTONIC_COOLING_STRATEGY "monotonic_cooling_strategy"
	#define NON_MONOTONIC_COOLING_STRATEGY "non_monotonic_cooling_strategy"
	//=============================================================================
	//
	//	Class Monotonic_Adaptative_Cooling
	//  		
	//=============================================================================
	/**
	 * In the non-monotonic adaptive cooling, the system temperature T at each state transition is computed multiplying
	 * the temperature value Tk, obtained by any of the former criteria, by an adaptive factor m based on the difference 
	 * between the current solution objective,f(si), and the best objective achieved until that moment by the algorithm, 
	 * noted f*:
	 * @author hdiaz
	 *
	 */
	class Non_Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	
		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		Monotonic_Adaptative_Cooling* monotonic;

		Non_Monotonic_Adaptative_Cooling(){}

		virtual std::vector<std::string>getName() const = 0;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			// Loads the specific parameters
			std::string value;
			MonSimulatedCoolingClassRegister::registerClasses();
			// Loads the monotonic cooling strategy to use
			value = parameters->getStringLower(MONOTONIC_COOLING_STRATEGY);
			this->monotonic = MonSimulatedCoolingClassRegister::getMACObject(value);
			if (this->monotonic == NULL) {
				std::string errorMsg = "Invalid monotonic cooling strategy";
				throw FuzzyFWException("Monotonic Adaptative Cooling", errorMsg);
			}
			this->monotonic->setUp(parameters);
		}

		virtual bool isSelected(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny,
			const FuzzyFW::SharedVarsEvolutionary *svars,
			const int generation) {
			return svars->rng->getDouble(0, 1) < this->getTemperature(origin, destiny, generation);
		}

		virtual double getTemperature(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny, const int generation) {
			return this->getMonotonicTemperature(generation)*this->getAdaptativeFactor(origin, destiny);
		}

		virtual double getMonotonicTemperature(const int generation) {
			return monotonic->getTemperature(generation);
		}

		virtual double getAdaptativeFactor(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny) const = 0;

		virtual ~Non_Monotonic_Adaptative_Cooling() {
			if (monotonic != NULL) {
				delete monotonic;
			}
		}
			
	};

	//=============================================================================
	//
	//	Class Adaptive_Non_Mon_Cooling
	//  		
	//=============================================================================
	/**
	 *  M. Locatelli (2000)		
	 * @author hdiaz
	 *
	 */
	class Adaptive_Non_Mon_Cooling :public Non_Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:

		//=========================================================================
		//		METHODS
		//=========================================================================
	
		virtual double getAdaptativeFactor(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny) const;

		virtual std::vector<std::string> getName() const;
	
	};

	//=============================================================================
//
//	Class Adaptive_Non_Mon_Cooling
//  		
//=============================================================================
/**
 *  M. Locatelli (2000)
 * @author hdiaz
 *
 */
	class Adaptive_Quadratic_Non_Mon_Cooling :public Non_Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:

		//=========================================================================
		//		METHODS
		//=========================================================================

		virtual double getAdaptativeFactor(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny) const;

		virtual std::vector<std::string> getName() const;

	};

	/**
 *  M. Locatelli (2000)
 * @author hdiaz
 *
 */
	class Disabled_Non_Mon_Cooling :public Non_Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:

		//=========================================================================
		//		METHODS
		//=========================================================================

		virtual double getAdaptativeFactor(const FuzzyFW::Individual* origin,
			const FuzzyFW::Individual* destiny) const;

		virtual std::vector<std::string> getName() const;

	};

}

