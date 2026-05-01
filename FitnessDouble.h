/*
* FitnessDouble.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessDouble
//
//=============================================================================
/**
* Fitness backed by a double value.
*/
class FitnessDouble : public Fitness {
protected:
	double value;

public:
	FitnessDouble(bool maxim = true) : Fitness(maxim) { }

	FitnessDouble(const double d, bool maxim = true)
		: Fitness(maxim), value(d) { }

	FitnessDouble(const FitnessDouble &fitness)
		: Fitness(fitness), value(fitness.value) { }

	virtual ~FitnessDouble() { }

	virtual Fitness* clone() const {
		return new FitnessDouble(*this);
	}

	virtual Fitness::Type getType() const {
		return Fitness::Type::DOUBLE;
	}

	double getValue() const {
		return this->value;
	}

	void setValue(const double source) {
		this->value = source;
	}

	virtual std::string toString() const {
		return valueToString(this->value);
	}

	virtual double toDouble() const {
		return this->value;
	}

	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	const FitnessDouble * convertType(const Fitness *f) const;
};

}
