/*
* CrispTime.h
*
*  Created on: September 18, 2026
*      Author: Hernan Diaz
*/
#pragma once

#include "heading.h"

namespace FuzzyFW {

//=========================================================================
//
//	Class Crisp
//
//=========================================================================
/**
* A processing time, head or completion time of the classic (crisp) job shop.
*
* This replaces Interval on this branch. A classic JSP instance is an IJSP
* instance whose processing times are degenerate intervals [p, p], and every
* Interval ranking method collapses to the order on the reals when a == b, so
* nothing is lost by storing one exact integer instead of two doubles.
*
* What is gained is the whole of the ranking machinery. Interval::isGreaterThan
* was an out-of-line call into a seven-branch switch over a runtime enum, each
* branch ending in two epsilon comparisons on doubles; it sat in the innermost
* loop of the scheduler and of the local search. Here every comparison is an
* inline integer compare, and a time occupies 4 bytes instead of 16.
*
* The Compare and Maximum parameters are kept, and ignored, so that this type
* drops into the existing call sites unchanged. They are removed, along with
* the arguments that pass them, once the crisp solver is verified.
*
* Durations in the Taillard instances are integers, and both operations the
* scheduler performs on them -- addition and maximum -- keep them integral, so
* an integer time is exact. There is no rounding to reason about, which is one
* fewer thing than the double version had.
*
* @author hdiaz
*/
class Crisp
{
	//=========================================================================
	//		COMPATIBILITY WITH THE INTERVAL INTERFACE
	//=========================================================================
public:
	/**
	* Ranking strategies. All of them agree on points, so all of them are the
	* order on the integers here. Kept only so that setups and call sites that
	* still name a strategy compile; the value is never read.
	*/
	enum Compare { C_COMPONENT, C_EV, C_SAKAWA, C_JIANG, C_LEX1, C_LEX2, C_YX,
		C_Err };

	/**
	* Maximum strategies. Same story: the componentwise maximum of two points
	* is the ordinary maximum.
	*/
	enum Maximum { M_COMPONENT, M_EV, M_SAKAWA, M_JIANG, M_Err };

	static std::string getComparison(Crisp::Compare cmp);
	static Crisp::Compare getComparison(std::string str);
	static std::string getMaximum(Crisp::Maximum mxm);
	static Crisp::Maximum getMaximum(std::string str);


	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* The time itself. Public because Interval's endpoints were, and the code
	* that reached for them is ported rather than rewritten.
	*/
	int v;


	//=========================================================================
	//		CONSTRUCTORS
	//=========================================================================
public:
	Crisp() : v(0) { }

	explicit Crisp(int value) : v(value) { }

	/**
	* Two-argument form, for the call sites that built a degenerate interval.
	* The endpoints must agree: a crisp time cannot represent anything else,
	* and silently keeping one of them would hide a real bug in a caller that
	* still thinks it is handling uncertainty.
	*/
	explicit Crisp(double a1, double a2) : v(static_cast<int>(a1)) {
		if (a1 != a2)
			throw FuzzyFWException("Crisp",
				"A crisp time cannot be built from a proper interval");
	}

	Crisp(const Crisp & source) : v(source.v) { }


	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	Crisp operator+(const Crisp & t) const { return Crisp(this->v + t.v); }

	Crisp operator-(const Crisp & t) const { return Crisp(this->v - t.v); }

	Crisp & operator=(const Crisp & t) { this->v = t.v; return *this; }

	Crisp & operator+=(const Crisp & t) { this->v += t.v; return *this; }

	Crisp & operator-=(const Crisp & t) { this->v -= t.v; return *this; }


	//=========================================================================
	//		COMPARATORS
	//=========================================================================
public:
	bool isEqualTo(const Crisp t, const Compare) const { return this->v == t.v; }

	bool isGreaterThan(const Crisp t, const Compare) const {
		return this->v > t.v;
	}

	bool isGreaterEqualTo(const Crisp t, const Compare) const {
		return this->v >= t.v;
	}

	bool isLesserThan(const Crisp t, const Compare) const {
		return this->v < t.v;
	}

	bool isLesserEqualTo(const Crisp t, const Compare) const {
		return this->v <= t.v;
	}

	/**
	* Equality "in one component". A point has one component, so the component
	* index is ignored. This is what lets the N2 neighbourhood drop its second
	* pass over the critical path: both passes were asking the same question.
	*/
	bool EqualComponent(const Crisp t, const unsigned int) const {
		return this->v == t.v;
	}


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	friend Crisp maximum(const Crisp & x, const Crisp & y, const Maximum) {
		return x.v >= y.v ? x : y;
	}

	friend Crisp minimum(const Crisp & x, const Crisp & y, const Maximum) {
		return x.v <= y.v ? x : y;
	}

	double expectedValue() const { return static_cast<double>(this->v); }

	std::string toString() const;


	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Reads either "(a, b)", the degenerate-interval form the converted
	* Taillard instances in TaillardJSP/ are written in, or a bare integer.
	* Accepting the first form is what keeps those instance files valid, so a
	* run of the crisp solver is comparable with every run made before it.
	*/
	friend std::ifstream & operator >> (std::ifstream & is, Crisp & t);

	friend std::ostream & operator << (std::ostream & os, const Crisp & t);
};

}
