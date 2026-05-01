/*
* Fitness.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "TFN.h"
#include "Interval.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Fitness
//
//=============================================================================
/**
* Abstract wrapper for fitness values that hides the maximization/minimization
* logic from callers. Concrete types: FitnessDouble, FitnessInteger,
* FitnessTFN, FitnessInterval.
*/
class Fitness {
public:
	enum Type { INTEGER, DOUBLE, FUZZY, LEXICOGRAPHIC, INTERVAL };

protected:
	bool maximize;

public:
	Fitness(bool maxim = true) : maximize(maxim) { }

	Fitness(const Fitness &source)
		: maximize(source.maximize) { }

	virtual ~Fitness() { }

	virtual Fitness* clone() const = 0;

	virtual bool mustMaximize() const {
		return this->maximize;
	}

	virtual Fitness::Type getType() const = 0;

	virtual std::string toString() const = 0;

	virtual double toDouble() const {
		throw new FuzzyFWException("Fitness",
			"This fitness type cannot be converted to double");
	}

	virtual bool isBetterOrEqualTo(const Fitness * f) const = 0;
	virtual bool isBetterThan(const Fitness * f) const = 0;
	virtual bool isEqualTo(const Fitness * f) const = 0;
	virtual bool isWorseThan(const Fitness * f) const = 0;
	virtual bool isWorseOrEqualTo(const Fitness * f) const = 0;
};

}

#include "FitnessDouble.h"
#include "FitnessInteger.h"
#include "FitnessTFN.h"
#include "FitnessInterval.h"
