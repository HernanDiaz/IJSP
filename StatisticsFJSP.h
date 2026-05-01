#pragma once

/*
* StatisticsFJSP.h
*
*  Created on: Oct 4, 2017
*/
#pragma once


#include "Statistics.h"
#include "EvaluationFJSP.h"


namespace FJSP {

//=============================================================================
//
//	Class StatisticsMakespan
//
//=============================================================================
/**
* This class gives the makespan from the population
*
*
*/
struct StatisticsMakespan : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*/
	StatisticsMakespan(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsMakespan(StatisticsMakespan & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsMakespan(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsMakespan() { }



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
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " Cmax";
	}
};





//=============================================================================
//
//	Class StatisticsAIavg
//
//=============================================================================
/**
* This class gives the average AI from the population
*
*
*/
struct StatisticsAIavg : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsAIavg(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsAIavg(StatisticsAIavg & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsAIavg(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsAIavg() { }



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
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " AIavg";
	}
};





//=============================================================================
//
//	Class StatisticsAImin
//
//=============================================================================
/**
* This class gives the minimum AI from the population
*
*
*/
struct StatisticsAImin : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsAImin(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsAImin(StatisticsAImin & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsAImin(*this);
	}



	/*
	* Destructor
	*/
	~StatisticsAImin() { }



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
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " AImin";
	}
};





//=============================================================================
//
//	Class StatisticsESDavg
//
//=============================================================================
/**
* This class gives the average Expected Satisfaction Degree from the population
*
*
*/
struct StatisticsESDavg : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsESDavg(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsESDavg(StatisticsESDavg & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsESDavg(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsESDavg() { }



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
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " ESDavg";
	}
};





//=============================================================================
//
//	Class StatisticsESDmin
//
//=============================================================================
/**
* This class gives the minimum Expected Satisfaction Degree from the population
*
*
*/
struct StatisticsESDmin : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsESDmin(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsESDmin(StatisticsAImin & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsESDmin(*this);
	}



	/*
	* Destructor
	*/
	~StatisticsESDmin() { }



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
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " ESDmin";
	}
};



}
