/*
* StatisticsFVRP.h
*
*  Created on: Nov 23, 2017
*/
#pragma once


#include "Statistics.h"
#include "EvaluationFVRP.h"


namespace FVRP {

//=============================================================================
//
//	Class StatisticsTravelCost
//
//=============================================================================
/**
* This class gives the travel cost from the population
*
*
*/
struct StatisticsTimeCost : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsTimeCost(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsTimeCost(StatisticsTimeCost & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsTimeCost(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsTimeCost() { }



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
		return this->getStatName() + " Travel Time";
	}
};





//=============================================================================
//
//	Class StatisticsDemandMinNec
//
//=============================================================================
/**
* This class gives the minimum necessity of meeting the demands
*
*
*/
struct StatisticsDemandMinNec : public FuzzyFW::Statistics {
protected:
	/*
	* Threshold used for computing the value
	*/
	double threshold;



public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsDemandMinNec(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsDemandMinNec(StatisticsDemandMinNec & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsDemandMinNec(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsDemandMinNec() { }



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
		return this->getStatName() + " minDemandNec";
	}
};





//=============================================================================
//
//	Class StatisticsDemandMinPos
//
//=============================================================================
/**
* This class gives the minimum possibility of meeting the demands
*
*
*/
struct StatisticsDemandMinPos : public FuzzyFW::Statistics {
protected:
	/*
	* Threshold used for computing the value
	*/
	double threshold;



public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsDemandMinPos(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsDemandMinPos(StatisticsDemandMinPos & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsDemandMinPos(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsDemandMinPos() { }



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
		return this->getStatName() + " minDemandPos";
	}
};







//=============================================================================
//
//	Class StatisticsDemandMinCred
//
//=============================================================================
/**
* This class gives the minimum credibility of meeting the demands
*
*
*/
struct StatisticsDemandMinCred : public FuzzyFW::Statistics {
protected:
	/*
	* Threshold used for computing the value
	*/
	double threshold;



public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsDemandMinCred(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsDemandMinCred(StatisticsDemandMinCred & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsDemandMinCred(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsDemandMinCred() { }



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
		return this->getStatName() + " minDemandCred";
	}
};

}
