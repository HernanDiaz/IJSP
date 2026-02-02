/*
 * SimulatedCooling.h
 *
 *  Created on: May 25, 2022
 *      Author: Hernan Diaz
 */
#pragma once
#include "Individual.h"
#include "SharedVarsEvolutionary.h"
#include <cmath>
namespace FuzzyFW {
	#define SIM_COOLING_A "simCooling.alpha"
	#define SIM_COOLING_T0 "simCooling.t0"
	#define SIM_COOLING_TN "simCooling.tn"
	#ifndef M_PI 
		#define M_PI (3.14159265358979323846)
	#endif

	//=============================================================================
	//
	//	Class Monotonic_Adaptative_Cooling
	//  		
	//=============================================================================
	/**
	 * @author hdiaz
	 *
	 */
	class Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	
		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		Monotonic_Adaptative_Cooling() {}

		virtual bool isSelected(const FuzzyFW::SharedVarsEvolutionary *svars,
			const int generation) {
			return svars->rng->getDouble(0, 1) < this->getTemperature(generation);
		}

		virtual double getTemperature(const int generation) const = 0;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) = 0;

		virtual std::vector<std::string>getName() const = 0;

		virtual ~Monotonic_Adaptative_Cooling() {}
			
	};

	//=============================================================================
	//
	//	Class Exponential_Multiplicative_Cooling
	//  		
	//=============================================================================
	/**
	 * Kirkpatrick, Gelatt and Vecchi (1983)		
	 * @author hdiaz
	 *
	 */
	class Exponential_Multiplicative_Cooling: public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;

	public:
		Exponential_Multiplicative_Cooling() {}

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 0.9);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 20);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	
	public:
		virtual double getTemperature(const int generation) const;

		virtual std::vector<std::string> getName() const;
	
	};
	//=============================================================================
	//
	//	Class Logaritmic_Multiplicative_Cooling
	//  		
	//=============================================================================
	/**
	 * (Aarts, E.H.L. & Korst, J., 1989)		
	 * @author hdiaz
	 *
	 */
	class Logaritmic_Multiplicative_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;

	public:
		Logaritmic_Multiplicative_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 1.2);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;
	
	};

	//=============================================================================
/**
 * @author hdiaz
 *
 */
	class Linear_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;

	public:
		Linear_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 1);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 150);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

	/**
 * @author hdiaz
 *
 */
	class Linear_Multiplicative_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;

	public:
		Linear_Multiplicative_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 0.02);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

	/**
* @author hdiaz
*
*/
	class Quadratic_Multiplicative_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;

	public:
		Quadratic_Multiplicative_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 0.02);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

	/**
* @author hdiaz
*
*/
	class Linear_Additive_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;
		double tn;

	public:
		Linear_Additive_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 150);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
			this->tn = parameters->getDouble(SIM_COOLING_TN, 0);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

	/**
* @author hdiaz
*
*/
	class Quadratic_Additive_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;
		double tn;

	public:
		Quadratic_Additive_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 150);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
			this->tn = parameters->getDouble(SIM_COOLING_TN, 0);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

	/**
* @author hdiaz
*
*/
	class Trigonometric_Additive_Cooling :public Monotonic_Adaptative_Cooling {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	private:
		double a;
		double t0;
		double tn;

	public:
		Trigonometric_Additive_Cooling() {}

		virtual std::vector<std::string> getName() const;

		virtual void setUp(FuzzyFW::ParameterDB *parameters) {
			this->a = parameters->getDouble(SIM_COOLING_A, 100);
			this->t0 = parameters->getDouble(SIM_COOLING_T0, 1);
			this->tn = parameters->getDouble(SIM_COOLING_TN, 0);
		}

		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		virtual double getTemperature(const int generation) const;

	};

}

