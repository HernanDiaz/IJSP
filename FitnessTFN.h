/*
* FitnessTFN.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessTFN
//
//=============================================================================
/**
* Fitness backed by a Triangular Fuzzy Number (TFN).
*/
class FitnessTFN : public Fitness {
public:
	static TFN::Compare FitnessCompareStrategy;

protected:
	TFN value;

public:
	FitnessTFN(bool maxim = true) : Fitness(maxim) { }

	FitnessTFN(const TFN & tfn, bool maxim = true)
		: Fitness(maxim), value(tfn) { }

	FitnessTFN(const FitnessTFN &fitness)
		: Fitness(fitness), value(fitness.value) { }

	virtual ~FitnessTFN() { }

	virtual Fitness* clone() const {
		return new FitnessTFN(*this);
	}

	virtual Fitness::Type getType() const {
		return Fitness::Type::FUZZY;
	}

	TFN getValue() const {
		return this->value;
	}

	void setValue(const TFN source) {
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
	const FitnessTFN * convertType(const Fitness *f) const;
};

}
