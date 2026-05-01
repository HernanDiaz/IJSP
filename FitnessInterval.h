/*
* FitnessInterval.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessInterval
//
//=============================================================================
/**
* Fitness backed by an Interval value.
*/
class FitnessInterval : public Fitness {
public:
	static Interval::Compare FitnessCompareStrategy;

protected:
	Interval value;

public:
	FitnessInterval(bool maxim = true) : Fitness(maxim) { }

	FitnessInterval(const Interval & tfn, bool maxim = true)
		: Fitness(maxim), value(tfn) { }

	FitnessInterval(const FitnessInterval &fitness)
		: Fitness(fitness), value(fitness.value) { }

	virtual ~FitnessInterval() { }

	virtual Fitness* clone() const {
		return new FitnessInterval(*this);
	}

	virtual Fitness::Type getType() const {
		return Fitness::Type::INTERVAL;
	}

	Interval getValue() const {
		return this->value;
	}

	void setValue(const Interval source) {
		this->value = source;
	}

	virtual std::string toString() const {
		return this->value.toString();
	}

	virtual double toDouble() const {
		return this->value.expectedValue();
	}

	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	const FitnessInterval * convertType(const Fitness *f) const;
};

}
