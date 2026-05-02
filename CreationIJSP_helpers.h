/*
 * CreationIJSP_helpers.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Fitness.h"
#include "Interval.h"

namespace {

inline bool fiBetter(const FuzzyFW::Interval& a, const FuzzyFW::Interval& b) {
	FuzzyFW::FitnessInterval fa(a, false), fb(b, false);
	return fa.isBetterThan(&fb);
}

inline bool fiWorse(const FuzzyFW::Interval& a, const FuzzyFW::Interval& b) {
	FuzzyFW::FitnessInterval fa(a, false), fb(b, false);
	return fa.isWorseThan(&fb);
}

} // anonymous namespace
