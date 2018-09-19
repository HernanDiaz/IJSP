#pragma once


/*
* StatisticsDiversity.h
*
*  Created on: Jun 13, 2018
*      Author: jjpalacios
*/

#include "Statistics.h"

namespace FuzzyFW {

	//=============================================================================
	//
	//	Class StatisticsHamming
	//
	//=============================================================================
	/**
	* This class allows to get Hamming distance between all the solutions of a
	* population.
	*
	* It accepts only individuals that are Integer Arrays
	*
	* @author jjpalacios
	*
	*/
class StatisticsHamming : public Statistics {
	public:
		/*
		* Main constructor
		* The flag indicates the metric to use:
		*/
		StatisticsHamming(STAT_TYPE flag = STAT_BEST);


		/*
		* Copy constructot
		*/
		StatisticsHamming(StatisticsHamming & source)
			: Statistics(source) { }


		/*
		* Clone method
		*/
		virtual Statistics * clone() {
			return new StatisticsHamming(*this);
		}


		/*
		* Destructor
		*/
		~StatisticsHamming() { }



		//=========================================================================
		//		METHODS
		//=========================================================================
	public:
		/*
		* Get the value from the population
		*/
		virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
			FuzzyFW::Population *population);

		/*
		* Get the value from an individual
		*/
		virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
			FuzzyFW::Individual *individual) {
			return 0.0;
		}


		/*
		* Get the value from the population
		*/
		virtual std::string getName() const {
			return this->getStatName() + " Hamming";
		}
	};





//=============================================================================
//
//	Class StatisticsKendall
//
//=============================================================================
/**
* This class allows to get Kendall-tau distance between all the solutions of a
* population.
*
* It accepts only individuals that are Integer Arrays
*
* @author jjpalacios
*
*/
class StatisticsKendall : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*/
	StatisticsKendall(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsKendall(StatisticsHamming & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsKendall(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsKendall() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);

	/*
	* Get the value from an individual
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Individual *individual) {
		return 0.0;
	}


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " Kendall-tau";
	}
};





//=============================================================================
//
//	Class StatisticsNeri
//
//=============================================================================
/**
* This class allows to compute the diversity metric used in the paper from Neri:
*
* An Adaptive Multimeme Algorithm for Designing HIV Multidrug Therapies.
* F.Neri, J.Toivanen, G.L.Cascella, Y-S. Ong. IEEE/ACM TRANSACTIONS ON
* COMPUTATIONAL BIOLOGY AND BIOINFORMATICS. Vol 4(2), 264-278. 2007
*
* @author jjpalacios
*
*/
class StatisticsNeri : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*/
	StatisticsNeri(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsNeri(StatisticsNeri & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsNeri(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsNeri() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);

	/*
	* Get the value from an individual
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Individual *individual) {
		return 0.0;
	}


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " Neri (W-B)";
	}
};





//=============================================================================
//
//	Class StatisticsNeriBest
//
//=============================================================================
/**
* This class allows to compute the diversity metric used in the paper from Neri:
*
* An Adaptive Multimeme Algorithm for Designing HIV Multidrug Therapies.
* F.Neri, J.Toivanen, G.L.Cascella, Y-S. Ong. IEEE/ACM TRANSACTIONS ON
* COMPUTATIONAL BIOLOGY AND BIOINFORMATICS. Vol 4(2), 264-278. 2007
*
* @author jjpalacios
*
*/
class StatisticsNeriBest : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*/
	StatisticsNeriBest(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsNeriBest(StatisticsNeriBest & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsNeriBest(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsNeriBest() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);

	/*
	* Get the value from an individual
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Individual *individual) {
		return 0.0;
	}


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " Neri (Best)";
	}
};




}
