/*
* FitnessCrisp.h
*
*  Created on: June 7, 2017
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessCrisp
//
//=============================================================================
/**
* Fitness backed by an Crisp value.
*/
class FitnessCrisp : public Fitness {
protected:
	Crisp value;

public:
	FitnessCrisp(bool maxim = true) : Fitness(maxim) { }

	FitnessCrisp(const Crisp & tfn, bool maxim = true)
		: Fitness(maxim), value(tfn) { }

	FitnessCrisp(const FitnessCrisp &fitness)
		: Fitness(fitness), value(fitness.value) { }

	virtual ~FitnessCrisp() { }

	virtual Fitness* clone() const {
		return new FitnessCrisp(*this);
	}

	virtual Fitness::Type getType() const {
		return Fitness::Type::CRISP;
	}

	Crisp getValue() const {
		return this->value;
	}

	void setValue(const Crisp source) {
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
	const FitnessCrisp * convertType(const Fitness *f) const;
};

}
