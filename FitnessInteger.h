/*
* FitnessInteger.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessInteger
//
//=============================================================================
/**
* Fitness backed by an integer value.
*/
class FitnessInteger : public Fitness {
protected:
	int value;

public:
	FitnessInteger(bool maxim = true) : Fitness(maxim) { }

	FitnessInteger(const int i, bool maxim = true)
		: Fitness(maxim), value(i) { }

	FitnessInteger(const FitnessInteger &fitness)
		: Fitness(fitness), value(fitness.value) { }

	virtual ~FitnessInteger() { }

	virtual Fitness* clone() const {
		return new FitnessInteger(*this);
	}

	virtual Fitness::Type getType() const {
		return Fitness::Type::INTEGER;
	}

	int getValue() const {
		return this->value;
	}

	void setValue(const int source) {
		this->value = source;
	}

	virtual std::string toString() const {
		return valueToString(this->value);
	}

	virtual double toDouble() const {
		return (double)this->value;
	}

	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	const FitnessInteger * convertType(const Fitness *f) const;
};

}
