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
* This replaces Interval on this branch. A classic JSP instance is an interval
* JSP instance whose processing times are degenerate intervals [p, p], and every
* Interval ranking method collapses to the order on the reals when a == b, so
* nothing is lost by storing one exact integer instead of two doubles.
*
* What is gained is the whole of the ranking machinery. Interval::isGreaterThan
* was an out-of-line call into a seven-branch switch over a runtime enum, each
* branch ending in two epsilon comparisons on doubles; it sat in the innermost
* loop of the scheduler and of the local search. Here every comparison is an
* inline integer compare, and a time occupies 4 bytes instead of 16.
*
* Durations in the Taillard instances are integers, and both operations the
* scheduler performs on them -- addition and maximum -- keep them integral, so
* an integer time is exact. There is no rounding to reason about, which is one
* fewer thing than the double version had.
*
* The ranking strategies are gone, along with the arguments that selected them.
* There is one order on the integers and std::max is the maximum, so a call site
* that named a strategy was naming the only one there is. Setups may still carry
* the old evaluation.interval.* and sgs.interval.* keys; they are ignored.
*
* @author hdiaz
*/
class Crisp
{
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
	// One order, inline, on one integer. std::max and std::min are the maximum
	// and the minimum; there is no componentwise variant to choose between.
	//=========================================================================
public:
	bool operator==(const Crisp & t) const { return this->v == t.v; }

	bool operator!=(const Crisp & t) const { return this->v != t.v; }

	bool operator<(const Crisp & t) const { return this->v < t.v; }

	bool operator<=(const Crisp & t) const { return this->v <= t.v; }

	bool operator>(const Crisp & t) const { return this->v > t.v; }

	bool operator>=(const Crisp & t) const { return this->v >= t.v; }


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
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
